#ifndef _PEDALINO_H
#define _PEDALINO_H

#define SIGNATURE "Pedalino(TM)"

#define BANKS             10

#if defined(__AVR_ATmega168__) || defined(__AVR_ATmega328P__)     // Arduino UNO, NANO
#define PEDALS             8
#elif defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)  // Arduino MEGA, MEGA2560
#define PEDALS            16
#endif

#define PIN_D(x)            23+2*x      // map 0..15 to 23,25,...53
#define PIN_A(x)            PIN_A0+x    // map 0..15 to A0, A1,...A15

#define INTERFACES          4

#define PED_PROGRAM_CHANGE  0
#define PED_CONTROL_CHANGE  1
#define PED_NOTE_ON_OFF     2
#define PED_PITCH_BEND      3

#define PED_MOMENTARY1      0
#define PED_LATCH1          1
#define PED_ANALOG          2
#define PED_JOG_WHEEL       3
#define PED_MOMENTARY2      4
#define PED_MOMENTARY3      5
#define PED_LATCH2          6

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
#define PED_START           3
#define PED_STOP            4
#define PED_TAP             5
#define PED_MENU            6
#define PED_CONFIRM         7
#define PED_ESCAPE          8
#define PED_NEXT            9
#define PED_PREVIOUS        10

#define PED_LINEAR          0
#define PED_LOG             1
#define PED_ANTILOG         2

#define PED_USBMIDI         0
#define PED_LEGACYMIDI      1
#define PED_APPLEMIDI       2   // also known as rtpMIDI protocol
#define PED_BLUETOOTHMIDI   3

#define PED_DISABLE         0
#define PED_ENABLE          1

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
  byte                   midiChannel;     /* MIDI channel 1-16 */
  byte                   midiCode;        /* Program Change, Control Code, Note or Pitch Bend value to send */
};

struct pedal {
  byte                   function;        /* 0 = MIDI
                                             1 = bank+
                                             2 = bank-
                                             3 = menu
                                             4 = confirm
                                             5 = escape
                                             6 = next
                                             7 = previous */
  byte                   autoSensing;     /* 0 = disable
                                             1 = enable   */
  byte                   mode;            /* 0 = momentary
                                             1 = latch
                                             2 = analog
                                             3 = jog wheel */
  byte                   pressMode;       /* 0 = single click
                                             1 = double click
                                             2 = long click
                                             3 = single and double click
                                             4 = single and long click
                                             5 = single, double and long click
                                             6 = double and long click */
  byte                   value_single;
  byte                   value_double;
  byte                   value_long;
  byte                   invertPolarity;
  byte                   mapFunction;
  int                    expZero;
  int                    expMax;
  int                    pedalValue[2];
  unsigned long          lastUpdate[2];         // last time the value is changed
  Bounce                *debouncer[2];
  MD_UISwitch           *footSwitch[2];
  ResponsiveAnalogRead  *analogPedal;
};

struct interface {
  byte                   midiIn;          // 0 = disable, 1 = enable
  byte                   midiOut;         // 0 = disable, 1 = enable
  byte                   midiThru;        // 0 = disable, 1 = enable
  byte                   midiRouting;     // 0 = disable, 1 = enable
  byte                   midiClock;       // 0 = disable, 1 = enable
};

bank      banks[BANKS][PEDALS];     // Banks Setup
pedal     pedals[PEDALS];           // Pedals Setup
interface interfaces[INTERFACES];   // Interfaces Setup

byte   currentProfile         = 0;
byte   currentBank            = 0;
byte   currentPedal           = 0;
byte   currentInterface       = PED_USBMIDI;
byte   currentLegacyMIDIPort  = PED_LEGACY_MIDI_OUT;
byte   currentWiFiMode        = PED_AP;
byte   lastUsedSwitch         = 0xFF;
byte   lastUsedPedal          = 0xFF;
bool   selectBank             = true;


// MIDI interfaces definition

struct USBSerialMIDISettings : public midi::DefaultSettings
{
  static const long BaudRate = 1000000;
};

struct RTPSerialMIDISettings : public midi::DefaultSettings
{
  static const long BaudRate = 115200;
};

MIDI_CREATE_CUSTOM_INSTANCE(HardwareSerial, Serial,  USB_MIDI, USBSerialMIDISettings);
MIDI_CREATE_INSTANCE(HardwareSerial, Serial2, DIN_MIDI);
MIDI_CREATE_CUSTOM_INSTANCE(HardwareSerial, Serial3, RTP_MIDI, RTPSerialMIDISettings);

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
#define RECV_LED_PIN   3

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

#endif

