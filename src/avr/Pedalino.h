/*  __________           .___      .__  .__                   ___ ________________    ___
 *  \______   \ ____   __| _/____  |  | |__| ____   ____     /  / \__    ___/     \   \  \   
 *   |     ___// __ \ / __ |\__  \ |  | |  |/    \ /  _ \   /  /    |    | /  \ /  \   \  \  
 *   |    |   \  ___// /_/ | / __ \|  |_|  |   |  (  <_> ) (  (     |    |/    Y    \   )  )
 *   |____|    \___  >____ |(____  /____/__|___|  /\____/   \  \    |____|\____|__  /  /  /
 *                 \/     \/     \/             \/           \__\                 \/  /__/
 *                                                                (c) 2018 alf45star
 *                                                        https://github.com/alf45tar/Pedalino
 */


#ifndef _PEDALINO_H
#define _PEDALINO_H

#ifdef DEBUG_PEDALINO
#define SERIALDEBUG       Serial
#define DPRINT(v)         SERIALDEBUG.print(v)
#define DPRINTF(v)        SERIALDEBUG.print(F(v))
#define DPRINT2(v, f)     SERIALDEBUG.print(v, f)
#define DPRINTLN(v)       SERIALDEBUG.println(v)
#define DPRINTLNF(v)      SERIALDEBUG.println(F(v))
#define DPRINTLN2(v, f)   SERIALDEBUG.println(v, f)
#else
#define DPRINT(...)
#define DPRINTF(...)
#define DPRINT2(...)
#define DPRINTLN(...)
#define DPRINTLNF(...)
#define DPRINTLN2(...)
#endif

#include <EEPROM.h>                     // https://www.arduino.cc/en/Reference/EEPROM
#include <MIDI.h>                       // https://github.com/FortySevenEffects/arduino_midi_library
#include <MD_Menu.h>                    // https://github.com/MajicDesigns/MD_Menu
#include <MD_UISwitch.h>                // https://github.com/MajicDesigns/MD_UISwitch
#include <ResponsiveAnalogRead.h>       // https://github.com/dxinteractive/ResponsiveAnalogRead

#define DEBOUNCE_INTERVAL 20
#define BOUNCE_LOCK_OUT                 // This method is a lot more responsive, but does not cancel noise.
//#define BOUNCE_WITH_PROMPT_DETECTION  // Report accurate switch time normally with no delay. Use when accurate switch transition timing is important.
#include <Bounce2.h>                    // https://github.com/thomasfredericks/Bounce2

#include "MidiTimeCode.h"

#define PROFILES           3
#define BANKS             10

#if defined(__AVR_ATmega168__) || defined(__AVR_ATmega328P__)     // Arduino UNO, NANO
#define PEDALS             8
#elif defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)  // Arduino MEGA, MEGA2560
#define PEDALS            16
#endif

#define PIN_D(x)            23+2*x      // map 0..15 to 23,25,...53
#define PIN_A(x)            PIN_A0+x    // map 0..15 to A0, A1,...A15

#define INTERFACES          6

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
#define PED_LADDER          7

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
#define PED_CONTINUE        5
#define PED_TAP             6
#define PED_MENU            7
#define PED_CONFIRM         8
#define PED_ESCAPE          9
#define PED_NEXT            10
#define PED_PREVIOUS        11

#define PED_LINEAR          0
#define PED_LOG             1
#define PED_ANTILOG         2

#define PED_USBMIDI         0
#define PED_LEGACYMIDI      1
#define PED_APPLEMIDI       2   // also known as RTP-MIDI protocol
#define PED_IPMIDI          3
#define PED_BLUETOOTHMIDI   4
#define PED_OSC             5

#define PED_DISABLE         0
#define PED_ENABLE          1

#define PED_LEGACY_MIDI_OUT   0
#define PED_LEGACY_MIDI_IN    1
#define PED_LEGACY_MIDI_THRU  2

#define PED_MTC_NONE            0
#define PED_MTC_SLAVE           1
#define PED_MTC_MASTER_24       2
#define PED_MTC_MASTER_25       3
#define PED_MTC_MASTER_30DF     4
#define PED_MTC_MASTER_30       5
#define PED_MIDI_CLOCK_SLAVE    6
#define PED_MIDI_CLOCK_MASTER   7

#define PED_TIMESIGNATURE_2_4   0
#define PED_TIMESIGNATURE_4_4   1
#define PED_TIMESIGNATURE_3_4   2
#define PED_TIMESIGNATURE_3_8   3
#define PED_TIMESIGNATURE_6_8   4
#define PED_TIMESIGNATURE_9_8   5
#define PED_TIMESIGNATURE_12_8  6

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
  byte                   midiValue1;      /* Single click */
  byte                   midiValue2;      /* Double click */
  byte                   midiValue3;      /* Long click */
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
                                             3 = jog wheel
                                             4 = momentary 2
                                             5 = momentary 3
                                             6 = latch 2
                                             7 = ladder */
  byte                   pressMode;       /* 0 = single click
                                             1 = double click
                                             2 = long click
                                             3 = single and double click
                                             4 = single and long click
                                             5 = single, double and long click
                                             6 = double and long click */
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
byte   lastUsedSwitch         = 0xFF;
byte   lastUsedPedal          = 0xFF;
bool   selectBank             = true;
byte   currentMidiTimeCode    = PED_MTC_MASTER_24;
byte   timeSignature          = PED_TIMESIGNATURE_4_4;

MidiTimeCode  MTC;
unsigned int  bpm             = 120;

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


// The keys value that works for most LCD Keypad Shield

MD_UISwitch_Analog::uiAnalogKeys_t kt[] =
{
  {  10, 10, 'R' },  // Right
  { 130, 15, 'U' },  // Up
  { 305, 15, 'D' },  // Down
  { 475, 15, 'L' },  // Left
  { 720, 15, 'S' },  // Select
};


// LCD display definitions
//
// New LiquidCrystal library https://bitbucket.org/fmalpartida/new-liquidcrystal/wiki/Home is 5 times faster than Arduino standard LCD library
#include <LiquidCrystal.h>

#define  LCD_ROWS  2
#define  LCD_COLS  16

// LCD pin definitions

/*
#define  LCD_BACKLIGHT  44    // PWM works only on 2-13, 44, 45, 46
#define  LCD_CONTRAST   46    // PWM works only on 2-13, 44, 45, 46
#define  LCD_RS         50
#define  LCD_ENA        48
#define  LCD_D4         42
#define  LCD_D5         40
#define  LCD_D6         38
#define  LCD_D7         36
*/

// LCD Keypad Shield
// https://www.dfrobot.com/wiki/index.php/Arduino_LCD_KeyPad_Shield_(SKU:_DFR0009)

#define  LCD_RS         8
#define  LCD_ENA        9
#define  LCD_D4         4
#define  LCD_D5         5
#define  LCD_D6         6
#define  LCD_D7         7
//#define  LCD_BACKLIGHT  10    // Backlight control issue on D10
#define  LCD_BACKLIGHT  13      // PWM works only on 2-13, 44, 45, 46

//LiquidCrystal lcd(LCD_RS, LCD_ENA, LCD_D4, LCD_D5, LCD_D6, LCD_D7, LCD_BACKLIGHT, POSITIVE);
LiquidCrystal lcd(LCD_RS, LCD_ENA, LCD_D4, LCD_D5, LCD_D6, LCD_D7);
boolean       powersaver = false;
byte          backlight  = 150;

// IR Remote receiver

#include <IRremote.h>

#define RECV_PIN       24     // connect Y to this PIN, G to GND, R to 5V
#define RECV_LED_PIN   26

#define IR_ON_OFF   0xFFA25D
#define IR_OK       0xFF02FD
#define IR_ESC      0xFFE21D
#define IR_LEFT     0xFFC23D
#define IR_RIGHT    0xFF22DD
#define IR_UP       0xFFA857
#define IR_DOWN     0xFF629D
#define IR_SWITCH   0xFFB04F
#define IR_KEY_1    0xFF30CF
#define IR_KEY_2    0xFF18E7
#define IR_KEY_3    0xFF7A85
#define IR_KEY_4    0xFF10EF
#define IR_KEY_5    0xFF38C7
#define IR_KEY_6    0xFF5AA5
#define IR_KEY_7    0xFF42BD
#define IR_KEY_8    0xFF4AB5
#define IR_KEY_9    0xFF52AD
#define IR_KEY_0    0xFF6897

enum IRCODES {IRC_ON_OFF = 0,
              IRC_OK,
              IRC_ESC,
              IRC_LEFT,
              IRC_RIGHT,
              IRC_UP,
              IRC_DOWN,
              IRC_SWITCH,
              IRC_KEY_1,
              IRC_KEY_2,
              IRC_KEY_3,
              IRC_KEY_4,
              IRC_KEY_5,
              IRC_KEY_6,
              IRC_KEY_7,
              IRC_KEY_8,
              IRC_KEY_9,
              IRC_KEY_0,
              IR_CUSTOM_CODES
             };

#define REPEAT_TO_SKIP  4     // Number of REPEAT codes to skip before start repeat

IRrecv          irrecv(RECV_PIN, RECV_LED_PIN);
decode_results  results;
unsigned long   ircustomcode[IR_CUSTOM_CODES];

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

bool powerPlug     = false;
bool batteryLow    = false;
bool wifiConnected = false;
bool bleConnected  = false;

#define POWERPLUG     (byte)4
#define BATTERYLOW    (byte)5
#define WIFIICON      (byte)6
#define BLUETOOTHICON (byte)7

byte partial_bar[4][8] = {
  B10000,
  B10000,
  B10000,
  B10000,
  B10000,
  B10000,
  B10000,
  B10000,

  B11000,
  B11000,
  B11000,
  B11000,
  B11000,
  B11000,
  B11000,
  B11000,

  B11100,
  B11100,
  B11100,
  B11100,
  B11100,
  B11100,
  B11100,
  B11100,

  B11110,
  B11110,
  B11110,
  B11110,
  B11110,
  B11110,
  B11110,
  B11110
};

byte power_plug[] = {
  B01010,
  B01010,
  B11111,
  B10001,
  B10001,
  B01110,
  B00100,
  B00100
};

byte battery_low[] = {
  B01110,
  B11011,
  B10001,
  B10001,
  B10001,
  B10001,
  B10001,
  B11111
};

byte bluetooth_icon[] = {
  B00100,
  B00110,
  B10101,
  B01110,
  B01110,
  B10101,
  B00110,
  B00100
};

byte wifi_icon[] = {
  B00001,
  B00001,
  B00001,
  B00101,
  B00101,
  B10101,
  B10101,
  B10101
};

#endif // _PEDALINO_H

