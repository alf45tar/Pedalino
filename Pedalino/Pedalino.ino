#include <EEPROM.h>
#include <MIDI.h>
#include <MD_UISwitch.h>
#include <ResponsiveAnalogRead.h>
#include <MD_Menu.h>

// Bounce 2 library
// https://github.com/thomasfredericks/Bounce2/wiki
//
// "LOCK-OUT" debounce method
//#define BOUNCE_LOCK_OUT
// "BOUNCE_WITH_PROMPT_DETECTION" debounce method
//#define BOUNCE_WITH_PROMPT_DETECTION

#include <Bounce2.h>

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
#define PED_PITCH_BEND      3

#define PED_MOMENTARY       0
#define PED_LATCH           1
#define PED_ANALOG          2
#define PED_JOG_WHEEL       3

#define PED_PRESS_1         0
#define PED_PRESS_2         1
#define PED_PRESS_L         2
#define PED_PRESS_1_2       3
#define PED_PRESS_1_L       4
#define PED_PRESS_1_2_L     5
#define PED_PRESS_2_L       6

#define PED_MIDI            0
#define PED_BANK_PLUS       1
#define PED_BANK_MINUS      2
#define PED_MENU            3
#define PED_CONFIRM         4
#define PED_ESCAPE          5
#define PED_NEXT            6
#define PED_PREVIOUS        7

#define PED_LINEAR          0
#define PED_LOG             1
#define PED_ANTILOG         2

#define PED_USBMIDI         0
#define PED_MIDIOUT         1
#define PED_APPLEMIDI       2   // also known as rtpMIDI protocol
#define PED_MIDI_ALL_INT    3

#define PED_LEGACY_MIDI_OUT   0
#define PED_LEGACY_MIDI_IN    1
#define PED_LEGACY_MIDI_THRU  2


#define PED_STA             0   // wifi client station with smart config
#define PED_AP              1   // wifi access point

#define MIDI_RESOLUTION         128       // MIDI 7-bit CC resolution
#define ADC_RESOLUTION         1024       // 10-bit ADC converter resolution
#define CALIBRATION_DURATION   8000       // milliseconds

struct bank {
  byte                   midiMessage;     /* 0 = Program Change,
                                             1 = Control Code
                                             2 = Note On/Note Off
                                             3 = Pitch Bend */
  byte                   midiChannel;     // MIDI channel 1-16
  byte                   midiCode;        // Program Change, Control Code, Note or Pitch Bend value to send
};

struct pedal {
  byte                   function;        /* 0 = MIDI
                                             1 = Bank+
                                             2 = Bank- */
  byte                   mode;            /* 0 = momentary
                                             1 = latch
                                             2 = analog
                                             3 = jog wheel */
  byte                   pressMode;
  byte                   invertPolarity;
  byte                   mapFunction;
  int                    expZero;
  int                    expMax;
  int                    pedalValue;
  unsigned long          lastUpdate;         // last time the value is changed
  Bounce                *debouncer;
  MD_UISwitch           *footSwitch;
  ResponsiveAnalogRead  *analogPedal;
};

bank   banks[BANKS][PEDALS];     // Banks Setup
pedal  pedals[PEDALS];           // Pedals Setup
byte   currentBank            = 0;
byte   currentPedal           = 0;
byte   currentInterface       = PED_USBMIDI;
byte   currentLegacyMIDIPort  = PED_LEGACY_MIDI_OUT;
byte   currentWiFiMode        = PED_AP;
byte   lastUsedSwitch         = 0xFF;
byte   lastUsedPedal          = 0xFF;
bool   selectBank             = true;


// MIDI interfaces definition

struct SerialMIDISettings : public midi::DefaultSettings
{
  static const long BaudRate = 115200;
};

MIDI_CREATE_INSTANCE(HardwareSerial, Serial,  USB_MIDI);
MIDI_CREATE_INSTANCE(HardwareSerial, Serial2, DIN_MIDI);
MIDI_CREATE_CUSTOM_INSTANCE(HardwareSerial, Serial3, RTP_MIDI, SerialMIDISettings);

#include "MIDIRouting.h"


//
MD_UISwitch_Analog::uiAnalogKeys_t kt[] =
{
  {   50,  50, 'L' },  // Left
  {  512, 100, 'N' },  // Center
  { 1023,  50, 'R' },  // Right
};


// LCD display definitions
//
// New LiquidCrystal library https://bitbucket.org/fmalpartida/new-liquidcrystal/wiki/Home is 5 times faster than Arduino standard LCD library
#include <LiquidCrystal.h>

#define  LCD_ROWS  2
#define  LCD_COLS  16

// LCD pin definitions

#define  LCD_RS         46
#define  LCD_ENA        44
#define  LCD_D4         42
#define  LCD_D5         40
#define  LCD_D6         38
#define  LCD_D7         36
#define  LCD_BACKLIGHT  34

LiquidCrystal lcd(LCD_RS, LCD_ENA, LCD_D4, LCD_D5, LCD_D6, LCD_D7, LCD_BACKLIGHT, POSITIVE);
boolean       powersaver = false;

// IR Remote receiver

#include <IRremote.h>

#define RECV_PIN       2     // connect Y to this PIN, G to GND, R to 5V
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
#define bluetooth Serial1
#endif

// Software reset via watchdog

#include <avr/io.h>
#include <avr/wdt.h>

#define Reset_AVR() wdt_enable(WDTO_30MS); while(1) {}


const char bar1[]  = {49, 50, 51, 52, 53, 54, 55, 56, 57, 48};
const char bar2[]  = { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1};

//
//  Load factory deafult value for banks and pedals
//
void load_factory_default()
{
  for (byte b = 0; b < BANKS; b++)
    for (byte p = 0; p < PEDALS; p++)
      banks[b][p] = {PED_CONTROL_CHANGE, b + 1, p + 1};

  for (byte p = 0; p < PEDALS; p++)
    pedals[p] = {PED_MIDI, PED_MOMENTARY, PED_PRESS_1, 0, 0, 50, 930, 0, millis(), nullptr, nullptr, nullptr};

  pedals[0].mode = PED_ANALOG;
  pedals[1].mode = PED_ANALOG;
  pedals[2].mode = PED_ANALOG;
  pedals[14].function = PED_ESCAPE;
  pedals[15].function = PED_MENU;
}

//
//  Create new MIDI controllers setup
//
void controller_setup()
{
  // Delete previous setup
  for (byte i = 0; i < PEDALS; i++) {
    delete pedals[i].debouncer;
    delete pedals[i].footSwitch;
    delete pedals[i].analogPedal;
  }

  lastUsedSwitch = 0xFF;
  lastUsedPedal  = 0xFF;

#ifdef DEBUG_PEDALINO
  Serial.print("MIDI Interface ");
  switch (currentInterface) {
    case PED_USBMIDI:
      Serial.println("USB");
      break;
    case PED_MIDIOUT:
      Serial.println("MIDI OUT");
      break;
    case PED_APPLEMIDI:
      Serial.println("AppleMIDI");
      break;
  }
  Serial.print("Bank ");
  Serial.println(currentBank + 1);
#endif

  // Build new MIDI controllers setup
  for (byte i = 0; i < PEDALS; i++) {

#ifdef DEBUG_PEDALINO
    Serial.print("Pedal ");
    if (i < 9) Serial.print(" ");
    Serial.print(i + 1);
    Serial.print("     Function ");
    Serial.print(pedals[i].function);
    Serial.print("     Mode ");
    Serial.print(pedals[i].mode);
    Serial.print("     Press Mode ");
    Serial.print(pedals[i].pressMode);
    Serial.print("   Polarity ");
    Serial.print(pedals[i].invertPolarity);
    Serial.print("   Message ");
    Serial.print(banks[currentBank][i].midiMessage);
    Serial.print("   Channel ");
    Serial.print(banks[currentBank][i].midiChannel);
    Serial.print("   Code ");
    Serial.print(banks[currentBank][i].midiCode);
    Serial.println("");
#endif

    switch (pedals[i].mode) {

      case PED_MOMENTARY:
        unsigned int input;
        unsigned int value;
        pedals[i].debouncer = new Bounce();
        pedals[i].debouncer->attach(PIN_A0 + i);
        pedals[i].debouncer->interval(50);
        pedals[i].debouncer->update();
        input = pedals[i].debouncer->read();                                    // reads the updated pin state
        if (pedals[i].invertPolarity) input = (input == LOW) ? HIGH : LOW;      // invert the value
        value = map_digital(i, input);                                          // apply the digital map function to the value
        pedals[i].pedalValue = value;
        pedals[i].lastUpdate = millis();

        pedals[i].footSwitch = new MD_UISwitch_Digital(PIN_A0 + i, pedals[i].invertPolarity ? HIGH : LOW);
        pedals[i].footSwitch->begin();
        pedals[i].footSwitch->setDebounceTime(50);
        if (pedals[i].function == PED_MIDI) {
          switch (pedals[i].pressMode) {
            case PED_PRESS_1:
              pedals[i].footSwitch->enableDoublePress(false);
              pedals[i].footSwitch->enableLongPress(false);
              break;
            case PED_PRESS_2:
            case PED_PRESS_1_2:
              pedals[i].footSwitch->enableDoublePress(true);
              pedals[i].footSwitch->enableLongPress(false);
              break;
            case PED_PRESS_L:
            case PED_PRESS_1_L:
              pedals[i].footSwitch->enableDoublePress(false);
              pedals[i].footSwitch->enableLongPress(true);
              break;
            case PED_PRESS_1_2_L:
            case PED_PRESS_2_L:
              pedals[i].footSwitch->enableDoublePress(true);
              pedals[i].footSwitch->enableLongPress(true);
              break;
              pedals[i].footSwitch->enableDoublePress(true);
              pedals[i].footSwitch->enableLongPress(true);
              break;
          }
          pedals[i].footSwitch->setDoublePressTime(300);
          pedals[i].footSwitch->setLongPressTime(500);
          pedals[i].footSwitch->enableRepeat(false);
        }
        else
        {
          pedals[i].footSwitch->setDoublePressTime(300);
          pedals[i].footSwitch->setLongPressTime(500);
          pedals[i].footSwitch->setRepeatTime(500);
          pedals[i].footSwitch->enableRepeatResult(true);
        }
        break;

      case PED_LATCH:
        break;

      case PED_ANALOG:
        if (pedals[i].function == PED_MIDI) {
          pedals[i].analogPedal = new ResponsiveAnalogRead(PIN_A0 + i, true);
          pedals[i].analogPedal->setActivityThreshold(6.0);
          pedals[i].analogPedal->setAnalogResolution(MIDI_RESOLUTION);        // 7-bit MIDI resolution
          pedals[i].analogPedal->enableEdgeSnap();                            // ensures that values at the edges of the spectrum can be easily reached when sleep is enabled
          if (lastUsedPedal == 0xFF) lastUsedPedal = i;
        }
        else
          pedals[i].footSwitch = new MD_UISwitch_Analog(PIN_A0 + i, kt, ARRAY_SIZE(kt));
        break;

      case PED_JOG_WHEEL:
        break;
    }
  }
}


void midi_send(byte i, byte value, bool on_off = true )
{
  switch (banks[currentBank][i].midiMessage) {

    case PED_NOTE_ON_OFF:

      if (on_off) {
#ifdef DEBUG_PEDALINO
        Serial.print("     NOTE ON     Note ");
        Serial.print(banks[currentBank][i].midiCode);
        Serial.print("     Velocity ");
        Serial.print(value);
        Serial.print("     Channel ");
        Serial.println(banks[currentBank][i].midiChannel);
#else
        switch (currentInterface) {
          case PED_USBMIDI:
            USB_MIDI.sendNoteOn(banks[currentBank][i].midiCode, value, banks[currentBank][i].midiChannel);
            break;
          case PED_MIDIOUT:
            DIN_MIDI.sendNoteOn(banks[currentBank][i].midiCode, value, banks[currentBank][i].midiChannel);
            break;
          case PED_APPLEMIDI:
            RTP_MIDI.sendNoteOn(banks[currentBank][i].midiCode, value, banks[currentBank][i].midiChannel);
            break;
        }
#endif
      }
      else {
#ifdef DEBUG_PEDALINO
        Serial.print("     NOTE OFF    Note ");
        Serial.print(banks[currentBank][i].midiCode);
        Serial.print("     Velocity ");
        Serial.print(value);
        Serial.print("     Channel ");
        Serial.println(banks[currentBank][i].midiChannel);
#else
        switch (currentInterface) {
          case PED_USBMIDI:
            USB_MIDI.sendNoteOff(banks[currentBank][i].midiCode, value, banks[currentBank][i].midiChannel);
            break;
          case PED_MIDIOUT:
            DIN_MIDI.sendNoteOff(banks[currentBank][i].midiCode, value, banks[currentBank][i].midiChannel);
            break;
          case PED_APPLEMIDI:
            RTP_MIDI.sendNoteOff(banks[currentBank][i].midiCode, value, banks[currentBank][i].midiChannel);
            break;
        }
#endif
      }
      break;

    case PED_CONTROL_CHANGE:

#ifdef DEBUG_PEDALINO
      Serial.print("     CONTROL CHANGE     Code ");
      Serial.print(banks[currentBank][i].midiCode);
      Serial.print("     Value ");
      Serial.print(value);
      Serial.print("     Channel ");
      Serial.println(banks[currentBank][i].midiChannel);
#else
      switch (currentInterface) {
        case PED_USBMIDI:
          USB_MIDI.sendControlChange(banks[currentBank][i].midiCode, value, banks[currentBank][i].midiChannel);
          break;
        case PED_MIDIOUT:
          DIN_MIDI.sendControlChange(banks[currentBank][i].midiCode, value, banks[currentBank][i].midiChannel);
          break;
        case PED_APPLEMIDI:
          RTP_MIDI.sendControlChange(banks[currentBank][i].midiCode, value, banks[currentBank][i].midiChannel);
          break;
      }
#endif
      break;

    case PED_PROGRAM_CHANGE:

#ifdef DEBUG_PEDALINO
      Serial.print("     PROGRAM CHANGE     Program ");
      Serial.print(banks[currentBank][i].midiCode);
      Serial.print("     Channel ");
      Serial.println(banks[currentBank][i].midiChannel);
#else
      switch (currentInterface) {
        case PED_USBMIDI:
          USB_MIDI.sendProgramChange(banks[currentBank][i].midiCode, banks[currentBank][i].midiChannel);
          break;
        case PED_MIDIOUT:
          DIN_MIDI.sendProgramChange(banks[currentBank][i].midiCode, banks[currentBank][i].midiChannel);
          break;
        case PED_APPLEMIDI:
          RTP_MIDI.sendProgramChange(banks[currentBank][i].midiCode, banks[currentBank][i].midiChannel);
          break;
      }
#endif
      break;

    case PED_PITCH_BEND:

      int bend = value << 4;       // make it a 14-bit number (pad with 4 zeros)
#ifdef DEBUG_PEDALINO
      Serial.print("     PITCH BEND     Value ");
      Serial.print(bend);
      Serial.print("     Channel ");
      Serial.println(banks[currentBank][i].midiChannel);
#else
      switch (currentInterface) {
        case PED_USBMIDI:
          USB_MIDI.sendPitchBend(bend, banks[currentBank][i].midiChannel);
          break;
        case PED_MIDIOUT:
          DIN_MIDI.sendPitchBend(bend, banks[currentBank][i].midiChannel);
          break;
        case PED_APPLEMIDI:
          RTP_MIDI.sendPitchBend(bend, banks[currentBank][i].midiChannel);
          break;
      }
#endif
      break;
  }
}

//
//  MIDI messages refresh
//
void midi_refresh()
{
  MD_UISwitch::keyResult_t k;
  unsigned int input;
  unsigned int value;

  for (byte i = 0; i < PEDALS; i++) {
    if (pedals[i].function == PED_MIDI) {
      switch (pedals[i].mode) {

        case PED_MOMENTARY:

          switch (pedals[i].pressMode) {

            case PED_PRESS_1:
              if (pedals[i].debouncer->update())                                        // pin state changed
              {
                input = pedals[i].debouncer->read();                                    // reads the updated pin state
                if (pedals[i].invertPolarity) input = (input == LOW) ? HIGH : LOW;      // invert the value
                value = map_digital(i, input);                                          // apply the digital map function to the value

#ifdef DEBUG_PEDALINO
                Serial.print("Pedal ");
                if (i < 9) Serial.print(" ");
                Serial.print(i + 1);
                Serial.print("     raw ");
                Serial.print(input);
                Serial.print(" -> ");
                Serial.print(value);
                Serial.println(" sent     ");
#endif
                midi_send(i, 127, value == LOW);
                pedals[i].pedalValue = value;
                pedals[i].lastUpdate = millis();
                lastUsedSwitch = i;
                break;
              }
              break;

            case PED_PRESS_1_2:
            case PED_PRESS_1_L:
            case PED_PRESS_1_2_L:
            case PED_PRESS_2:
            case PED_PRESS_2_L:
            case PED_PRESS_L:
              if (pedals[i].footSwitch == nullptr) continue;      // sanity check
              k = pedals[i].footSwitch->read();
              switch (k) {

                case MD_UISwitch::KEY_PRESS:
                  midi_send(i, 127);
                  midi_send(i, 127, false);
                  break;

                case MD_UISwitch::KEY_DPRESS:
                  midi_send(i, 63);
                  midi_send(i, 63, false);
                  break;

                case MD_UISwitch::KEY_LONGPRESS:
                  midi_send(i, 0);
                  midi_send(i, 0, false);
                  break;
              }

              pedals[i].pedalValue = value;
              pedals[i].lastUpdate = millis();
              lastUsedSwitch = i;
          }
          break;

        case PED_ANALOG:

          if (pedals[i].analogPedal == nullptr) continue;           // sanity check

          input = analogRead(PIN_A0 + i);                           // read the raw analog input value
          if (pedals[i].invertPolarity) input = ADC_RESOLUTION - 1 - input;     // invert the scale
          value = map_analog(i, input);                             // apply the digital map function to the value
          value = value >> 3;                                       // map from 10-bit value [0, 1023] to the 7-bit MIDI value [0, 127]
          pedals[i].analogPedal->update(value);                     // update the responsive analog average
          if (pedals[i].analogPedal->hasChanged())                  // if the value changed since last time
          {
            value = pedals[i].analogPedal->getValue();              // get the responsive analog average value
            int velocity = (value - pedals[i].pedalValue) / (millis() - pedals[i].lastUpdate);
#ifdef DEBUG_PEDALINO
            Serial.print("Pedal ");
            if (i < 9) Serial.print(" ");
            Serial.print(i + 1);
            Serial.print("     raw ");
            Serial.print(input);
            Serial.print("->");
            Serial.print(value);
            Serial.println(" sent      ");
#endif
            midi_send(i, value, value > 63);
            pedals[i].pedalValue = value;
            pedals[i].lastUpdate = millis();
            lastUsedPedal = i;
          }
          break;
      }
    }
  }
}


//
// Calibration for analog controllers
//
void calibrate()
{
  unsigned long start = millis();

  // Clear display
  lcd.clear();
  lcd.setCursor(0, 0);

  // Display countdown bar
  for (int i = 1; i <= LCD_COLS; i++)
    lcd.print(char(B10100101));

  // Move expression pedal from min to max during CALIBRATION_DURATION
  pedals[currentPedal].expZero = ADC_RESOLUTION - 1;
  pedals[currentPedal].expMax = 0;

  while (millis() - start < CALIBRATION_DURATION) {

    // Read the current value and update min and max
    int ax = analogRead(PIN_A0 + currentPedal);
    pedals[currentPedal].expZero = min( pedals[currentPedal].expZero, ax + 20);
    pedals[currentPedal].expMax  = max( pedals[currentPedal].expMax, ax - 20);

    // Update countdown bar (1st row)
    lcd.setCursor(LCD_COLS - map(millis() - start, 0, CALIBRATION_DURATION, 0, LCD_COLS), 0);
    lcd.print(" ");

    // Update value bar (2nd row)
    lcd.setCursor(0, 1);
    lcd.print(pedals[currentPedal].expZero);
    for (int i = 1; i < LCD_COLS - floor(log10(pedals[currentPedal].expZero + 1)) - floor(log10(pedals[currentPedal].expMax + 1)) - 1; i++)
      lcd.print(" ");
    lcd.print(pedals[currentPedal].expMax);
  }
}

byte map_digital(byte p, byte value)
{
  p = constrain(p, 0, PEDALS - 1);
  return value;
}

unsigned int map_analog(byte p, unsigned int value)
{
  p = constrain(p, 0, PEDALS - 1);
  value = constrain(value, pedals[p].expZero, pedals[p].expMax);                  // make sure that the analog value is between the minimum and maximum value
  value = map(value, pedals[p].expZero, pedals[p].expMax, 0, ADC_RESOLUTION - 1); // map the value from [minimumValue, maximumValue] to [0, 1023]
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
  return value;
}

void screen_update(bool force = false) {

  static char screen1[LCD_COLS + 1];
  static char screen2[LCD_COLS + 1];

  if (!powersaver) {
    char buf[LCD_COLS + 1];
    // Line 1
    memset(buf, 0, sizeof(buf));
    for (byte i = 0; i < PEDALS; i++) {
      buf[i] = foot_char(i);
    }
    if (force || strcmp(screen1, buf) != 0) {
      memset(screen1, 0, LCD_COLS + 1);
      strncpy(screen1, buf, LCD_COLS);
      lcd.setCursor(0, 0);
      lcd.print(buf);
    }
    // Line 2
    memset(buf, 0, sizeof(buf));
    sprintf(&buf[strlen(buf)], "Bank%2d", currentBank + 1);
    //sprintf(&buf[strlen(buf)], "%2x%2x%2x", currentBank + 1, banks[currentBank][lastUsedSwitch].midiChannel, banks[currentBank][lastUsedSwitch].midiCode);
    if (lastUsedPedal >= 0 && lastUsedPedal < PEDALS) {
      strncpy(&buf[strlen(buf)], &bar2[0], map(pedals[lastUsedPedal].pedalValue, 0, MIDI_RESOLUTION - 1, 0, 10));
      strncpy(&buf[strlen(buf)], "          ", 10 - map(pedals[lastUsedPedal].pedalValue, 0, MIDI_RESOLUTION - 1, 0, 10));
    }
    if (force || strcmp(screen2, buf) != 0) {
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
  if (pedals[footswitch].function != PED_MIDI) return ' ';
  if (footswitch == lastUsedPedal || pedals[footswitch].mode == PED_MOMENTARY && pedals[footswitch].pedalValue == LOW) return bar1[footswitch % 10];
  return ' ';
}

//
//  Write current configuration to EEPROM (changes only)
//
void update_eeprom() {

  int offset = 0;

#ifdef DEBUG_PEDALINO
  Serial.print("Updating EEPROM ... ");
#endif

  EEPROM.put(offset, SIGNATURE);
  offset += sizeof(SIGNATURE);

  for (byte b = 0; b < BANKS; b++)
    for (byte p = 0; p < PEDALS; p++) {
      EEPROM.put(offset, banks[b][p].midiMessage);
      offset += sizeof(byte);
      EEPROM.put(offset, banks[b][p].midiChannel);
      offset += sizeof(byte);
      EEPROM.put(offset, banks[b][p].midiCode);
      offset += sizeof(byte);
    }

  for (byte p = 0; p < PEDALS; p++) {
    EEPROM.put(offset, pedals[p].function);
    offset += sizeof(byte);
    EEPROM.put(offset, pedals[p].mode);
    offset += sizeof(byte);
    EEPROM.put(offset, pedals[p].pressMode);
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
  EEPROM.put(offset, currentWiFiMode);
  offset += sizeof(byte);

#ifdef DEBUG_PEDALINO
  Serial.println("end.");
#endif

}

//
//  Read configuration from EEPROM
//
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

    for (byte b = 0; b < BANKS; b++)
      for (byte p = 0; p < PEDALS; p++) {
        EEPROM.get(offset, banks[b][p].midiMessage);
        offset += sizeof(byte);
        EEPROM.get(offset, banks[b][p].midiChannel);
        offset += sizeof(byte);
        EEPROM.get(offset, banks[b][p].midiCode);
        offset += sizeof(byte);
      }

    for (byte p = 0; p < PEDALS; p++) {
      EEPROM.get(offset, pedals[p].function);
      offset += sizeof(byte);
      EEPROM.get(offset, pedals[p].mode);
      offset += sizeof(byte);
      EEPROM.get(offset, pedals[p].pressMode);
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
    EEPROM.get(offset, currentWiFiMode);
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
#define II_MIDIMESSAGE    23
#define II_MIDICODE       24
#define II_MIDINOTE       25
#define II_FUNCTION       26
#define II_MODE           27
#define II_PRESS_MODE     28
#define II_POLARITY       29
#define II_CALIBRATE      30
#define II_ZERO           31
#define II_MAX            32
#define II_RESPONSECURVE  33
#define II_INTERFACE      34
#define II_LEGACY_MIDI    35
#define II_WIFI           36
#define II_DEFAULT        37

// Global menu data and definitions

MD_Menu::value_t vBuf;  // interface buffer for values

// Menu Headers --------
const PROGMEM MD_Menu::mnuHeader_t mnuHdr[] =
{
  { M_ROOT,       SIGNATURE,      10, 12, 0 },
  { M_BANKSETUP,  "Banks Setup",  20, 34, 0 },
  { M_PEDALSETUP, "Pedals Setup", 40, 48, 0 },
  { M_PROFILE,    "Options",      50, 53, 0 },
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
  { 32, "SetMIDIMessage",  MD_Menu::MNU_INPUT, II_MIDIMESSAGE },
  { 33, "Set MIDI Code",   MD_Menu::MNU_INPUT, II_MIDICODE },
  { 34, "Set MIDI Note",   MD_Menu::MNU_INPUT, II_MIDINOTE },
  // Pedals Setup
  { 40, "Select Pedal",    MD_Menu::MNU_INPUT, II_PEDAL },
  { 41, "Set Function",    MD_Menu::MNU_INPUT, II_FUNCTION },
  { 42, "Set Mode",        MD_Menu::MNU_INPUT, II_MODE },
  { 43, "Set Press Mode",  MD_Menu::MNU_INPUT, II_PRESS_MODE },
  { 44, "Set Polarity",    MD_Menu::MNU_INPUT, II_POLARITY },
  { 45, "Calibrate",       MD_Menu::MNU_INPUT, II_CALIBRATE },
  { 46, "Set Zero",        MD_Menu::MNU_INPUT, II_ZERO },
  { 47, "Set Max",         MD_Menu::MNU_INPUT, II_MAX },
  { 48, "Response Curve",  MD_Menu::MNU_INPUT, II_RESPONSECURVE },
  // Options
  { 50, "OutputInterface", MD_Menu::MNU_INPUT, II_INTERFACE },
  { 51, "LegacyMIDIPort",  MD_Menu::MNU_INPUT, II_LEGACY_MIDI },
  { 52, "WiFi Mode",       MD_Menu::MNU_INPUT, II_WIFI },
  { 53, "Factory default", MD_Menu::MNU_INPUT, II_DEFAULT }
};

// Input Items ---------
const PROGMEM char listMidiMessage[] =     "Program Change| Control Code |  Note On/Off |  Pitch Bend  ";
const PROGMEM char listPedalFunction[] =   "     MIDI     |    Bank +    |    Bank -    |     Menu     |    Confirm   |    Escape    |     Next     ";
const PROGMEM char listPedalMode[] =       "   Momentary  |     Latch    |    Analog    |   Jog Wheel  ";
const PROGMEM char listPedalPressMode[] =  "    Single    |    Double    |     Long     |      1+2     |      1+L     |     1+2+L    |      2+L     ";
const PROGMEM char listPolarity[] =        " No|Yes";
const PROGMEM char listResponseCurve[] =   "    Linear    |      Log     |   Anti-Log   ";
const PROGMEM char listOutputInterface[] = "     USB      |   MIDI OUT   |   AppleMIDI  |      All     ";
const PROGMEM char listLegacyMIDI[]      = "   MIDI OUT   |   MIDI IN    |   MIDI THRU  ";
const PROGMEM char listWiFiMode[] =        " Smart Config | Access Point ";
#include "ControlChange.h"
#include "NoteNumbers.h"

const PROGMEM MD_Menu::mnuInput_t mnuInp[] =
{
  { II_BANK,          ">1-10:      ", MD_Menu::INP_INT,   mnuValueRqst,  2, 1, 0,  BANKS, 0, 10, nullptr },
#if defined(__AVR_ATmega168__) || defined(__AVR_ATmega328P__)
  { II_PEDAL,         ">1-8:       ", MD_Menu::INP_INT,   mnuValueRqst,  2, 1, 0, PEDALS, 0, 10, nullptr },
#elif defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
  { II_PEDAL,         ">1-16:      ", MD_Menu::INP_INT,   mnuValueRqst,  2, 1, 0, PEDALS, 0, 10, nullptr },
#endif
  { II_MIDICHANNEL,   ">1-16:      ", MD_Menu::INP_INT,   mnuValueRqst,  2, 1, 0,                 16, 0, 10, nullptr },
  { II_MIDIMESSAGE,   ""            , MD_Menu::INP_LIST,  mnuValueRqst, 14, 0, 0,                  0, 0,  0, listMidiMessage },
  { II_MIDICODE,      ""            , MD_Menu::INP_LIST,  mnuValueRqst, 14, 0, 0,                  0, 0,  0, listMidiControlChange },
  { II_MIDINOTE,      ""            , MD_Menu::INP_LIST,  mnuValueRqst, 14, 0, 0,                  0, 0,  0, listMidiNoteNumbers },
  //  { II_MIDICODE,      "0-127:     " , MD_Menu::INP_INT,   mnuValueRqst,  3, 0, 0,    127, 0, 10, nullptr },
  { II_FUNCTION,      ""            , MD_Menu::INP_LIST,  mnuValueRqst, 14, 0, 0,                  0, 0,  0, listPedalFunction },
  { II_MODE,          ""            , MD_Menu::INP_LIST,  mnuValueRqst, 14, 0, 0,                  0, 0,  0, listPedalMode },
  { II_PRESS_MODE,    ""            , MD_Menu::INP_LIST,  mnuValueRqst, 14, 0, 0,                  0, 0,  0, listPedalPressMode },
  { II_POLARITY,      "Invert:    " , MD_Menu::INP_LIST,  mnuValueRqst,  3, 0, 0,                  0, 0,  0, listPolarity },
  { II_CALIBRATE,     "Confirm"     , MD_Menu::INP_RUN,   mnuValueRqst,  0, 0, 0,                  0, 0,  0, nullptr },
  { II_ZERO,          ">0-1023:  "  , MD_Menu::INP_INT,   mnuValueRqst,  4, 0, 0, ADC_RESOLUTION - 1, 0, 10, nullptr },
  { II_MAX,           ">0-1023:  "  , MD_Menu::INP_INT,   mnuValueRqst,  4, 0, 0, ADC_RESOLUTION - 1, 0, 10, nullptr },
  { II_RESPONSECURVE, ""            , MD_Menu::INP_LIST,  mnuValueRqst, 14, 0, 0,                  0, 0,  0, listResponseCurve },
  { II_INTERFACE,     ""            , MD_Menu::INP_LIST,  mnuValueRqst, 14, 0, 0,                  0, 0,  0, listOutputInterface },
  { II_LEGACY_MIDI,   ""            , MD_Menu::INP_LIST,  mnuValueRqst, 14, 0, 0,                  0, 0,  0, listLegacyMIDI },
  { II_WIFI,          ""            , MD_Menu::INP_LIST,  mnuValueRqst, 14, 0, 0,                  0, 0,  0, listWiFiMode },
  { II_DEFAULT,       "Confirm"     , MD_Menu::INP_RUN,   mnuValueRqst,  0, 0, 0,                  0, 0,  0, nullptr }
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

    case II_MIDIMESSAGE:
      if (bGet) vBuf.value = banks[currentBank][currentPedal].midiMessage;
      else banks[currentBank][currentPedal].midiMessage = vBuf.value;
      break;

    case II_MIDICODE:
    case II_MIDINOTE:
      if (bGet) vBuf.value = banks[currentBank][currentPedal].midiCode;
      else banks[currentBank][currentPedal].midiCode = vBuf.value;
      break;

    case II_FUNCTION:
      if (bGet) vBuf.value = pedals[currentPedal].function;
      else pedals[currentPedal].function = vBuf.value;
      break;

    case II_MODE:
      if (bGet) vBuf.value = pedals[currentPedal].mode;
      else pedals[currentPedal].mode = vBuf.value;
      break;

    case II_PRESS_MODE:
      if (bGet) vBuf.value = pedals[currentPedal].pressMode;
      else pedals[currentPedal].pressMode = vBuf.value;
      break;

    case II_POLARITY:
      if (bGet) vBuf.value = pedals[currentPedal].invertPolarity;
      else pedals[currentPedal].invertPolarity = vBuf.value;
      break;

    case II_CALIBRATE:
      if (!bGet && pedals[currentPedal].mode == PED_ANALOG) calibrate();
      r = nullptr;
      break;

    case II_ZERO:
      if (bGet)
        if (pedals[currentPedal].mode == PED_ANALOG)
          vBuf.value = pedals[currentPedal].expZero;
        else r = nullptr;
      else pedals[currentPedal].expZero = vBuf.value;
      break;

    case II_MAX:
      if (bGet)
        if (pedals[currentPedal].mode == PED_ANALOG)
          vBuf.value = pedals[currentPedal].expMax;
        else r = nullptr;
      else pedals[currentPedal].expMax = vBuf.value;
      break;

    case II_RESPONSECURVE:
      if (bGet)
        if (pedals[currentPedal].mode == PED_ANALOG)
          vBuf.value = pedals[currentPedal].mapFunction;
        else r = nullptr;
      else pedals[currentPedal].mapFunction = vBuf.value;
      break;

    case II_INTERFACE:
      if (bGet) vBuf.value = currentInterface;
      else {
        currentInterface = vBuf.value;

      }
      break;

    case II_LEGACY_MIDI:
      if (bGet) vBuf.value = currentLegacyMIDIPort;
      else currentLegacyMIDIPort = vBuf.value;
      break;

    case II_WIFI:
      if (bGet) vBuf.value = currentWiFiMode;
      else {
        currentWiFiMode = vBuf.value;
        switch (currentWiFiMode) {
          case PED_STA:
            RTP_MIDI.sendSysEx(5, "+STA");
            break;
          case PED_AP:
            RTP_MIDI.sendSysEx(4, "+AP");
            break;
        }
      }
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
    controller_setup();
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
      if (strcmp(msg, "Pedals Setup") == 0) {
        lcd.print("Pedal ");
        if (currentPedal < 9) lcd.print(" ");
        lcd.print(currentPedal + 1);
      }
      else if (strcmp(msg, "Banks Setup") == 0) {
        lcd.print("Bank ");
        if (currentPedal < 9) lcd.print(" ");
        lcd.print(currentBank + 1);
        lcd.print(" Pedal ");
        if (currentPedal < 9) lcd.print(" ");
        lcd.print(currentPedal + 1);
      }
      else
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

  MD_UISwitch::keyResult_t k;

  for (byte i = 0; i < PEDALS; i++) {
    if (pedals[i].footSwitch == nullptr) continue;
    if (pedals[i].function == PED_MIDI) continue;
    k = pedals[i].footSwitch->read();
    switch (k) {
      case MD_UISwitch::KEY_NULL:
        pedals[i].footSwitch->setDoublePressTime(300);
        pedals[i].footSwitch->setLongPressTime(500);
        pedals[i].footSwitch->setRepeatTime(500);
        pedals[i].footSwitch->enableDoublePress(true);
        pedals[i].footSwitch->enableLongPress(true);
        break;
      case MD_UISwitch::KEY_RPTPRESS:
        pedals[i].footSwitch->setDoublePressTime(0);
        pedals[i].footSwitch->setLongPressTime(0);
        pedals[i].footSwitch->setRepeatTime(10);
        pedals[i].footSwitch->enableDoublePress(false);
        pedals[i].footSwitch->enableLongPress(false);
      case MD_UISwitch::KEY_PRESS:
        switch (pedals[i].function) {
          case PED_MIDI:
            break;
          case PED_BANK_PLUS:
            currentBank++;
            break;
          case PED_BANK_MINUS:
            currentBank--;
            break;
          case PED_MENU:
            return MD_Menu::NAV_INC;
            break;
          case PED_CONFIRM:
            return MD_Menu::NAV_SEL;
            break;
          case PED_ESCAPE:
            return MD_Menu::NAV_ESC;
            break;
          case PED_NEXT:
            return MD_Menu::NAV_INC;
            break;
          case PED_PREVIOUS:
            return MD_Menu::NAV_DEC;
            break;
        }
        break;
      case MD_UISwitch::KEY_DPRESS:
        if (pedals[i].function == PED_MENU) return MD_Menu::NAV_SEL;
        break;
      case MD_UISwitch::KEY_LONGPRESS:
        if (pedals[i].function == PED_MENU) return MD_Menu::NAV_ESC;
        break;
      default:
        break;
    }
    if (k != MD_UISwitch::KEY_NULL) {
      switch (pedals[i].footSwitch->getKey()) {
        case 'L':
          return MD_Menu::NAV_INC;
          break;
        case 'R':
          return MD_Menu::NAV_DEC;
          break;
      }
    }
  }

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
      controller_setup();
    }
    else {
      pedals[numberPressed - 1].pedalValue = LOW;
      //pedals[numberPressed - 1].midiController->push();
      screen_update();
      delay(200);
      pedals[numberPressed - 1].pedalValue = HIGH;
      //pedals[numberPressed - 1].midiController->release();
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
  // Initiate serial MIDI communications, listen to all channels
  midi_routing_start();
#ifdef DEBUG_PEDALINO
  Serial.begin(115200);
#else
  USB_MIDI.begin(MIDI_CHANNEL_OMNI);
#endif
  DIN_MIDI.begin(MIDI_CHANNEL_OMNI);
  RTP_MIDI.begin(MIDI_CHANNEL_OMNI);

  read_eeprom();
  controller_setup();

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
  if (prevMenuRun && !M.isInMenu())
    screen_update(true);
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
  else lcd.noCursor();

  M.runMenu();   // just run the menu code

  // Check whether the input has changed since last time, if so, send the new value over MIDI
  midi_refresh();
}


