#include <MD_Menu.h>
#include <MIDI_Controller.h>
#include <EEPROM.h>

#define DEBUG_PEDALINO

#define SIGNATURE "Pedalino(TM)"

#define BANKS             10

#if defined(__AVR_ATmega168__) || defined(__AVR_ATmega328P__)     // Arduino UNO, NANO
#define PEDALS             8
#elif defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)  // Arduino MEGA, MEGA2560
#define PEDALS            8
#endif

#define PED_PROGRAM_CHANGE  0
#define PED_CONTROL_CHANGE  1
#define PED_NOTE_ON_OFF     2

#define PED_MOMENTARY       0
#define PED_LATCH           1
#define PED_ANALOG          2
#define PED_JOG_WHEEL       3

#define PED_LINEAR          0
#define PED_LOGARITHMIC     1
#define PED_EXPONENTIAL     2

#define PED_USBMIDI         0
#define PED_MIDIOUT         1


#define CALIBRATION_DURATION   8000

struct bank {
  byte                   midiFunction;   // 0 = Program Change, 1 = Control Code, 2 = Note On/Note Off
  byte                   midiChannel;    // MIDI channel 1-16
  byte                   midiCode;       // Program Change, Control Code or Note
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
byte   currentBank = 0;
byte   currentPedal = 0;
byte   currentInterface = PED_USBMIDI;

#ifdef DEBUG_PEDALINO
USBDebugMIDI_Interface            midiInterface1(115200);
HardwareSerialDebugMIDI_Interface midiInterface2(Serial3, MIDI_BAUD);
#endif
#ifndef DEBUG_PEDALINO
USBMIDI_Interface            midiInterface1;
HardwareSerialMIDI_Interface midiInterface2(Serial3, MIDI_BAUD);
#endif

// LCD display definitions

#include <LiquidCrystal.h>

#define  LCD_ROWS  2
#define  LCD_COLS  16

// LCD pin definitions

#define  LCD_RS         43
#define  LCD_ENA        41
#define  LCD_D4         31
#define  LCD_D5         33
#define  LCD_D6         35
#define  LCD_D7         37
#define  LCD_BACKLIGHT  12

static LiquidCrystal lcd(LCD_RS, LCD_ENA, LCD_D4, LCD_D5, LCD_D6, LCD_D7, LCD_BACKLIGHT, POSITIVE);
boolean powersaver = false;

// IR Remote receiver

#include <IRremote.h>

#define RECV_PIN      30     // connect Y to this PIN, G to GND, R to 5V
#define RECV_LED_PIN  32

IRrecv          irrecv(RECV_PIN, RECV_LED_PIN);
decode_results  results;

// BLE receiver

#include <SoftwareSerial.h>

#define BLE_RX_PIN  51
#define BLE_TX_PIN  50

SoftwareSerial  bluetooth(BLE_RX_PIN, BLE_TX_PIN);

// Software reset via watchdog

#include <avr/io.h>
#include <avr/wdt.h>

#define Reset_AVR() wdt_enable(WDTO_30MS); while(1) {}

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
        pedals[i].footPedal = new DigitalLatch(PIN_A0 + i, banks[currentBank][i].midiCode, banks[currentBank][i].midiChannel);
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
  //pedals[currentPedal].expZero += 5;
  //pedals[currentPedal].expMax  -= 5;
}

int mapDigital(int p, int value)
{
  p -= PIN_A0;
  p = constrain(p, 0, PEDALS);
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
    case PED_LOGARITHMIC:
      value = round(log(value + 1) * 147.61);             // y=log(x+1)/log(1023)*1023
      break;
    case PED_EXPONENTIAL:
      value = round((exp(value / 511.5) - 1) * 160.12);   // y=[e^(2*x/1023)-1]/[e^2-1]*1023
      break;
  }
  pedals[p].pedalValue = value;
  return value;
}

void screen_update() {

  if (!powersaver) {
    char buf[LCD_COLS * LCD_ROWS + 2 * LCD_ROWS];
    const char bar[] = { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1};
    //const char bar[] = {49,50,51,52,53,54,55,56,57,48};
    memset(buf, 0, sizeof(buf));
    for (int i = 0; i <= 5; i++) {
      buf[i] = foot_char(i + 3);
    }
    strncpy(&buf[6], &bar[0], map(pedals[0].pedalValue, 0, 1023, 0, 10));
    strncpy(&buf[strlen(buf)], "          ", 10 - map(pedals[0].pedalValue, 0, 1023, 0, 10));
    lcd.setCursor(0, 0);
    lcd.print(buf);
    memset(buf, 0, sizeof(buf));
    sprintf(&buf[strlen(buf)], "Bank%2d", currentBank + 1);
    strncpy(&buf[strlen(buf)], &bar[0], map(pedals[1].pedalValue, 0, 1023, 0, 10));
    strncpy(&buf[strlen(buf)], "          ", 10 - map(pedals[1].pedalValue, 0, 1023, 0, 10));
    lcd.setCursor(0, 1);
    lcd.print(buf);
  }
}

char foot_char (int footswitch)
{
  footswitch = constrain(footswitch, 0, PEDALS - 1);
  if (pedals[footswitch].pedalValue == LOW) return '1' + footswitch;
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
const PROGMEM char listMapFunction[] = "    Linear    |  Logarithmic  |  Exponential  ";
const PROGMEM char listOutputInterface[] = "     USB     |   MIDI OUT   ";

const PROGMEM MD_Menu::mnuInput_t mnuInp[] =
{
  { II_BANK,          ">1-10:      ", MD_Menu::INP_INT,   mnuValueRqst,  2, 1, 0,  BANKS, 0, 10, nullptr },
  { II_PEDAL,         ">1-8:       ", MD_Menu::INP_INT,   mnuValueRqst,  2, 1, 0, PEDALS, 0, 10, nullptr },
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
  static bool           selectBank = true;
  unsigned long         ircode;
  bool                  repeat;

  if (irrecv.decode(&results)) {
    ircode = results.value;
    irrecv.resume();
    repeat = (ircode == REPEAT && count > 4);
    if (repeat) ircode = previous_value;
    //Serial.println(ircode, HEX);
    switch (ircode) {
      case 0xFF22DD:
      case 0xFF6897:
      case 0xFF28D7:
        if (!repeat) count = 0;
        previous_value = ircode;
        return MD_Menu::NAV_DEC;
      case 0xFFC23D:
      case 0xFF7887:
      case 0xFF708F:
        if (!repeat) count = 0;
        previous_value = ircode;
        return MD_Menu::NAV_INC;
      case 0xFF02FD:
      case 0xFF48B7:
        if (!repeat) count = 0;
        previous_value = ircode;
        return MD_Menu::NAV_SEL;
      case 0xFFE21D:
      case 0xFFE817:
        if (!repeat) count = 0;
        previous_value = ircode;
        return MD_Menu::NAV_ESC;
      case 0xFFA25D:
      case 0xFFEA15:
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

      case 0xFFDA25: // 1
        currentBank = 0;
        update_eeprom();
        midi_setup();
        break;
      case 0xFFF20D: // 2
        currentBank = 1;
        update_eeprom();
        midi_setup();
        break;
      case 0xFFCA35: // 3
        currentBank = 2;
        update_eeprom();
        midi_setup();
        break;
      case 0xFF5AA5: //4
        currentBank = 3;
        update_eeprom();
        midi_setup();
        break;
      case 0xFFF00F: // 5
        currentBank = 4;
        update_eeprom();
        midi_setup();
        break;
      case 0xFF7A85: // 6
        currentBank = 5;
        update_eeprom();
        midi_setup();
        break;
      case 0xFF6A95: // 7
        currentBank = 6;
        update_eeprom();
        midi_setup();
        break;
      case 0xFF728D: // 8
        currentBank = 7;
        update_eeprom();
        midi_setup();
        break;
      case 0xFF4AB5: // 9
        currentBank = 8;
        update_eeprom();
        midi_setup();
        break;
      case 0xFFAA55: // 0
        currentBank = 9;
        update_eeprom();
        midi_setup();
        break;
      case 0xFFBA45:
        selectBank = !selectBank;
        break;
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
  return MD_Menu::NAV_NULL;;
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
