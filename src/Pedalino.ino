#include <MD_Menu.h>
#include <MIDI_Controller.h>
#include <EEPROM.h>

#define DEBUG_PEDALINO

#define SIGNATURE "Pedalino(TM)"

#define BANKS             10

#if defined(__AVR_ATmega168__) || defined(__AVR_ATmega328P__)     // Arduino UNO, NANO
#define PEDALS             8
#elif defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)  // Arduino MEGA, MEGA2560
#define PEDALS            16
#endif

#define PED_PROGRAM_CHANGE  0
#define PED_CONTROL_CHANGE  1
#define PED_NOTE_ON_OFF     2

#define PED_MOMENTARY       0
#define PED_LATCH           1
#define PED_ANALOG          2
#define PED_JOG_WHEEL       3

#define PED_LINEAR          0
#define PED_LOG             1
#define PED_ANTILOG         2

#define PED_USBMIDI         0
#define PED_MIDIOUT         1


#define CALIBRATION_DURATION   8000

struct bank {
  byte                   midiFunction;   // 0 = Program Change, 1 = Control Code, 2 = Note On/Note Off
  byte                   midiChannel;    // MIDI channel 1-16
  byte                   midiCode;       // Program Change, Control Code or Note to send
};

struct pedal {
  byte                   mode;           // 0 = momentary, 1 = latch, 2 = analog, 3 = jog wheel
  byte                   invertPolarity;
  byte                   mapFunction;
  int                    expZero;
  int                    expMax;
  int                    pedalValue;
  MIDI_Control_Element  *footPedal;
};

bank   banks[BANKS][PEDALS];     // Banks Setup
pedal  pedals[PEDALS];           // Pedals Setup
byte   currentBank      = 0;
byte   currentPedal     = 0;
byte   currentInterface = PED_USBMIDI;
byte   lastUsedSwitch   = 0;
byte   lastUsedPedal    = 0;
bool   selectBank       = true;

#ifdef DEBUG_PEDALINO
USBDebugMIDI_Interface            midiInterface1(115200);
HardwareSerialDebugMIDI_Interface midiInterface2(Serial3, MIDI_BAUD);
#else
USBMIDI_Interface                 midiInterface1;
HardwareSerialMIDI_Interface      midiInterface2(Serial3, MIDI_BAUD);
#endif

// LCD display definitions

#include <LiquidCrystal.h>

#define  LCD_ROWS  2
#define  LCD_COLS  16

// LCD pin definitions

#define  LCD_RS         53
#define  LCD_ENA        51
#define  LCD_D4         49
#define  LCD_D5         47
#define  LCD_D6         45
#define  LCD_D7         43
#define  LCD_BACKLIGHT  41

LiquidCrystal lcd(LCD_RS, LCD_ENA, LCD_D4, LCD_D5, LCD_D6, LCD_D7, LCD_BACKLIGHT, POSITIVE);
boolean       powersaver = false;

// IR Remote receiver

#include <IRremote.h>

#define RECV_PIN      33     // connect Y to this PIN, G to GND, R to 5V
#define RECV_LED_PIN  35

#define IR_ON_OFF   0xFFEA15
#define IR_OK       0xFF48B7
#define IR_ESC      0xFFE817
#define IR_LEFT     0xFF7887
#define IR_RIGHT    0xFF6897
#define IR_UP       0xFF708F
#define IR_DOWN     0xFF28D7
#define IR_SWITCH   0xFFBA45
#define IR_KEY_1    0xFFDA25
#define IR_KEY_2    0xFFF20D
#define IR_KEY_3    0xFFCA35
#define IR_KEY_4    0xFF5AA5
#define IR_KEY_5    0xFFF00F
#define IR_KEY_6    0xFF7A85
#define IR_KEY_7    0xFF6A95
#define IR_KEY_8    0xFF728D
#define IR_KEY_9    0xFF4AB5
#define IR_KEY_0    0xFFAA55

IRrecv          irrecv(RECV_PIN, RECV_LED_PIN);
decode_results  results;

// BLE receiver

#if defined(__AVR_ATmega168__) || defined(__AVR_ATmega328P__)     // Arduino UNO, NANO
#include <SoftwareSerial.h>
#define BLE_RX_PIN  10
#define BLE_TX_PIN  11
SoftwareSerial  bluetooth(BLE_RX_PIN, BLE_TX_PIN);
#elif defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)  // Arduino MEGA, MEGA2560
#define bluetooth Serial2
#endif

// Software reset via watchdog

#include <avr/io.h>
#include <avr/wdt.h>

#define Reset_AVR() wdt_enable(WDTO_30MS); while(1) {}

const char bar1[]  = {49, 50, 51, 52, 53, 54, 55, 56, 57, 48};
const char bar2[]  = { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1};

void load_factory_default()
{
  for (byte b = 0; b < BANKS; b++)
    for (byte p = 0; p < PEDALS; p++)
      banks[b][p] = {PED_CONTROL_CHANGE, b + 1, p + 1};

  for (byte p = 0; p < PEDALS; p++)
    pedals[p] = {PED_MOMENTARY, 0, 0, 50, 930, 0, nullptr};

  pedals[0].mode = PED_ANALOG;
  pedals[1].mode = PED_ANALOG;
  pedals[2].mode = PED_ANALOG;
}


void midi_setup()
{
  for (int i = 0; i < PEDALS; i++) delete pedals[i].footPedal;

  switch (currentInterface) {
    case PED_USBMIDI:
      midiInterface1.setDefault();
      break;
    case PED_MIDIOUT:
      midiInterface2.setDefault();
      break;
  }

#ifdef DEBUG_PEDALINO
  Serial.print("Interface ");
  switch (currentInterface) {
    case PED_USBMIDI:
      Serial.println("USB");
      break;
    case PED_MIDIOUT:
      Serial.println("MIDI OUT");
      break;
  }
  Serial.print("Bank ");
  Serial.println(currentBank + 1);
#endif

  for (int i = 0; i < PEDALS; i++) {

#ifdef DEBUG_PEDALINO
    Serial.print("Pedal ");
    Serial.print(i + 1);
    Serial.print("     Mode ");
    Serial.print(pedals[i].mode);
    Serial.print("   Polarity ");
    Serial.print(pedals[i].invertPolarity);
    Serial.print("   MIDI Function ");
    Serial.print(banks[currentBank][i].midiFunction);
    Serial.print("   Channel ");
    Serial.print(banks[currentBank][i].midiChannel);
    Serial.print("   Code ");
    Serial.print(banks[currentBank][i].midiCode);
    Serial.println("");
#endif

    switch (pedals[i].mode) {

      case PED_MOMENTARY:
        switch (banks[currentBank][i].midiFunction)
        {
          case PED_PROGRAM_CHANGE:
            pedals[i].footPedal = new Digital(PIN_A0 + i, PROGRAM_CHANGE, banks[currentBank][i].midiCode, banks[currentBank][i].midiChannel);
            break;
          case PED_CONTROL_CHANGE:
            pedals[i].footPedal = new Digital(PIN_A0 + i, CONTROL_CHANGE, banks[currentBank][i].midiCode, banks[currentBank][i].midiChannel);
            break;
          case PED_NOTE_ON_OFF:
            pedals[i].footPedal = new Digital(PIN_A0 + i, NOTE_ON, banks[currentBank][i].midiCode, banks[currentBank][i].midiChannel);
            break;
        }
        pedals[i].footPedal->map(mapDigital);
        break;

      case PED_LATCH:
        switch (banks[currentBank][i].midiFunction)
        {
          case PED_PROGRAM_CHANGE:
            pedals[i].footPedal = new DigitalLatch(PIN_A0 + i, PROGRAM_CHANGE, banks[currentBank][i].midiCode, banks[currentBank][i].midiChannel);
            break;
          case PED_CONTROL_CHANGE:
            pedals[i].footPedal = new DigitalLatch(PIN_A0 + i, CONTROL_CHANGE, banks[currentBank][i].midiCode, banks[currentBank][i].midiChannel);
            break;
          case PED_NOTE_ON_OFF:
            pedals[i].footPedal = new DigitalLatch(PIN_A0 + i, NOTE_ON, banks[currentBank][i].midiCode, banks[currentBank][i].midiChannel);
            break;
        }
        pedals[i].footPedal->map(mapDigital);
        break;

      case PED_ANALOG:
        pedals[i].footPedal = new AnalogResponsive(PIN_A0 + i, banks[currentBank][i].midiCode, banks[currentBank][i].midiChannel);
        pedals[i].footPedal->map(mapAnalog);
        break;

      case PED_JOG_WHEEL:
        break;
    }
    if (pedals[i].invertPolarity) pedals[i].footPedal->invert();
  }
}


void calibrate()
{
  unsigned long start = millis();

  lcd.clear();
  lcd.setCursor(0, 0);
  for (int i = 1; i <= LCD_COLS; i++)
    lcd.print(char(B10100101));
  pedals[currentPedal].expZero = 1023;
  pedals[currentPedal].expMax = 0;
  while (millis() - start < CALIBRATION_DURATION) {
    int ax = analogRead(PIN_A0 + currentPedal);
    pedals[currentPedal].expZero = min( pedals[currentPedal].expZero, ax + 20);
    pedals[currentPedal].expMax = max( pedals[currentPedal].expMax, ax - 20);
    lcd.setCursor(LCD_COLS - map(millis() - start, 0, CALIBRATION_DURATION, 0, LCD_COLS), 0);
    lcd.print(" ");
    lcd.setCursor(0, 1);
    lcd.print(pedals[currentPedal].expZero);
    for (int i = 1; i < LCD_COLS - floor(log10(pedals[currentPedal].expZero + 1)) - floor(log10(pedals[currentPedal].expMax + 1)) - 1; i++)
      lcd.print(" ");
    lcd.print(pedals[currentPedal].expMax);
  }
}

int mapDigital(int p, int value)
{
  p -= PIN_A0;
  p = constrain(p, 0, PEDALS);
  if (pedals[p].pedalValue != value) lastUsedSwitch = p;
  pedals[p].pedalValue = value;
  return value;
}

int mapAnalog(int p, int value)
{
  p -= PIN_A0;
  p = constrain(p, 0, PEDALS);
  value = constrain(value, pedals[p].expZero, pedals[p].expMax);    // make sure that the analog value is between the minimum and maximum value
  value = map(value, pedals[p].expZero, pedals[p].expMax, 0, 1023); // map the value from [minimumValue, maximumValue] to [0, 1023]
  switch (pedals[p].mapFunction) {
    case PED_LINEAR:
      break;
    case PED_LOG:
      value = round(log(value + 1) * 147.61);             // y=log(x+1)/log(1023)*1023
      break;
    case PED_ANTILOG:
      value = round((exp(value / 511.5) - 1) * 160.12);   // y=[e^(2*x/1023)-1]/[e^2-1]*1023
      break;
  }
  if (abs(pedals[p].pedalValue - value) > 30) lastUsedPedal = p;
  pedals[p].pedalValue = value;
  return value;
}

void screen_update() {

  static char screen1[LCD_COLS + 1];
  static char screen2[LCD_COLS + 1];

  if (!powersaver) {
    char buf[LCD_COLS + 1];
    // Line 1
    memset(buf, 0, sizeof(buf));
    for (byte i = 0; i < PEDALS; i++) {
      buf[i] = foot_char(i);
    }
    if (strcmp(screen1, buf) != 0) {
      memset(screen1, 0, LCD_COLS + 1);
      strncpy(screen1, buf, LCD_COLS);
      lcd.setCursor(0, 0);
      lcd.print(buf);
    }
    // Line 2
    memset(buf, 0, sizeof(buf));
    sprintf(&buf[strlen(buf)], "Bank%2d", currentBank + 1);
    //sprintf(&buf[strlen(buf)], "%2x%2x%2x", currentBank + 1, banks[currentBank][lastUsedSwitch].midiChannel, banks[currentBank][lastUsedSwitch].midiCode);
    strncpy(&buf[strlen(buf)], &bar2[0], map(pedals[lastUsedPedal].pedalValue, 0, 1023, 0, 10));
    strncpy(&buf[strlen(buf)], "          ", 10 - map(pedals[lastUsedPedal].pedalValue, 0, 1023, 0, 10));
    if (strcmp(screen2, buf) != 0) {
      memset(screen2, 0, LCD_COLS + 1);
      strncpy(screen2, buf, LCD_COLS);
      lcd.setCursor(0, 1);
      lcd.print(buf);
    }
    if (selectBank) {
      lcd.setCursor(5, 1);
      lcd.cursor();
    }
    else
      lcd.noCursor();
  }
}

char foot_char (byte footswitch)
{
  footswitch = constrain(footswitch, 0, PEDALS - 1);
  if (footswitch == lastUsedPedal || pedals[footswitch].pedalValue == LOW) return bar1[footswitch % 10];
  return ' ';
}


void update_eeprom() {

  int offset = 0;

#ifdef DEBUG_PEDALINO
  Serial.print("Updating EEPROM ... ");
#endif

  EEPROM.put(offset, SIGNATURE);
  offset += sizeof(SIGNATURE);

  for (int b = 0; b < BANKS; b++)
    for (int p = 0; p < PEDALS; p++) {
      EEPROM.put(offset, banks[b][p].midiFunction);
      offset += sizeof(byte);
      EEPROM.put(offset, banks[b][p].midiChannel);
      offset += sizeof(byte);
      EEPROM.put(offset, banks[b][p].midiCode);
      offset += sizeof(byte);
    }

  for (int p = 0; p < PEDALS; p++) {
    EEPROM.put(offset, pedals[p].mode);
    offset += sizeof(byte);
    EEPROM.put(offset, pedals[p].invertPolarity);
    offset += sizeof(byte);
    EEPROM.put(offset, pedals[p].mapFunction);
    offset += sizeof(byte);
    EEPROM.put(offset, pedals[p].expZero);
    offset += sizeof(int);
    EEPROM.put(offset, pedals[p].expMax);
    offset += sizeof(int);
  }

  EEPROM.put(offset, currentBank);
  offset += sizeof(byte);
  EEPROM.put(offset, currentPedal);
  offset += sizeof(byte);
  EEPROM.put(offset, currentInterface);
  offset += sizeof(byte);

#ifdef DEBUG_PEDALINO
  Serial.println("end.");
#endif

}

void read_eeprom() {

  int offset = 0;
  char signature[LCD_COLS + 1];

  load_factory_default();

  EEPROM.get(offset, signature);
  offset += sizeof(SIGNATURE);

#ifdef DEBUG_PEDALINO
  Serial.print("EEPROM signature: ");
  Serial.println(signature);
#endif

  if (strcmp(signature, SIGNATURE) == 0) {

#ifdef DEBUG_PEDALINO
    Serial.print("Reading EEPROM ... ");
#endif

    for (int b = 0; b < BANKS; b++)
      for (int p = 0; p < PEDALS; p++) {
        EEPROM.get(offset, banks[b][p].midiFunction);
        offset += sizeof(byte);
        EEPROM.get(offset, banks[b][p].midiChannel);
        offset += sizeof(byte);
        EEPROM.get(offset, banks[b][p].midiCode);
        offset += sizeof(byte);
      }

    for (int p = 0; p < PEDALS; p++) {
      EEPROM.get(offset, pedals[p].mode);
      offset += sizeof(byte);
      EEPROM.get(offset, pedals[p].invertPolarity);
      offset += sizeof(byte);
      EEPROM.get(offset, pedals[p].mapFunction);
      offset += sizeof(byte);
      EEPROM.get(offset, pedals[p].expZero);
      offset += sizeof(int);
      EEPROM.get(offset, pedals[p].expMax);
      offset += sizeof(int);
    }

    EEPROM.get(offset, currentBank);
    offset += sizeof(byte);
    EEPROM.get(offset, currentPedal);
    offset += sizeof(byte);
    EEPROM.get(offset, currentInterface);
    offset += sizeof(byte);

#ifdef DEBUG_PEDALINO
    Serial.println("end.");
#endif

  }
}

const bool      AUTO_START = true;    // auto start the menu, manual detect and start if false
const uint16_t  MENU_TIMEOUT = 5000;  // in milliseconds

// Function prototypes for Navigation/Display
bool display(MD_Menu::userDisplayAction_t, char* = nullptr);
MD_Menu::userNavAction_t navigation(uint16_t &incDelta);

// Function prototypes for variable get/set functions
MD_Menu::value_t *mnuValueRqst(MD_Menu::mnuId_t id, bool bGet);

#define M_ROOT            10
#define M_BANKSETUP       11
#define M_PEDALSETUP      12
#define M_PROFILE         13

#define II_BANK           20
#define II_PEDAL          21
#define II_MIDICHANNEL    22
#define II_MIDIFUNCTION   23
#define II_MIDICODE       24
#define II_MODE           25
#define II_POLARITY       26
#define II_CALIBRATE      27
#define II_ZERO           28
#define II_MAX            29
#define II_MAP            30
#define II_INTERFACE      31
#define II_DEFAULT        32

// Global menu data and definitions

MD_Menu::value_t vBuf;  // interface buffer for values

// Menu Headers --------
const PROGMEM MD_Menu::mnuHeader_t mnuHdr[] =
{
  { M_ROOT,       SIGNATURE,      10, 12, 0 },
  { M_BANKSETUP,  "Banks Setup",  20, 33, 0 },
  { M_PEDALSETUP, "Pedals Setup", 40, 46, 0 },
  { M_PROFILE,    "Options",      50, 51, 0 },
};

// Menu Items ----------
const PROGMEM MD_Menu::mnuItem_t mnuItm[] =
{
  // Starting (Root) menu
  { 10, "Banks Setup",     MD_Menu::MNU_MENU,  M_BANKSETUP },
  { 11, "Pedals Setup",    MD_Menu::MNU_MENU,  M_PEDALSETUP },
  { 12, "Options",         MD_Menu::MNU_MENU,  M_PROFILE },
  // Banks Setup
  { 20, "Select Bank",     MD_Menu::MNU_INPUT, II_BANK },
  { 30, "Select Pedal",    MD_Menu::MNU_INPUT, II_PEDAL },
  { 31, "SetMIDIChannel",  MD_Menu::MNU_INPUT, II_MIDICHANNEL },
  { 32, "SetMIDIFunction", MD_Menu::MNU_INPUT, II_MIDIFUNCTION },
  { 33, "Set MIDI Code",   MD_Menu::MNU_INPUT, II_MIDICODE },
  // Pedals Setup
  { 40, "Select Pedal",    MD_Menu::MNU_INPUT, II_PEDAL },
  { 41, "Set Mode",        MD_Menu::MNU_INPUT, II_MODE },
  { 42, "Set Polarity",    MD_Menu::MNU_INPUT, II_POLARITY },
  { 43, "Calibrate",       MD_Menu::MNU_INPUT, II_CALIBRATE },
  { 44, "Set Zero",        MD_Menu::MNU_INPUT, II_ZERO },
  { 45, "Set Max",         MD_Menu::MNU_INPUT, II_MAX },
  { 46, "Map Function",    MD_Menu::MNU_INPUT, II_MAP },
  // Options
  { 50, "Interface",       MD_Menu::MNU_INPUT, II_INTERFACE },
  { 51, "Factory default", MD_Menu::MNU_INPUT, II_DEFAULT }
};

// Input Items ---------
const PROGMEM char listPedalMode[] = "   Momentary  |     Latch    |    Analog    |   Jog Wheel  ";
const PROGMEM char listMidiFunction[] = "Program Change| Control Code |  Note On/Off  ";
const PROGMEM char listPolarity[] = " No|Yes";
const PROGMEM char listMapFunction[] = "    Linear    |     Log     |   Anti-Log   ";
const PROGMEM char listOutputInterface[] = "     USB     |   MIDI OUT   ";

const PROGMEM MD_Menu::mnuInput_t mnuInp[] =
{
  { II_BANK,          ">1-10:      ", MD_Menu::INP_INT,   mnuValueRqst,  2, 1, 0,  BANKS, 0, 10, nullptr },
#if defined(__AVR_ATmega168__) || defined(__AVR_ATmega328P__)
  { II_PEDAL,         ">1-8:       ", MD_Menu::INP_INT,   mnuValueRqst,  2, 1, 0, PEDALS, 0, 10, nullptr },
#elif defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
  { II_PEDAL,         ">1-16:      ", MD_Menu::INP_INT,   mnuValueRqst,  2, 1, 0, PEDALS, 0, 10, nullptr },
#endif
  { II_MIDICHANNEL,   ">1-16:      ", MD_Menu::INP_INT,   mnuValueRqst,  2, 1, 0,     16, 0, 10, nullptr },
  { II_MIDIFUNCTION,  ""            , MD_Menu::INP_LIST,  mnuValueRqst, 14, 0, 0,      0, 0,  0, listMidiFunction },
  { II_MIDICODE,      "0-127:     " , MD_Menu::INP_INT,   mnuValueRqst,  3, 0, 0,    127, 0, 10, nullptr },
  { II_MODE,          ""            , MD_Menu::INP_LIST,  mnuValueRqst, 14, 0, 0,      0, 0,  0, listPedalMode },
  { II_POLARITY,      "Invert:    " , MD_Menu::INP_LIST,  mnuValueRqst,  3, 0, 0,      0, 0,  0, listPolarity },
  { II_CALIBRATE,     "Confirm"     , MD_Menu::INP_RUN,   mnuValueRqst,  0, 0, 0,      0, 0,  0, nullptr },
  { II_ZERO,          ">0-1023:  "  , MD_Menu::INP_INT,   mnuValueRqst,  4, 0, 0,   1023, 0, 10, nullptr },
  { II_MAX,           ">0-1023:  "  , MD_Menu::INP_INT,   mnuValueRqst,  4, 0, 0,   1023, 0, 10, nullptr },
  { II_MAP,           ""            , MD_Menu::INP_LIST,  mnuValueRqst, 14, 0, 0,      0, 0,  0, listMapFunction },
  { II_INTERFACE,     ""            , MD_Menu::INP_LIST,  mnuValueRqst, 14, 0, 0,      0, 0,  0, listOutputInterface },
  { II_DEFAULT,       "Confirm"     , MD_Menu::INP_RUN,   mnuValueRqst,  0, 0, 0,      0, 0,  0, nullptr }
};

// bring it all together in the global menu object
MD_Menu M(navigation, display,        // user navigation and display
          mnuHdr, ARRAY_SIZE(mnuHdr), // menu header data
          mnuItm, ARRAY_SIZE(mnuItm), // menu item data
          mnuInp, ARRAY_SIZE(mnuInp));// menu input data

// Callback code for menu set/get input values

MD_Menu::value_t *mnuValueRqst(MD_Menu::mnuId_t id, bool bGet)
{
  MD_Menu::value_t *r = &vBuf;

  switch (id)
  {
    case II_BANK:
      if (bGet) vBuf.value = currentBank + 1;
      else currentBank = vBuf.value - 1;
      break;

    case II_PEDAL:
      if (bGet) vBuf.value = currentPedal + 1;
      else currentPedal = vBuf.value - 1;
      break;

    case II_MIDICHANNEL:
      if (bGet) vBuf.value = banks[currentBank][currentPedal].midiChannel;
      else banks[currentBank][currentPedal].midiChannel = vBuf.value;
      break;

    case II_MIDIFUNCTION:
      if (bGet) vBuf.value = banks[currentBank][currentPedal].midiFunction;
      else banks[currentBank][currentPedal].midiFunction = vBuf.value;
      break;

    case II_MIDICODE:
      if (bGet) vBuf.value = banks[currentBank][currentPedal].midiCode;
      else banks[currentBank][currentPedal].midiCode = vBuf.value;
      break;

    case II_MODE:
      if (bGet) vBuf.value = pedals[currentPedal].mode;
      else pedals[currentPedal].mode = vBuf.value;
      break;

    case II_POLARITY:
      if (bGet) vBuf.value = pedals[currentPedal].invertPolarity;
      else pedals[currentPedal].invertPolarity = vBuf.value;
      break;

    case II_CALIBRATE:
      if (!bGet && pedals[currentPedal].mode == 2) calibrate();
      r = nullptr;
      break;

    case II_ZERO:
      if (bGet)
        if (pedals[currentPedal].mode == 2)
          vBuf.value = pedals[currentPedal].expZero;
        else r = nullptr;
      else pedals[currentPedal].expZero = vBuf.value;
      break;

    case II_MAX:
      if (bGet)
        if (pedals[currentPedal].mode == 2)
          vBuf.value = pedals[currentPedal].expMax;
        else r = nullptr;
      else pedals[currentPedal].expMax = vBuf.value;
      break;

    case II_MAP:
      if (bGet)
        if (pedals[currentPedal].mode == 2)
          vBuf.value = pedals[currentPedal].mapFunction;
        else r = nullptr;
      else pedals[currentPedal].mapFunction = vBuf.value;
      break;

    case II_INTERFACE:
      if (bGet) vBuf.value = currentInterface;
      else currentInterface = vBuf.value;
      break;

    case II_DEFAULT:
      if (!bGet) {
        lcd.clear();
        // Sets all of the bytes of the EEPROM to 0.
        for (int i = 0 ; i < EEPROM.length() ; i++) {
          EEPROM.write(i, 0);
          lcd.setCursor(map(i, 0, EEPROM.length(), 0, LCD_COLS - 1), 0);
          lcd.print(char(B10100101));
        }
        Reset_AVR();
      }

    default:
      r = nullptr;
      break;
  }

  if (!bGet) {
    update_eeprom();
    midi_setup();
  }

  return (r);
}

bool display(MD_Menu::userDisplayAction_t action, char *msg)
{
  static char szLine[LCD_COLS + 1] = { '\0' };

  switch (action)
  {
    case MD_Menu::DISP_INIT:
      lcd.begin(LCD_COLS, LCD_ROWS);
      lcd.clear();
      lcd.noCursor();
      memset(szLine, ' ', LCD_COLS);
      break;

    case MD_Menu::DISP_CLEAR:
      lcd.clear();
      break;

    case MD_Menu::DISP_L0:
      lcd.setCursor(0, 0);
      lcd.print(szLine);
      lcd.setCursor(0, 0);
      lcd.print(msg);
      break;

    case MD_Menu::DISP_L1:
      lcd.setCursor(0, 1);
      lcd.print(szLine);
      lcd.setCursor(0, 1);
      lcd.print(msg);
      break;
  }

  return (true);
}

MD_Menu::userNavAction_t navigation(uint16_t &incDelta)
{
  incDelta = 1;
  static unsigned long  previous_value;
  static byte           count = 0;
  unsigned long         ircode;
  bool                  repeat;
  byte                  numberPressed = 127;

  if (irrecv.decode(&results)) {
    ircode = results.value;
    irrecv.resume();
    repeat = (ircode == REPEAT && count > 4);
    if (repeat) ircode = previous_value;
#ifdef DEBUG_PEDALINO
    if (ircode > 0xFFFFFF && ircode != REPEAT) return MD_Menu::NAV_NULL;
    Serial.print("Key pressed: ");
    Serial.println(ircode, HEX);
#endif
    switch (ircode) {
      case 0xFF22DD:
      case IR_RIGHT:
      case IR_DOWN:
        if (!repeat) count = 0;
        previous_value = ircode;
        return MD_Menu::NAV_DEC;

      case 0xFFC23D:
      case IR_LEFT:
      case IR_UP:
        if (!repeat) count = 0;
        previous_value = ircode;
        return MD_Menu::NAV_INC;

      case 0xFF02FD:
      case IR_OK:
        if (!repeat) count = 0;
        previous_value = ircode;
        return MD_Menu::NAV_SEL;

      case 0xFFE21D:
      case IR_ESC:
        if (!repeat) count = 0;
        previous_value = ircode;
        return MD_Menu::NAV_ESC;

      case 0xFFA25D:
      case IR_ON_OFF:
        if (powersaver) {
          lcd.on();
          powersaver = false;
        }
        else {
          lcd.off();
          powersaver = true;
        }
        return MD_Menu::NAV_NULL;

      case REPEAT:
        count++;
        break;

      case IR_KEY_1: // 1
        numberPressed = 1;
        break;
      case IR_KEY_2: // 2
        numberPressed = 2;
        break;
      case IR_KEY_3: // 3
        numberPressed = 3;
        break;
      case IR_KEY_4: // 4
        numberPressed = 4;
        break;
      case IR_KEY_5: // 5
        numberPressed = 5;
        break;
      case IR_KEY_6: // 6
        numberPressed = 6;
        break;
      case IR_KEY_7: // 7
        numberPressed = 7;
        break;
      case IR_KEY_8: // 8
        numberPressed = 8;
        break;
      case IR_KEY_9: // 9
        numberPressed = 9;
        break;
      case IR_KEY_0: // 0
        numberPressed = 10;
        break;

      case IR_SWITCH:
        selectBank = !selectBank;
        break;

      default:
        previous_value = 0;
        break;
    }
  }
  if (numberPressed != 127) {
    if (selectBank) {
      currentBank = numberPressed - 1;
      update_eeprom();
      midi_setup();
    }
    else {
      pedals[numberPressed - 1].pedalValue = LOW;
      pedals[numberPressed - 1].footPedal->push();
      screen_update();
      delay(200);
      pedals[numberPressed - 1].pedalValue = HIGH;
      pedals[numberPressed - 1].footPedal->release();
    }
  }

  if (bluetooth.available()) {
    switch (char(bluetooth.read())) {
      case 'L':
        return MD_Menu::NAV_DEC;
      case 'R':
        return MD_Menu::NAV_INC;
      case 'C':
        return MD_Menu::NAV_SEL;
      case 'E':
        return MD_Menu::NAV_ESC;
    }
  }
  return MD_Menu::NAV_NULL;
}


// Standard setup() and loop()

void setup(void)
{

#ifdef DEBUG_PEDALINO
  Serial.begin(115200);
#endif

  read_eeprom();
  midi_setup();

  irrecv.enableIRIn();                        // Start the IR receiver
  irrecv.blink13(true);
  bluetooth.begin(9600);                      // Start the Bluetooth receiver
  bluetooth.println("AT+NAME=Pedalino(TM)");  // Set bluetooth device name

  display(MD_Menu::DISP_INIT);
  M.begin();
  M.setMenuWrap(true);
  M.setAutoStart(AUTO_START);
  //M.setTimeout(MENU_TIMEOUT);
}


void loop(void)
{
  static bool prevMenuRun = true;

  // Detect if we need to initiate running normal user code
  //if (prevMenuRun && !M.isInMenu())
  //
  prevMenuRun = M.isInMenu();

  // If we are not running and not autostart
  // check if there is a reason to start the menu
  if (!M.isInMenu() && !AUTO_START)
  {
    uint16_t dummy;

    if (navigation(dummy) == MD_Menu::NAV_SEL)
      M.runMenu(true);
  }
  if (!M.isInMenu()) screen_update();

  M.runMenu();   // just run the menu code

  // Refresh the MIDI controller (check whether the input has changed since last time, if so, send the new value over MIDI)
  MIDI_Controller.refresh();
}
