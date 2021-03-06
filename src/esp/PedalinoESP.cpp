/*  __________           .___      .__  .__                   ___ ________________    ___
 *  \______   \ ____   __| _/____  |  | |__| ____   ____     /  / \__    ___/     \   \  \   
 *   |     ___// __ \ / __ |\__  \ |  | |  |/    \ /  _ \   /  /    |    | /  \ /  \   \  \  
 *   |    |   \  ___// /_/ | / __ \|  |_|  |   |  (  <_> ) (  (     |    |/    Y    \   )  )
 *   |____|    \___  >____ |(____  /____/__|___|  /\____/   \  \    |____|\____|__  /  /  /
 *                 \/     \/     \/             \/           \__\                 \/  /__/
 *                                                                (c) 2018 alf45star
 *                                                        https://github.com/alf45tar/Pedalino
 */

/*
    ESP8266/ESP32 MIDI Gateway between

      - Serial MIDI
      - WiFi AppleMIDI a.k.a. RTP-MIDI a.k.a. Network MIDI
      - ipMIDI
      - Bluetooth LE MIDI
      - WiFi OSC
*/

#include <Arduino.h>
#include <ArduinoJson.h>
#include <EEPROM.h>
#include <MIDI.h>

#ifdef ARDUINO_ARCH_ESP8266
#define NOBLE
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ESP8266LLMNR.h>
#include <ESP8266HTTPUpdateServer.h>

#ifdef BLYNK
#include <BlynkSimpleEsp8266.h>
#endif
#endif  // ARDUINO_ARCH_ESP8266

#ifdef ARDUINO_ARCH_ESP32
#include <esp_log.h>
#include <string>

#ifndef NOWIFI
#include <WiFi.h>
#include <ESPmDNS.h>
#endif

#ifndef NOBLE
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#endif

#ifdef BLYNK
#include <BlynkSimpleEsp32.h>
#endif

#define NOWEBCONFIG   // temporary
#endif  // ARDUINO_ARCH_ESP32

#ifndef NOWIFI
#include <WiFiClient.h>
#include <WiFiUdp.h>

#include <AppleMidi.h>
#include <OSCMessage.h>
#include <OSCBundle.h>
#include <OSCData.h>
#endif

#ifdef PEDALINO_TELNET_DEBUG
#include <RemoteDebug.h>          // Remote debug over telnet - not recommended for production, only for development    
RemoteDebug Debug;
#endif

#define WIFI_CONNECT_TIMEOUT    10
#define SMART_CONFIG_TIMEOUT    30

#if defined(ARDUINO_ARCH_ESP8266) && defined(DEBUG_ESP_PORT)
#define SERIALDEBUG       DEBUG_ESP_PORT
#define DPRINT(...)       DEBUG_ESP_PORT.printf( __VA_ARGS__ )
#define DPRINTLN(...)     DEBUG_ESP_PORT.printf( __VA_ARGS__ ); DEBUG_ESP_PORT.println();
#endif

#ifdef ARDUINO_ARCH_ESP32
#define SERIALDEBUG       Serial
#define LOG_TAG           "PedalinoESP"
#define DPRINT(...)       ESP_LOGI(LOG_TAG, __VA_ARGS__)
#define DPRINTLN(...)     ESP_LOGI(LOG_TAG, __VA_ARGS__)
#endif

#ifdef PEDALINO_TELNET_DEBUG
#define DPRINT(...)       rdebugI(__VA_ARGS__)
#define DPRINTLN(...)     rdebugIln(__VA_ARGS__)
#endif

#ifdef ARDUINO_ARCH_ESP8266
#define BLE_LED_OFF()
#define BLE_LED_ON()
#define WIFI_LED       2      // Onboard LED on GPIO2 is shared with Serial1 TX
#define WIFI_LED_OFF() digitalWrite(WIFI_LED, HIGH)
#define WIFI_LED_ON()  digitalWrite(WIFI_LED, LOW)
#endif

#ifdef ARDUINO_ARCH_ESP32
#define BLE_LED         21
#define WIFI_LED        19
#define BLE_LED_OFF()   digitalWrite(BLE_LED, LOW)
#define BLE_LED_ON()    digitalWrite(BLE_LED, HIGH)
#define WIFI_LED_OFF()  digitalWrite(WIFI_LED, LOW)
#define WIFI_LED_ON()   digitalWrite(WIFI_LED, HIGH)
#endif

#ifndef LED_BUILTIN
#define LED_BUILTIN    2
#endif

#ifndef WIFI_LED
#define WIFI_LED        LED_BUILTIN  // onboard LED, used as status indicator
#endif

#ifndef BLE_LED
#define BLE_LED         LED_BUILTIN  // onboard LED, used as status indicator
#endif

#ifndef DPRINT
#define DPRINT(...)
#endif

#ifndef DPRINTLN
#define DPRINTLN(...)
#define DPRINTMIDI(...)
#else
#define DPRINTMIDI(...)   printMIDI(__VA_ARGS__)
#endif

const char host[]           = "pedalino";

#ifdef BLYNK
const char blynkAuthToken[] = "63c670c13d334b059b9dbc9a0b690f4b";
WidgetLCD  blynkLCD(V0);
#endif

#ifdef ARDUINO_ARCH_ESP8266
ESP8266WebServer        httpServer(80);
ESP8266HTTPUpdateServer httpUpdater;
#endif

// Bluetooth LE MIDI interface

#if defined(ARDUINO_ARCH_ESP32) && !defined(NOBLE)

#define MIDI_SERVICE_UUID        "03b80e5a-ede8-4b33-a751-6ce34ec4c700"
#define MIDI_CHARACTERISTIC_UUID "7772e5db-3868-4112-a1a9-f2669d106bf3"

BLEServer             *pServer;
BLEService            *pService;
BLEAdvertising        *pAdvertising;
BLECharacteristic     *pCharacteristic;
BLESecurity           *pSecurity;
#endif
bool                  bleMidiConnected = false;
unsigned long         bleLastOn        = 0;

// WiFi MIDI interface to comunicate with AppleMIDI/RTP-MDI devices

#ifndef NOWIFI
APPLEMIDI_CREATE_INSTANCE(WiFiUDP, AppleMIDI); // see definition in AppleMidi_Defs.h
#endif

bool          appleMidiConnected = false;
unsigned long wifiLastOn         = 0;

// Serial MIDI interface to comunicate with Arduino

#define SERIALMIDI_BAUD_RATE  115200

struct SerialMIDISettings : public midi::DefaultSettings
{
  static const long BaudRate = SERIALMIDI_BAUD_RATE;
};

#ifdef ARDUINO_ARCH_ESP8266
#define SerialMIDI            Serial
#endif

#ifdef ARDUINO_ARCH_ESP32
#define SERIALMIDI_RX         16
#define SERIALMIDI_TX         17
HardwareSerial                SerialMIDI(2);
#endif

MIDI_CREATE_CUSTOM_INSTANCE(HardwareSerial, SerialMIDI, MIDI, SerialMIDISettings);

// ipMIDI

#ifndef NOWIFI
WiFiUDP                 ipMIDI;
IPAddress               ipMIDImulticast(225, 0, 0, 37);
unsigned int            ipMIDIdestPort = 21928;
#endif

// WiFi OSC comunication

#ifndef NOWIFI
WiFiUDP                 oscUDP;                  // A UDP instance to let us send and receive packets over UDP
IPAddress               oscRemoteIp;             // remote IP of an external OSC device or broadcast address
const unsigned int      oscRemotePort = 9000;    // remote port of an external OSC device
const unsigned int      oscLocalPort = 8000;     // local port to listen for OSC packets (actually not used for sending)
OSCMessage              oscMsg;
#endif

// Interfaces

#define INTERFACES          6

#define PED_USBMIDI         0
#define PED_DINMIDI         1
#define PED_RTPMIDI         2
#define PED_IPMIDI          3
#define PED_BLEMIDI         4
#define PED_OSC             5

struct interface {
  char                   name[4];
  byte                   midiIn;          // 0 = disable, 1 = enable
  byte                   midiOut;         // 0 = disable, 1 = enable
  byte                   midiThru;        // 0 = disable, 1 = enable
  byte                   midiRouting;     // 0 = disable, 1 = enable
  byte                   midiClock;       // 0 = disable, 1 = enable
};

interface interfaces[INTERFACES] = { "USB", 1, 1, 0, 1, 0,
                                     "DIN", 1, 1, 0, 1, 0,
                                     "RTP", 1, 1, 0, 1, 0,
                                     "IP ", 1, 1, 0, 1, 0,
                                     "BLE", 1, 1, 0, 1, 0,                                     
                                     "OSC", 1, 1, 0, 1, 0 };   // Interfaces Setup


void wifi_connect();

void serialize_on() {

  StaticJsonBuffer<100> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();

  root["on"] = true;
  
  String jsonString;
  root.printTo(jsonString);
  DPRINTLN("%s", jsonString.c_str());
  MIDI.sendSysEx(jsonString.length(), (byte *)(jsonString.c_str()));
}

void serialize_wifi_status(bool status) {

  StaticJsonBuffer<100> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();

  root["wifi.connected"] = status;
  
  String jsonString;
  root.printTo(jsonString);
  DPRINTLN("%s", jsonString.c_str());
  MIDI.sendSysEx(jsonString.length(), (byte *)(jsonString.c_str()));
}

void serialize_ble_status(bool status) {

  StaticJsonBuffer<100> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();

  root["ble.connected"] = status;
  
  char jsonString[50];
  root.printTo(jsonString);
  DPRINTLN("%s", jsonString);
  MIDI.sendSysEx(strlen(jsonString), (byte *)jsonString);
}

void save_wifi_credentials(String ssid, String password)
{
#ifdef ARDUINO_ARCH_ESP32
  int address = 32;
  EEPROM.writeString(address, ssid);
  address += ssid.length() + 1;
  EEPROM.writeString(address, password);
  EEPROM.commit();
  DPRINTLN("WiFi credentials saved into EEPROM");
#endif
}

void printMIDI(const char *interface, midi::StatusByte status, const byte *data)
{
  midi::MidiType  type;
  midi::Channel   channel;
  byte            note, velocity, pressure, number, value;
  int             bend;
  unsigned int    beats;

    type    = MIDI.getTypeFromStatusByte(status);
    channel = MIDI.getChannelFromStatusByte(status);

    switch(type) {
     
      case midi::NoteOff:
        note     = data[0];
        velocity = data[1];
        DPRINTLN("Received from %s  NoteOff 0x%02X   Velocity 0x%02X   Channel %02d", interface, note, velocity, channel);
        break;

      case midi::NoteOn:
        note     = data[0];
        velocity = data[1];
        DPRINTLN("Received from %s  NoteOn  0x%02X   Velocity 0x%02X   Channel %02d", interface, note, velocity, channel);
        break;

      case midi::AfterTouchPoly:
        note     = data[0];
        pressure = data[1];
        DPRINTLN("Received from %s  AfterTouchPoly   Note 0x%02X   Pressure 0x%02X   Channel %02d", interface, note, pressure, channel);
        break;

      case midi::ControlChange:
        number  = data[0];
        value   = data[1];
        DPRINTLN("Received from %s  ControlChange 0x%02X   Value 0x%02X   Channel %02d", interface, number, value, channel);
        break;

      case midi::ProgramChange:
        number  = data[0];
        DPRINTLN("Received from %s  ProgramChange 0x%02X   Channel %02d", interface, number, channel);
        break;

      case midi::AfterTouchChannel:    
        pressure = data[0];
        DPRINTLN("Received from %s  AfterTouchChannel   Pressure 0x%02X   Channel %02d", interface, pressure, channel);
        break;

      case midi::PitchBend:
        bend = data[1] << 7 | data[0];
        DPRINTLN("Received from %s  PitchBend   Bend 0x%02X   Channel %02d", interface, bend, channel);
        break;

      case 0xf0:
        switch(status) {

          case midi::SystemExclusive:
            DPRINTLN("Received from %s  SystemExclusive 0x%02X", interface, data[0]);
            break;

          case midi::TimeCodeQuarterFrame:
            value = data[0];
            DPRINTLN("Received from %s  TimeCodeQuarterFrame 0x%02X", interface, value);
            break;

          case midi::SongPosition:
            beats = data[1] << 7 | data[0];
            DPRINTLN("Received from %s  SongPosition Beats 0x%04X", interface, beats);
            break;

          case midi::SongSelect:
            number = data[0];
            DPRINTLN("Received from %s  SongSelect 0x%02X", interface, number);
            break;

          case midi::TuneRequest:
            DPRINTLN("Received from %s  TuneRequest", interface);
            break;

          case midi::Clock:
            //DPRINTLN("Received from %s  Clock", interface);
            break;

          case midi::Start:
            DPRINTLN("Received from %s  Start", interface);
            break;
          
          case midi::Continue:
            DPRINTLN("Received from %s  Continue", interface);
            break;

          case midi::Stop:
            DPRINTLN("Received from %s  Stop", interface);
            break;

          case midi::ActiveSensing:
            DPRINTLN("Received from %s  ActiveSensing", interface);
            break;

          case midi::SystemReset:
            DPRINTLN("Received from %s  SystemReset", interface);
            break;
        }
        break;
      
      default:
        break;
    }
}

void printMIDI (const char *interface, const midi::MidiType type, const midi::Channel channel, const byte data1, const byte data2)
{ 
  midi::StatusByte  status;
  byte              data[2];

  status = type | ((channel - 1) & 0x0f);
  data[0] = data1;
  data[1] = data2;
  printMIDI(interface, status, data);
}

#ifdef NOBLE
#define BLEMidiReceive(...)
#define BLESendNoteOn(...)
#define BLESendNoteOff(...)
#define BLESendAfterTouchPoly(...)
#define BLESendControlChange(...)
#define BLESendProgramChange(...)
#define BLESendAfterTouch(...)
#define BLESendPitchBend(...)
#define BLESendSystemExclusive(...)
#define BLESendTimeCodeQuarterFrame(...)
#define BLESendSongPosition(...)
#define BLESendSongSelect(...)
#define BLESendTuneRequest(...)
#define BLESendClock(...)
#define BLESendStart(...)
#define BLESendContinue(...)
#define BLESendStop(...) {}
#define BLESendActiveSensing(...)
#define BLESendSystemReset(...)
#else
void BLEMidiReceive(uint8_t *, uint8_t);

class MyBLEServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      bleMidiConnected = true;
      serialize_ble_status(true);
      DPRINT("BLE client connected");
    };

    void onDisconnect(BLEServer* pServer) {
      bleMidiConnected = false;
      serialize_ble_status(false);
      DPRINT("BLE client disconnected");
    }
};

class MyBLECharateristicCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      std::string rxValue = pCharacteristic->getValue();
      if (rxValue.length() > 0)
        if (interfaces[PED_BLEMIDI].midiIn) {
          BLEMidiReceive((uint8_t *)(rxValue.c_str()), rxValue.length());
          DPRINT("BLE Received %2d bytes", rxValue.length());
        }        
    }
};

void ble_midi_start_service ()
{
  BLEDevice::init("Pedal");
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyBLEServerCallbacks());

  pService = pServer->createService(MIDI_SERVICE_UUID);
  pCharacteristic = pService->createCharacteristic(MIDI_CHARACTERISTIC_UUID,
                    BLECharacteristic::PROPERTY_READ   |
                    BLECharacteristic::PROPERTY_NOTIFY |
                    BLECharacteristic::PROPERTY_WRITE_NR);

  pCharacteristic->setCallbacks(new MyBLECharateristicCallbacks());

  pCharacteristic->addDescriptor(new BLE2902());
  pService->start();

  pAdvertising = pServer->getAdvertising();
  pAdvertising->addServiceUUID(pService->getUUID());
  pAdvertising->start();

  pSecurity = new BLESecurity();
  pSecurity->setAuthenticationMode(ESP_LE_AUTH_REQ_SC_BOND);
}

void BLEMidiTimestamp (uint8_t *header, uint8_t *timestamp)
{
  /*
    The first byte of all BLE packets must be a header byte. This is followed by timestamp bytes and MIDI messages.

    Header Byte
      bit 7     Set to 1.
      bit 6     Set to 0. (Reserved for future use)
      bits 5-0  timestampHigh:Most significant 6 bits of timestamp information.
    The header byte contains the topmost 6 bits of timing information for MIDI events in the BLE
    packet. The remaining 7 bits of timing information for individual MIDI messages encoded in a
    packet is expressed by timestamp bytes.
    Timestamp Byte
    bit 7       Set to 1.
    bits 6-0    timestampLow: Least Significant 7 bits of timestamp information.
    The 13-bit timestamp for the first MIDI message in a packet is calculated using 6 bits from the
    header byte and 7 bits from the timestamp byte.
    Timestamps are 13-bit values in milliseconds, and therefore the maximum value is 8,191 ms.
    Timestamps must be issued by the sender in a monotonically increasing fashion.
    timestampHigh is initially set using the lower 6 bits from the header byte while the timestampLow is
    formed of the lower 7 bits from the timestamp byte. Should the timestamp value of a subsequent
    MIDI message in the same packet overflow/wrap (i.e., the timestampLow is smaller than a
    preceding timestampLow), the receiver is responsible for tracking this by incrementing the
    timestampHigh by one (the incremented value is not transmitted, only understood as a result of the
    overflow condition).
    In practice, the time difference between MIDI messages in the same BLE packet should not span
    more than twice the connection interval. As a result, a maximum of one overflow/wrap may occur
    per BLE packet.
    Timestamps are in the sender’s clock domain and are not allowed to be scheduled in the future.
    Correlation between the receiver’s clock and the received timestamps must be performed to
    ensure accurate rendering of MIDI messages, and is not addressed in this document.
  */
  /*
    Calculating a Timestamp
    To calculate the timestamp, the built-in millis() is used.
    The BLE standard only specifies 13 bits worth of millisecond data though,
    so it’s bitwise anded with 0x1FFF for an ever repeating cycle of 13 bits.
    This is done right after a MIDI message is detected. It’s split into a 6 upper bits, 7 lower bits,
    and the MSB of both bytes are set to indicate that this is a header byte.
    Both bytes are placed into the first two position of an array in preparation for a MIDI message.
  */
  unsigned long currentTimeStamp = millis() & 0x01FFF;

  *header = ((currentTimeStamp >> 7) & 0x3F) | 0x80;        // 6 bits plus MSB
  *timestamp = (currentTimeStamp & 0x7F) | 0x80;            // 7 bits plus MSB
}


// Decodes the BLE characteristics and calls MIDI.send if the packet contains sendable MIDI data
// https://learn.sparkfun.com/tutorials/midi-ble-tutorial

void BLEMidiReceive(uint8_t *buffer, uint8_t bufferSize)
{
  /*
    The general form of a MIDI message follows:

    n-byte MIDI Message
      Byte 0            MIDI message Status byte, Bit 7 is Set to 1.
      Bytes 1 to n-1    MIDI message Data bytes, if n > 1. Bit 7 is Set to 0
    There are two types of MIDI messages that can appear in a single packet: full MIDI messages and
    Running Status MIDI messages. Each is encoded differently.
    A full MIDI message is simply the MIDI message with the Status byte included.
    A Running Status MIDI message is a MIDI message with the Status byte omitted. Running Status
    MIDI messages may only be placed in the data stream if the following criteria are met:
    1.  The original MIDI message is 2 bytes or greater and is not a System Common or System
        Real-Time message.
    2.  The omitted Status byte matches the most recently preceding full MIDI message’s Status
        byte within the same BLE packet.
    In addition, the following rules apply with respect to Running Status:
    1.  A Running Status MIDI message is allowed within the packet after at least one full MIDI
        message.
    2.  Every MIDI Status byte must be preceded by a timestamp byte. Running Status MIDI
        messages may be preceded by a timestamp byte. If a Running Status MIDI message is not
        preceded by a timestamp byte, the timestamp byte of the most recently preceding message
        in the same packet is used.
    3.  System Common and System Real-Time messages do not cancel Running Status if
        interspersed between Running Status MIDI messages. However, a timestamp byte must
        precede the Running Status MIDI message that follows.
    4.  The end of a BLE packet does cancel Running Status.
    In the MIDI 1.0 protocol, System Real-Time messages can be sent at any time and may be
    inserted anywhere in a MIDI data stream, including between Status and Data bytes of any other
    MIDI messages. In the MIDI BLE protocol, the System Real-Time messages must be deinterleaved
    from other messages – except for System Exclusive messages.
  */
  midi::Channel   channel;
  midi::MidiType  command;

  //Pointers used to search through payload.
  uint8_t lPtr = 0;
  uint8_t rPtr = 0;
  //Decode first packet -- SHALL be "Full MIDI message"
  lPtr = 2; //Start at first MIDI status -- SHALL be "MIDI status"
  //While statement contains incrementing pointers and breaks when buffer size exceeded.
  while (1) {
    //lastStatus used to capture runningStatus
    uint8_t lastStatus = buffer[lPtr];
    if ( (buffer[lPtr] < 0x80) ) {
      //Status message not present, bail
      return;
    }
    command = MIDI.getTypeFromStatusByte(lastStatus);
    channel = MIDI.getChannelFromStatusByte(lastStatus);
    //Point to next non-data byte
    rPtr = lPtr;
    while ( (buffer[rPtr + 1] < 0x80) && (rPtr < (bufferSize - 1)) ) {
      rPtr++;
    }
    //look at l and r pointers and decode by size.
    if ( rPtr - lPtr < 1 ) {
      //Time code or system
      MIDI.send(command, 0, 0, channel);
    } else if ( rPtr - lPtr < 2 ) {
      MIDI.send(command, buffer[lPtr + 1], 0, channel);
    } else if ( rPtr - lPtr < 3 ) {
      MIDI.send(command, buffer[lPtr + 1], buffer[lPtr + 2], channel);
    } else {
      //Too much data
      //If not System Common or System Real-Time, send it as running status
      switch ( buffer[lPtr] & 0xF0 )
      {
        case 0x80:
        case 0x90:
        case 0xA0:
        case 0xB0:
        case 0xE0:
          for (int i = lPtr; i < rPtr; i = i + 2) {
            MIDI.send(command, buffer[i + 1], buffer[i + 2], channel);
          }
          break;
        case 0xC0:
        case 0xD0:
          for (int i = lPtr; i < rPtr; i = i + 1) {
            MIDI.send(command, buffer[i + 1], 0, channel);
          }
          break;
        default:
          break;
      }
    }
    //Point to next status
    lPtr = rPtr + 2;
    if (lPtr >= bufferSize) {
      //end of packet
      return;
    }
  }
}

void BLESendChannelMessage1(byte type, byte channel, byte data1)
{
  uint8_t midiPacket[4];

  if (!interfaces[PED_BLEMIDI].midiOut) return;

  BLEMidiTimestamp(&midiPacket[0], &midiPacket[1]);
  midiPacket[2] = (type & 0xf0) | ((channel - 1) & 0x0f);
  midiPacket[3] = data1;
  pCharacteristic->setValue(midiPacket, 4);
  pCharacteristic->notify();
}

void BLESendChannelMessage2(byte type, byte channel, byte data1, byte data2)
{
  uint8_t midiPacket[5];

  if (!interfaces[PED_BLEMIDI].midiOut) return;

  BLEMidiTimestamp(&midiPacket[0], &midiPacket[1]);
  midiPacket[2] = (type & 0xf0) | ((channel - 1) & 0x0f);
  midiPacket[3] = data1;
  midiPacket[4] = data2;
  pCharacteristic->setValue(midiPacket, 5);
  pCharacteristic->notify();
}

void BLESendSystemCommonMessage1(byte type, byte data1)
{
  uint8_t midiPacket[4];

  if (!interfaces[PED_BLEMIDI].midiOut) return;
 
  BLEMidiTimestamp(&midiPacket[0], &midiPacket[1]);
  midiPacket[2] = type;
  midiPacket[3] = data1;
  pCharacteristic->setValue(midiPacket, 4);
  pCharacteristic->notify();
}

void BLESendSystemCommonMessage2(byte type, byte data1, byte data2)
{
  uint8_t midiPacket[5];

  if (!interfaces[PED_BLEMIDI].midiOut) return;
 
  BLEMidiTimestamp(&midiPacket[0], &midiPacket[1]);
  midiPacket[2] = type;
  midiPacket[3] = data1;
  midiPacket[4] = data2;
  pCharacteristic->setValue(midiPacket, 5);
  pCharacteristic->notify();
}

void BLESendRealTimeMessage(byte type)
{
  uint8_t midiPacket[3];

  if (!interfaces[PED_BLEMIDI].midiOut) return;
 
  BLEMidiTimestamp(&midiPacket[0], &midiPacket[1]);
  midiPacket[2] = type;
  pCharacteristic->setValue(midiPacket, 3);
  pCharacteristic->notify();
}

void BLESendNoteOn(byte note, byte velocity, byte channel)
{
  BLESendChannelMessage2(midi::NoteOn, channel, note, velocity);
}

void BLESendNoteOff(byte note, byte velocity, byte channel)
{
  BLESendChannelMessage2(midi::NoteOff, channel, note, velocity);
}

void BLESendAfterTouchPoly(byte note, byte pressure, byte channel)
{
  BLESendChannelMessage2(midi::AfterTouchPoly, channel, note, pressure);
}

void BLESendControlChange(byte number, byte value, byte channel)
{
  BLESendChannelMessage2(midi::ControlChange, channel, number, value);
}

void BLESendProgramChange(byte number, byte channel)
{
  BLESendChannelMessage1(midi::ProgramChange, channel, number);
}

void BLESendAfterTouch(byte pressure, byte channel)
{
  BLESendChannelMessage1(midi::AfterTouchChannel, channel, pressure);
}

void BLESendPitchBend(int bend, byte channel)
{
  BLESendChannelMessage1(midi::PitchBend, channel, bend);
}

void BLESendSystemExclusive(const byte* array, unsigned size)
{
  /*
    Multiple Packet Encoding (SysEx Only)
    Only a SysEx (System Exclusive) message may span multiple BLE packets and is encoded as
    follows:
    1.  The SysEx start byte, which is a MIDI Status byte, is preceded by a timestamp byte.
    2.  Following the SysEx start byte, any number of Data bytes (up to the number of the
        remaining bytes in the packet) may be written.
    3.  Any remaining data may be sent in one or more SysEx continuation packets. A SysEx
        continuation packet begins with a header byte but does not contain a timestamp byte. It
        then contains one or more bytes of the SysEx data, up to the maximum packet length. This
        lack of a timestamp byte serves as a signal to the decoder of a SysEx continuation.
    4.  System Real-Time messages may appear at any point inside a SysEx message and must
        be preceded by a timestamp byte.
    5.  SysEx continuations for unterminated SysEx messages must follow either the packet’s
        header byte or a real-time byte.
    6.  Continue sending SysEx continuation packets until the entire message is transmitted.
    7.  In the last packet containing SysEx data, precede the EOX message (SysEx end byte),
        which is a MIDI Status byte, with a timestamp byte.
    Once a SysEx transfer has begun, only System Real-Time messages are allowed to precede its
    completion as follows:
    1.  A System Real-Time message interrupting a yet unterminated SysEx message must be
        preceded by its own timestamp byte.
    2.  SysEx continuations for unterminated SysEx messages must follow either the packet’s
        header byte or a real-time byte.
  */

  //
  //  to be implemented
  //
}

void BLESendTimeCodeQuarterFrame(byte data)
{
  BLESendSystemCommonMessage1(midi::TimeCodeQuarterFrame, data);
}

void BLESendSongPosition(unsigned int beats)
{
  BLESendSystemCommonMessage2(midi::SongPosition, beats >> 4, beats & 0x0f);
}

void BLESendSongSelect(byte songnumber)
{
  BLESendSystemCommonMessage1(midi::SongSelect, songnumber);
}

void BLESendTuneRequest(void)
{
  BLESendRealTimeMessage(midi::TuneRequest);
}

void BLESendClock(void)
{
  BLESendRealTimeMessage(midi::Clock);
}

void BLESendStart(void)
{
  BLESendRealTimeMessage(midi::Start);
}

void BLESendContinue(void)
{
  BLESendRealTimeMessage(midi::Continue);
}

void BLESendStop(void)
{
  BLESendRealTimeMessage(midi::Stop);
}

void BLESendActiveSensing(void)
{
  BLESendRealTimeMessage(midi::ActiveSensing);
}

void BLESendSystemReset(void)
{
  BLESendRealTimeMessage(midi::SystemReset);
}
#endif  // NOBLE


#ifdef NOWIFI
#define AppleMidiSendNoteOn(...)
#define AppleMidiSendNoteOff(...)
#define AppleMidiSendAfterTouchPoly(...)
#define AppleMidiSendControlChange(...)
#define AppleMidiSendProgramChange(...)
#define AppleMidiSendAfterTouch(...)
#define AppleMidiSendPitchBend(...)
#define AppleMidiSendSystemExclusive(...)
#define AppleMidiSendTimeCodeQuarterFrame(...)
#define AppleMidiSendSongPosition(...)
#define AppleMidiSendSongSelect(...)
#define AppleMidiSendTuneRequest(...)
#define AppleMidiSendClock(...)
#define AppleMidiSendStart(...)
#define AppleMidiSendContinue(...)
#define AppleMidiSendStop(...)
#define AppleMidiSendActiveSensing(...)
#define AppleMidiSendSystemReset(...)
#define ipMIDISendChannelMessage1(...)
#define ipMIDISendChannelMessage2(...)
#define ipMIDISendSystemCommonMessage1(...)
#define ipMIDISendSystemCommonMessage2(...)
#define ipMIDISendRealTimeMessage(...)
#define ipMIDISendNoteOn(...)
#define ipMIDISendNoteOff(...)
#define ipMIDISendAfterTouchPoly(...)
#define ipMIDISendControlChange(...)
#define ipMIDISendProgramChange(...)
#define ipMIDISendAfterTouch(...)
#define ipMIDISendPitchBend(...)
#define ipMIDISendSystemExclusive(...)
#define ipMIDISendTimeCodeQuarterFrame(...)
#define ipMIDISendSongPosition(...)
#define ipMIDISendSongSelect(...)
#define ipMIDISendTuneRequest(...)
#define ipMIDISendClock(...)
#define ipMIDISendStart(...)
#define ipMIDISendContinue(...)
#define ipMIDISendStop(...)
#define ipMIDISendActiveSensing(...)
#define ipMIDISendSystemReset(...)
#define OSCSendNoteOn(...)
#define OSCSendNoteOff(...)
#define OSCSendAfterTouchPoly(...)
#define OSCSendControlChange(...)
#define OSCSendProgramChange(...)
#define OSCSendAfterTouch(...)
#define OSCSendPitchBend(...)
#define OSCSendSystemExclusive(...)
#define OSCSendTimeCodeQuarterFrame(...)
#define OSCSendSongPosition(...)
#define OSCSendSongSelect(...)
#define OSCSendTuneRequest(...)
#define OSCSendClock(...)
#define OSCSendStart(...)
#define OSCSendContinue(...)
#define OSCSendStop(...)
#define OSCSendActiveSensing(...)
#define OSCSendSystemReset(...)
#else

void AppleMidiSendNoteOn(byte note, byte velocity, byte channel)
{
  if (interfaces[PED_RTPMIDI].midiOut) AppleMIDI.noteOn(note, velocity, channel);
}

void AppleMidiSendNoteOff(byte note, byte velocity, byte channel)
{
  if (interfaces[PED_RTPMIDI].midiOut) AppleMIDI.noteOff(note, velocity, channel);
}

void AppleMidiSendAfterTouchPoly(byte note, byte pressure, byte channel)
{
  if (interfaces[PED_RTPMIDI].midiOut) AppleMIDI.polyPressure(note, pressure, channel);
}

void AppleMidiSendControlChange(byte number, byte value, byte channel)
{
  if (interfaces[PED_RTPMIDI].midiOut) AppleMIDI.controlChange(number, value, channel);
}

void AppleMidiSendProgramChange(byte number, byte channel)
{
  if (interfaces[PED_RTPMIDI].midiOut) AppleMIDI.programChange(number, channel);
}

void AppleMidiSendAfterTouch(byte pressure, byte channel)
{
  if (interfaces[PED_RTPMIDI].midiOut) AppleMIDI.afterTouch(pressure, channel);
}

void AppleMidiSendPitchBend(int bend, byte channel)
{
  if (interfaces[PED_RTPMIDI].midiOut) AppleMIDI.pitchBend(bend, channel);
}

void AppleMidiSendSystemExclusive(byte* array, unsigned size)
{
  if (interfaces[PED_RTPMIDI].midiOut) AppleMIDI.sysEx(array, size);
}

void AppleMidiSendTimeCodeQuarterFrame(byte data)
{
  if (interfaces[PED_RTPMIDI].midiOut) AppleMIDI.timeCodeQuarterFrame(data);
}

void AppleMidiSendSongPosition(unsigned int beats)
{
  if (interfaces[PED_RTPMIDI].midiOut) AppleMIDI.songPosition(beats);
}

void AppleMidiSendSongSelect(byte songnumber)
{
  if (interfaces[PED_RTPMIDI].midiOut) AppleMIDI.songSelect(songnumber);
}

void AppleMidiSendTuneRequest(void)
{
  if (interfaces[PED_RTPMIDI].midiOut) AppleMIDI.tuneRequest();
}

void AppleMidiSendClock(void)
{
  if (interfaces[PED_RTPMIDI].midiOut) AppleMIDI.clock();
}

void AppleMidiSendStart(void)
{
  if (interfaces[PED_RTPMIDI].midiOut) AppleMIDI.start();
}

void AppleMidiSendContinue(void)
{
  if (interfaces[PED_RTPMIDI].midiOut) AppleMIDI._continue();
}

void AppleMidiSendStop(void)
{
  if (interfaces[PED_RTPMIDI].midiOut) AppleMIDI.stop();
}

void AppleMidiSendActiveSensing(void)
{
  if (interfaces[PED_RTPMIDI].midiOut) AppleMIDI.activeSensing();
}

void AppleMidiSendSystemReset(void)
{
  if (interfaces[PED_RTPMIDI].midiOut) AppleMIDI.reset();
}


// Send messages to WiFi ipMIDI interface

void ipMIDISendChannelMessage1(byte type, byte channel, byte data1)
{
  byte midiPacket[2];

  if (!interfaces[PED_IPMIDI].midiOut) return;

  midiPacket[0] = (type & 0xf0) | ((channel - 1) & 0x0f);
  midiPacket[1] = data1;
#ifdef ARDUINO_ARCH_ESP8266
  ipMIDI.beginPacketMulticast(ipMIDImulticast, ipMIDIdestPort, WiFi.localIP());
#endif
#ifdef ARDUINO_ARCH_ESP32
  ipMIDI.beginMulticastPacket();
#endif
  ipMIDI.write(midiPacket, 2);
  ipMIDI.endPacket();
}

void ipMIDISendChannelMessage2(byte type, byte channel, byte data1, byte data2)
{
  byte midiPacket[3];

  if (!interfaces[PED_IPMIDI].midiOut) return;

  midiPacket[0] = (type & 0xf0) | ((channel - 1) & 0x0f);
  midiPacket[1] = data1;
  midiPacket[2] = data2;
#ifdef ARDUINO_ARCH_ESP8266
  ipMIDI.beginPacketMulticast(ipMIDImulticast, ipMIDIdestPort, WiFi.localIP());
#endif
#ifdef ARDUINO_ARCH_ESP32
  ipMIDI.beginMulticastPacket();
#endif
  ipMIDI.write(midiPacket, 3);
  ipMIDI.endPacket();
}

void ipMIDISendSystemCommonMessage1(byte type, byte data1)
{
  byte midiPacket[2];

  if (!interfaces[PED_IPMIDI].midiOut) return;

  midiPacket[0] = type;
  midiPacket[1] = data1;
#ifdef ARDUINO_ARCH_ESP8266
  ipMIDI.beginPacketMulticast(ipMIDImulticast, ipMIDIdestPort, WiFi.localIP());
#endif
#ifdef ARDUINO_ARCH_ESP32
  ipMIDI.beginMulticastPacket();
#endif
  ipMIDI.write(midiPacket, 2);
  ipMIDI.endPacket();
}

void ipMIDISendSystemCommonMessage2(byte type, byte data1, byte data2)
{
  byte  midiPacket[3];

  if (!interfaces[PED_IPMIDI].midiOut) return;

  midiPacket[0] = type;
  midiPacket[1] = data1;
  midiPacket[2] = data2;
#ifdef ARDUINO_ARCH_ESP8266
  ipMIDI.beginPacketMulticast(ipMIDImulticast, ipMIDIdestPort, WiFi.localIP());
#endif
#ifdef ARDUINO_ARCH_ESP32
  ipMIDI.beginMulticastPacket();
#endif
  ipMIDI.write(midiPacket, 3);
  ipMIDI.endPacket();
}

void ipMIDISendRealTimeMessage(byte type)
{
  byte midiPacket[1];

  if (!interfaces[PED_IPMIDI].midiOut) return;

  midiPacket[0] = type;
#ifdef ARDUINO_ARCH_ESP8266
  ipMIDI.beginPacketMulticast(ipMIDImulticast, ipMIDIdestPort, WiFi.localIP());
#endif
#ifdef ARDUINO_ARCH_ESP32
  ipMIDI.beginMulticastPacket();
#endif
  ipMIDI.write(midiPacket, 1);
  ipMIDI.endPacket();
}

void ipMIDISendNoteOn(byte note, byte velocity, byte channel)
{
  ipMIDISendChannelMessage2(midi::NoteOn, channel, note, velocity);
}

void ipMIDISendNoteOff(byte note, byte velocity, byte channel)
{
  ipMIDISendChannelMessage2(midi::NoteOff, channel, note, velocity);
}

void ipMIDISendAfterTouchPoly(byte note, byte pressure, byte channel)
{
  ipMIDISendChannelMessage2(midi::AfterTouchPoly, channel, note, pressure);
}

void ipMIDISendControlChange(byte number, byte value, byte channel)
{
  ipMIDISendChannelMessage2(midi::ControlChange, channel, number, value);
}

void ipMIDISendProgramChange(byte number, byte channel)
{
  ipMIDISendChannelMessage1(midi::ProgramChange, channel, number);
}

void ipMIDISendAfterTouch(byte pressure, byte channel)
{
  ipMIDISendChannelMessage1(midi::AfterTouchChannel, channel, pressure);
}

void ipMIDISendPitchBend(int bend, byte channel)
{
  ipMIDISendChannelMessage1(midi::PitchBend, channel, bend);
}

void ipMIDISendSystemExclusive(const byte* array, unsigned size)
{
  //
  //  to be implemented
  //
}

void ipMIDISendTimeCodeQuarterFrame(byte data)
{
  ipMIDISendSystemCommonMessage1(midi::TimeCodeQuarterFrame, data);
}

void ipMIDISendSongPosition(unsigned int beats)
{
  ipMIDISendSystemCommonMessage2(midi::SongPosition, beats >> 4, beats & 0x0f);
}

void ipMIDISendSongSelect(byte songnumber)
{
  ipMIDISendSystemCommonMessage1(midi::SongSelect, songnumber);
}

void ipMIDISendTuneRequest(void)
{
  ipMIDISendRealTimeMessage(midi::TuneRequest);
}

void ipMIDISendClock(void)
{
  ipMIDISendRealTimeMessage(midi::Clock);
}

void ipMIDISendStart(void)
{
  ipMIDISendRealTimeMessage(midi::Start);
}

void ipMIDISendContinue(void)
{
  ipMIDISendRealTimeMessage(midi::Continue);
}

void ipMIDISendStop(void)
{
  ipMIDISendRealTimeMessage(midi::Stop);
}

void ipMIDISendActiveSensing(void)
{
  ipMIDISendRealTimeMessage(midi::ActiveSensing);
}

void ipMIDISendSystemReset(void)
{
  ipMIDISendRealTimeMessage(midi::SystemReset);
}


// Send messages to WiFI OSC interface

void OSCSendNoteOn(byte note, byte velocity, byte channel)
{
  if (!interfaces[PED_OSC].midiOut) return;

  String msg = "/pedalino/midi/note/";
  msg += note;
  OSCMessage oscMsg(msg.c_str());
  oscUDP.beginPacket(oscRemoteIp, oscRemotePort);
  oscMsg.add((float)(velocity / 127.0)).add((int32_t)channel).send(oscUDP).empty();
  oscUDP.endPacket();
}

void OSCSendNoteOff(byte note, byte velocity, byte channel)
{
  if (!interfaces[PED_OSC].midiOut) return;

  String msg = "/pedalino/midi/note/";
  msg += note;
  OSCMessage oscMsg(msg.c_str());
  oscUDP.beginPacket(oscRemoteIp, oscRemotePort);
  oscMsg.add((float)0).add((int32_t)channel).send(oscUDP).empty();
  oscUDP.endPacket();
}

void OSCSendAfterTouchPoly(byte note, byte pressure, byte channel)
{
  if (!interfaces[PED_OSC].midiOut) return;

  String msg = "/pedalino/midi/aftertouchpoly/";
  msg += note;
  OSCMessage oscMsg(msg.c_str());
  oscUDP.beginPacket(oscRemoteIp, oscRemotePort);
  oscMsg.add((float)(pressure / 127.0)).add((int32_t)channel).send(oscUDP).empty();
  oscUDP.endPacket();
}

void OSCSendControlChange(byte number, byte value, byte channel)
{
  if (!interfaces[PED_OSC].midiOut) return;

  String msg = "/pedalino/midi/cc/";
  msg += number;
  OSCMessage oscMsg(msg.c_str());
  oscUDP.beginPacket(oscRemoteIp, oscRemotePort);
  oscMsg.add((float)(value / 127.0)).add((int32_t)channel).send(oscUDP).empty();
  oscUDP.endPacket();
}

void OSCSendProgramChange(byte number, byte channel)
{
  if (!interfaces[PED_OSC].midiOut) return;

  String msg = "/pedalino/midi/pc/";
  msg += number;
  OSCMessage oscMsg(msg.c_str());
  oscUDP.beginPacket(oscRemoteIp, oscRemotePort);
  oscMsg.add((int32_t)channel).send(oscUDP).empty();
  oscUDP.endPacket();
}

void OSCSendAfterTouch(byte pressure, byte channel)
{
  if (!interfaces[PED_OSC].midiOut) return;

  String msg = "/pedalino/midi/aftertouchchannel/";
  msg += channel;
  OSCMessage oscMsg(msg.c_str());
  oscUDP.beginPacket(oscRemoteIp, oscRemotePort);
  oscMsg.add((float)(pressure / 127.0)).add((int32_t)channel).send(oscUDP).empty();
  oscUDP.endPacket();
}

void OSCSendPitchBend(int bend, byte channel)
{
  if (!interfaces[PED_OSC].midiOut) return;

  String msg = "/pedalino/midi/pitchbend/";
  msg += channel;
  OSCMessage oscMsg(msg.c_str());
  oscUDP.beginPacket(oscRemoteIp, oscRemotePort);
  oscMsg.add((float)((bend + 8192) / 16383.0)).add((int32_t)channel).send(oscUDP).empty();
  oscUDP.endPacket();
}

void OSCSendSystemExclusive(const byte* array, unsigned size)
{
}

void OSCSendTimeCodeQuarterFrame(byte data)
{
}

void OSCSendSongPosition(unsigned int beats)
{
  if (!interfaces[PED_OSC].midiOut) return;

  String msg = "/pedalino/midi/songpostion/";
  msg += beats;
  OSCMessage oscMsg(msg.c_str());
  oscUDP.beginPacket(oscRemoteIp, oscRemotePort);
  oscMsg.add((int32_t)beats).send(oscUDP).empty();
  oscUDP.endPacket();
}

void OSCSendSongSelect(byte songnumber)
{
  if (!interfaces[PED_OSC].midiOut) return;

  String msg = "/pedalino/midi/songselect/";
  msg += songnumber;
  OSCMessage oscMsg(msg.c_str());
  oscUDP.beginPacket(oscRemoteIp, oscRemotePort);
  oscMsg.add((int32_t)songnumber).send(oscUDP).empty();
  oscUDP.endPacket();
}

void OSCSendTuneRequest(void)
{
  if (!interfaces[PED_OSC].midiOut) return;

  OSCMessage oscMsg("/pedalino/midi/tunerequest/");
  oscUDP.beginPacket(oscRemoteIp, oscRemotePort);
  oscMsg.send(oscUDP).empty();
  oscUDP.endPacket();
}

void OSCSendClock(void)
{
}

void OSCSendStart(void)
{
  if (!interfaces[PED_OSC].midiOut) return;

  OSCMessage oscMsg("/pedalino/midi/start/");
  oscUDP.beginPacket(oscRemoteIp, oscRemotePort);
  oscMsg.send(oscUDP).empty();
  oscUDP.endPacket();
}

void OSCSendContinue(void)
{
  if (!interfaces[PED_OSC].midiOut) return;

  OSCMessage oscMsg("/pedalino/midi/continue/");
  oscUDP.beginPacket(oscRemoteIp, oscRemotePort);
  oscMsg.send(oscUDP).empty();
  oscUDP.endPacket();
}

void OSCSendStop(void)
{
  if (!interfaces[PED_OSC].midiOut) return;

  OSCMessage oscMsg("/pedalino/midi/stop/");
  oscUDP.beginPacket(oscRemoteIp, oscRemotePort);
  oscMsg.send(oscUDP).empty();
  oscUDP.endPacket();
}

void OSCSendActiveSensing(void)
{
  if (!interfaces[PED_OSC].midiOut) return;

  OSCMessage oscMsg("/pedalino/midi/activesensing/");
  oscUDP.beginPacket(oscRemoteIp, oscRemotePort);
  oscMsg.send(oscUDP).empty();
  oscUDP.endPacket();
}

void OSCSendSystemReset(void)
{
  if (!interfaces[PED_OSC].midiOut) return;

  OSCMessage oscMsg("/pedalino/midi/reset/");
  oscUDP.beginPacket(oscRemoteIp, oscRemotePort);
  oscMsg.send(oscUDP).empty();
  oscUDP.endPacket();
}
#endif  //  NOWIFI


// Forward messages received from serial MIDI interface to WiFI interface

void OnSerialMidiNoteOn(byte channel, byte note, byte velocity)
{
  BLESendNoteOn(note, velocity, channel);
  ipMIDISendNoteOn(note, velocity, channel);
  AppleMidiSendNoteOn(note, velocity, channel);
  OSCSendNoteOn(note, velocity, channel);
}

void OnSerialMidiNoteOff(byte channel, byte note, byte velocity)
{
  BLESendNoteOff(note, velocity, channel);
  ipMIDISendNoteOff(note, velocity, channel);
  AppleMidiSendNoteOff(note, velocity, channel);
  OSCSendNoteOff(note, velocity, channel);
}

void OnSerialMidiAfterTouchPoly(byte channel, byte note, byte pressure)
{
  BLESendAfterTouchPoly(note, pressure, channel);
  ipMIDISendAfterTouchPoly(note, pressure, channel);
  AppleMidiSendAfterTouchPoly(note, pressure, channel);
  OSCSendAfterTouchPoly(note, pressure, channel);
}

void OnSerialMidiControlChange(byte channel, byte number, byte value)
{
  BLESendControlChange(number, value, channel);
  ipMIDISendControlChange(number, value, channel);
  AppleMidiSendControlChange(number, value, channel);
  OSCSendControlChange(number, value, channel);
}

void OnSerialMidiProgramChange(byte channel, byte number)
{
  BLESendProgramChange(number, channel);
  ipMIDISendProgramChange(number, channel);
  AppleMidiSendProgramChange(number, channel);
  OSCSendProgramChange(number, channel);
}

void OnSerialMidiAfterTouchChannel(byte channel, byte pressure)
{
  BLESendAfterTouch(pressure, channel);
  ipMIDISendAfterTouch(pressure, channel);
  AppleMidiSendAfterTouch(pressure, channel);
  OSCSendAfterTouch(pressure, channel);
}

void OnSerialMidiPitchBend(byte channel, int bend)
{
  BLESendPitchBend(bend, channel);
  ipMIDISendPitchBend(bend, channel);
  AppleMidiSendPitchBend(bend, channel);
  OSCSendPitchBend(bend, channel);
}

void OnSerialMidiSystemExclusive(byte* array, unsigned size)
{
  char json[size - 1];
  //byte decodedArray[size];
  //unsigned int decodedSize;

  //decodedSize = midi::decodeSysEx(array, decodedArray, size);

  // Extract JSON string
  //
  memset(json, 0, size - 1);
  memcpy(json, &array[1], size - 2);
  //memcpy(json, &decodedArray[1], size - 2);
  //for (int i = 0; i < size; i++)
  //  DPRINTLN("%2d %0X", i, array[i]);
  DPRINTLN("JSON: %s", json);

  // Memory pool for JSON object tree.
  //
  StaticJsonBuffer<200> jsonBuffer;

  // Root of the object tree.
  //
  JsonObject& root = jsonBuffer.parseObject(json);

  // Test if parsing succeeds.
  if (root.success()) {
    // Fetch values.
    //   
#ifdef BLYNK
    const char *lcdclear = root["lcd.clear"];
    const char *lcd1     = root["lcd1"];
    const char *lcd2     = root["lcd2"];
    if (lcdclear) blynkLCD.clear();
    if (lcd1) blynkLCD.print(0, 0, lcd1);
    if (lcd2) blynkLCD.print(0, 1, lcd2);
#endif
    if (root.containsKey("interface")) {
      byte currentInterface = constrain(root["interface"], 0, INTERFACES - 1);
      interfaces[currentInterface].midiIn       = root["in"];
      interfaces[currentInterface].midiOut      = root["out"];
      interfaces[currentInterface].midiThru     = root["thru"];
      interfaces[currentInterface].midiRouting  = root["routing"];
      interfaces[currentInterface].midiClock    = root["clock"];
      DPRINTLN("Updating EEPROM");
      int address = 0;
      for (byte i = 0; i < INTERFACES; i++)
        {
          EEPROM.write(address, interfaces[i].midiIn);
          address += sizeof(byte);
          EEPROM.write(address, interfaces[i].midiOut);
          address += sizeof(byte);
          EEPROM.write(address, interfaces[i].midiThru);
          address += sizeof(byte);
          EEPROM.write(address, interfaces[i].midiRouting);
          address += sizeof(byte);
          EEPROM.write(address, interfaces[i].midiClock);
          address += sizeof(byte);
          DPRINTLN("Interface %s:  %s  %s  %s  %s  %s",
                    interfaces[i].name, 
                    interfaces[i].midiIn      ? "IN"      : "  ",
                    interfaces[i].midiOut     ? "OUT"     : "   ",
                    interfaces[i].midiThru    ? "THRU"    : "    ",
                    interfaces[i].midiRouting ? "ROUTING" : "       ",
                    interfaces[i].midiClock   ? "CLOCK"   : "     ");
        }
      EEPROM.commit();
    }
    else if (root.containsKey("ssid")) {
       save_wifi_credentials(root["ssid"], root["password"]);
#ifndef NOWIFI
       wifi_connect();
#endif
    }
    else if (root.containsKey("factory.default")) {
      save_wifi_credentials("", "");
#ifndef NOWIFI      
      WiFi.disconnect(true);
#endif
      DPRINTLN("EEPROM clear");
      serialize_wifi_status(false);
      serialize_ble_status(false);
      DPRINTLN("ESP restart");
      delay(1000);
      ESP.restart();
    }
    else {
      BLESendSystemExclusive(array, size);
      ipMIDISendSystemExclusive(array, size);
      AppleMidiSendSystemExclusive(array, size);
      OSCSendSystemExclusive(array, size);
    }
  }
}

void OnSerialMidiTimeCodeQuarterFrame(byte data)
{
  BLESendTimeCodeQuarterFrame(data);
  ipMIDISendTimeCodeQuarterFrame(data);
  AppleMidiSendTimeCodeQuarterFrame(data);
  OSCSendTimeCodeQuarterFrame(data);
}

void OnSerialMidiSongPosition(unsigned int beats)
{
  BLESendSongPosition(beats);
  ipMIDISendSongPosition(beats);
  AppleMidiSendSongPosition(beats);
  OSCSendSongPosition(beats);
}

void OnSerialMidiSongSelect(byte songnumber)
{
  BLESendSongSelect(songnumber);
  ipMIDISendSongSelect(songnumber);
  AppleMidiSendSongSelect(songnumber);
  OSCSendSongSelect(songnumber);
}

void OnSerialMidiTuneRequest(void)
{
  BLESendTuneRequest();
  ipMIDISendTuneRequest();
  AppleMidiSendTuneRequest();
  OSCSendTuneRequest();
}

void OnSerialMidiClock(void)
{
  BLESendClock();
  ipMIDISendClock();
  AppleMidiSendClock();
  OSCSendClock();
}

void OnSerialMidiStart(void)
{
  BLESendStart();
  ipMIDISendStart();
  AppleMidiSendStart();
  OSCSendStart();
}

void OnSerialMidiContinue(void)
{
  BLESendContinue();
  ipMIDISendContinue();
  AppleMidiSendContinue();
  OSCSendContinue();
}

void OnSerialMidiStop(void)
{
  BLESendStop();
  ipMIDISendStop();
  AppleMidiSendStop();
  OSCSendStop();
}

void OnSerialMidiActiveSensing(void)
{
  BLESendActiveSensing();
  ipMIDISendActiveSensing();
  AppleMidiSendActiveSensing();
  OSCSendActiveSensing();
}

void OnSerialMidiSystemReset(void)
{
  BLESendSystemReset();
  ipMIDISendSystemReset();
  AppleMidiSendSystemReset();
  OSCSendSystemReset();
}

#ifdef NOWIFI
#define apple_midi_start(...)
#define rtpMIDI_listen(...)
#define ipMIDI_listen(...)
#define oscUDP_listen(...)
#else

// Forward messages received from WiFI MIDI interface to serial MIDI interface

void OnAppleMidiConnected(uint32_t ssrc, char* name)
{
  appleMidiConnected  = true;
  DPRINTLN("AppleMIDI Connected Session ID: %u Name: %s", ssrc, name);
}

void OnAppleMidiDisconnected(uint32_t ssrc)
{
  appleMidiConnected  = false;
  DPRINTLN("AppleMIDI Disconnected Session ID %u", ssrc);
}

void OnAppleMidiNoteOn(byte channel, byte note, byte velocity)
{
  if (!interfaces[PED_RTPMIDI].midiIn) return;

  MIDI.sendNoteOn(note, velocity, channel);
  BLESendNoteOn(note, velocity, channel);
  ipMIDISendNoteOn(note, velocity, channel);
  OSCSendNoteOn(note, velocity, channel);
}

void OnAppleMidiNoteOff(byte channel, byte note, byte velocity)
{
  if (!interfaces[PED_RTPMIDI].midiIn) return;

  MIDI.sendNoteOff(note, velocity, channel);
  BLESendNoteOff(note, velocity, channel);
  ipMIDISendNoteOff(note, velocity, channel);
  OSCSendNoteOff(note, velocity, channel);
}

void OnAppleMidiReceiveAfterTouchPoly(byte channel, byte note, byte pressure)
{
  if (!interfaces[PED_RTPMIDI].midiIn) return;

  MIDI.sendAfterTouch(note, pressure, channel);
  BLESendAfterTouchPoly(note, pressure, channel);
  ipMIDISendAfterTouchPoly(note, pressure, channel);
  OSCSendAfterTouchPoly(note, pressure, channel);
}

void OnAppleMidiReceiveControlChange(byte channel, byte number, byte value)
{
  if (!interfaces[PED_RTPMIDI].midiIn) return;

  MIDI.sendControlChange(number, value, channel);
  BLESendControlChange(number, value, channel);
  ipMIDISendControlChange(number, value, channel);
  OSCSendControlChange(number, value, channel);
}

void OnAppleMidiReceiveProgramChange(byte channel, byte number)
{
  if (!interfaces[PED_RTPMIDI].midiIn) return;

  MIDI.sendProgramChange(number, channel);
  BLESendProgramChange(number, channel);
  OSCSendProgramChange(number, channel);
}

void OnAppleMidiReceiveAfterTouchChannel(byte channel, byte pressure)
{
  if (!interfaces[PED_RTPMIDI].midiIn) return;

  MIDI.sendAfterTouch(pressure, channel);
  BLESendAfterTouch(pressure, channel);
  ipMIDISendAfterTouch(pressure, channel);
  OSCSendAfterTouch(pressure, channel);
}

void OnAppleMidiReceivePitchBend(byte channel, int bend)
{
  if (!interfaces[PED_RTPMIDI].midiIn) return;

  MIDI.sendPitchBend(bend, channel);
  BLESendPitchBend(bend, channel);
  ipMIDISendPitchBend(bend, channel);
  OSCSendPitchBend(bend, channel);
}

void OnAppleMidiReceiveSysEx(const byte * data, uint16_t size)
{
  if (!interfaces[PED_RTPMIDI].midiIn) return;

  MIDI.sendSysEx(size, data);
  BLESendSystemExclusive(data, size);
  ipMIDISendSystemExclusive(data, size);
  OSCSendSystemExclusive(data, size);
}

void OnAppleMidiReceiveTimeCodeQuarterFrame(byte data)
{
  if (!interfaces[PED_RTPMIDI].midiIn) return;

  MIDI.sendTimeCodeQuarterFrame(data);
  BLESendTimeCodeQuarterFrame(data);
  ipMIDISendTimeCodeQuarterFrame(data);
  OSCSendTimeCodeQuarterFrame(data);
}

void OnAppleMidiReceiveSongPosition(unsigned short beats)
{
  if (!interfaces[PED_RTPMIDI].midiIn) return;

  MIDI.sendSongPosition(beats);
  BLESendSongPosition(beats);
  ipMIDISendSongPosition(beats);
  OSCSendSongPosition(beats);
}

void OnAppleMidiReceiveSongSelect(byte songnumber)
{
  if (!interfaces[PED_RTPMIDI].midiIn) return;

  MIDI.sendSongSelect(songnumber);
  BLESendSongSelect(songnumber);
  ipMIDISendSongSelect(songnumber);
  OSCSendSongSelect(songnumber);
}

void OnAppleMidiReceiveTuneRequest(void)
{
  if (!interfaces[PED_RTPMIDI].midiIn) return;

  MIDI.sendTuneRequest();
  BLESendTuneRequest();
  ipMIDISendTuneRequest();
  OSCSendTuneRequest();
}

void OnAppleMidiReceiveClock(void)
{
  if (!interfaces[PED_RTPMIDI].midiIn) return;

  MIDI.sendRealTime(midi::Clock);
  BLESendClock();
  ipMIDISendClock();
  OSCSendClock();
}

void OnAppleMidiReceiveStart(void)
{
  if (!interfaces[PED_RTPMIDI].midiIn) return;

  MIDI.sendRealTime(midi::Start);
  BLESendStart();
  ipMIDISendStart();
  OSCSendStart();
}

void OnAppleMidiReceiveContinue(void)
{
  if (!interfaces[PED_RTPMIDI].midiIn) return;

  MIDI.sendRealTime(midi::Continue);
  BLESendContinue();
  ipMIDISendContinue();
  OSCSendContinue();
}

void OnAppleMidiReceiveStop(void)
{
  if (!interfaces[PED_RTPMIDI].midiIn) return;

  MIDI.sendRealTime(midi::Stop);
  BLESendStop();
  ipMIDISendStop();
  OSCSendStop();
}

void OnAppleMidiReceiveActiveSensing(void)
{
  if (!interfaces[PED_RTPMIDI].midiIn) return;

  MIDI.sendRealTime(midi::ActiveSensing);
  BLESendActiveSensing();
  ipMIDISendActiveSensing();
  OSCSendActiveSensing();
}

void OnAppleMidiReceiveReset(void)
{
  if (!interfaces[PED_RTPMIDI].midiIn) return;

  MIDI.sendRealTime(midi::SystemReset);
  BLESendSystemReset();
  ipMIDISendSystemReset();
  OSCSendSystemReset();
}

void apple_midi_start()
{
  // Create a session and wait for a remote host to connect to us
  AppleMIDI.begin("Pedalino(TM)");

  AppleMIDI.OnConnected(OnAppleMidiConnected);
  AppleMIDI.OnDisconnected(OnAppleMidiDisconnected);

  // Connect the handle function called upon reception of a MIDI message from WiFi MIDI interface
  AppleMIDI.OnReceiveNoteOn(OnAppleMidiNoteOn);
  AppleMIDI.OnReceiveNoteOff(OnAppleMidiNoteOff);
  AppleMIDI.OnReceiveAfterTouchPoly(OnAppleMidiReceiveAfterTouchPoly);
  AppleMIDI.OnReceiveControlChange(OnAppleMidiReceiveControlChange);
  AppleMIDI.OnReceiveProgramChange(OnAppleMidiReceiveProgramChange);
  AppleMIDI.OnReceiveAfterTouchChannel(OnAppleMidiReceiveAfterTouchChannel);
  AppleMIDI.OnReceivePitchBend(OnAppleMidiReceivePitchBend);
  AppleMIDI.OnReceiveSysEx(OnAppleMidiReceiveSysEx);
  AppleMIDI.OnReceiveTimeCodeQuarterFrame(OnAppleMidiReceiveTimeCodeQuarterFrame);
  AppleMIDI.OnReceiveSongPosition(OnAppleMidiReceiveSongPosition);
  AppleMIDI.OnReceiveSongSelect(OnAppleMidiReceiveSongSelect);
  AppleMIDI.OnReceiveTuneRequest(OnAppleMidiReceiveTuneRequest);
  AppleMIDI.OnReceiveClock(OnAppleMidiReceiveClock);
  AppleMIDI.OnReceiveStart(OnAppleMidiReceiveStart);
  AppleMIDI.OnReceiveContinue(OnAppleMidiReceiveContinue);
  AppleMIDI.OnReceiveStop(OnAppleMidiReceiveStop);
  AppleMIDI.OnReceiveActiveSensing(OnAppleMidiReceiveActiveSensing);
  AppleMIDI.OnReceiveReset(OnAppleMidiReceiveReset);
}

// Forward messages received from WiFI OSC interface to serial MIDI interface

void OnOscNoteOn(OSCMessage &msg)
{
  MIDI.sendNoteOn(msg.getInt(1), msg.getInt(2), msg.getInt(0));
}

void OnOscNoteOff(OSCMessage &msg)
{
  MIDI.sendNoteOff(msg.getInt(1), msg.getInt(2), msg.getInt(0));
}

void OnOscControlChange(OSCMessage &msg)
{
  MIDI.sendControlChange(msg.getInt(1), msg.getInt(2), msg.getInt(0));
}

// Listen to incoming OSC messages from WiFi

void oscUDP_listen() {
  
  if (!interfaces[PED_OSC].midiIn) return;

  if (!WiFi.isConnected()) return;

  int size = oscUDP.parsePacket();

  if (size > 0) {
    while (size--) oscMsg.fill(oscUDP.read());
    if (!oscMsg.hasError()) {
      oscMsg.dispatch(" / pedalino / midi / noteOn",        OnOscNoteOn);
      oscMsg.dispatch(" / pedalino / midi / noteOff",       OnOscNoteOff);
      oscMsg.dispatch(" / pedalino / midi / controlChange", OnOscControlChange);
    } else {
      DPRINTLN("OSC error: %d", oscMsg.getError());
    }
  }
}

// Listen to incoming AppleMIDI messages from WiFi

inline void rtpMIDI_listen() {
  AppleMIDI.run();
}

// Listen to incoming ipMIDI messages from WiFi

void ipMIDI_listen() {
  
  byte status, type, channel;
  byte data[2];
  byte note, velocity, pressure, number, value;
  int  bend;
  unsigned int beats;
  
  if (!interfaces[PED_IPMIDI].midiIn) return;

  if (!WiFi.isConnected()) return;

  ipMIDI.parsePacket();

  while (ipMIDI.available() > 0) {
    
    ipMIDI.read(&status, 1);
    type    = MIDI.getTypeFromStatusByte(status);
    channel = MIDI.getChannelFromStatusByte(status);

    switch(type) {
     
      case midi::NoteOff:
        ipMIDI.read(data, 2);
        note     = data[0];
        velocity = data[1];
        MIDI.sendNoteOff(note, velocity, channel);
        BLESendNoteOff(note, velocity, channel);
        AppleMidiSendNoteOff(note, velocity, channel);
        OSCSendNoteOff(note, velocity, channel);
        break;

      case midi::NoteOn:
        ipMIDI.read(data, 2);
        note     = data[0];
        velocity = data[1];
        MIDI.sendNoteOn(note, velocity, channel);
        BLESendNoteOn(note, velocity, channel);
        AppleMidiSendNoteOn(note, velocity, channel);
        OSCSendNoteOn(note, velocity, channel);
        break;

      case midi::AfterTouchPoly:
        ipMIDI.read(data, 2);
        note     = data[0];
        pressure = data[1];
        MIDI.sendAfterTouch(note, pressure, channel);
        BLESendAfterTouchPoly(note, pressure, channel);
        AppleMidiSendAfterTouchPoly(note, pressure, channel);
        OSCSendAfterTouchPoly(note, pressure, channel);
        break;

      case midi::ControlChange:
        ipMIDI.read(data, 2);
        number  = data[0];
        value   = data[1];
        MIDI.sendControlChange(number, value, channel);
        BLESendControlChange(number, value, channel);
        AppleMidiSendControlChange(number, value, channel);
        OSCSendControlChange(number, value, channel);
        break;

      case midi::ProgramChange:
        ipMIDI.read(data, 1);
        number  = data[0];
        MIDI.sendProgramChange(number, channel);
        BLESendProgramChange(number, channel);
        AppleMidiSendProgramChange(number, channel);
        OSCSendProgramChange(number, channel);
        break;

      case midi::AfterTouchChannel: 
        ipMIDI.read(data, 1);
        pressure = data[0];
        MIDI.sendAfterTouch(pressure, channel);
        BLESendAfterTouch(pressure, channel);
        AppleMidiSendAfterTouch(pressure, channel);
        OSCSendAfterTouch(pressure, channel);
        break;

      case midi::PitchBend:
        ipMIDI.read(data, 2);
        bend = data[1] << 7 | data[0];
        MIDI.sendPitchBend(bend, channel);
        BLESendPitchBend(bend, channel);
        AppleMidiSendPitchBend(bend, channel);
        OSCSendPitchBend(bend, channel);
        break;

      case 0xf0:
        switch(status) {

          case midi::SystemExclusive:
            while (ipMIDI.read(data, 1) && data[0] != 0xf7);
            break;

          case midi::TimeCodeQuarterFrame:
            ipMIDI.read(data, 1);
            value = data[0];
            MIDI.sendTimeCodeQuarterFrame(value);
            BLESendTimeCodeQuarterFrame(value);
            AppleMidiSendTimeCodeQuarterFrame(value);
            OSCSendTimeCodeQuarterFrame(value);
            break;

          case midi::SongPosition:
            ipMIDI.read(data, 2);
            beats = data[1] << 7 | data[0];
            MIDI.sendSongPosition(beats);
            BLESendSongPosition(beats);
            AppleMidiSendSongPosition(beats);
            OSCSendSongPosition(beats);
            break;

          case midi::SongSelect:
            ipMIDI.read(data, 1);
            number = data[0];
            MIDI.sendSongSelect(number);
            BLESendSongSelect(number);
            AppleMidiSendSongSelect(number);
            OSCSendSongSelect(number);
            break;

          case midi::TuneRequest:
            MIDI.sendRealTime(midi::TuneRequest);
            BLESendTuneRequest();
            AppleMidiSendTuneRequest();
            OSCSendTuneRequest();
            break;

          case midi::Clock:
            MIDI.sendRealTime(midi::Clock);
            BLESendClock();
            AppleMidiSendClock();
            OSCSendClock();
            break;

          case midi::Start:
            MIDI.sendRealTime(midi::Start);
            BLESendStart();
            AppleMidiSendStart();
            OSCSendStart();
            break;
          
          case midi::Continue:
            MIDI.sendRealTime(midi::Continue);
            BLESendContinue();
            AppleMidiSendContinue();
            OSCSendContinue();
            break;

          case midi::Stop:
            MIDI.sendRealTime(midi::Stop);
            BLESendStop();
            AppleMidiSendStop();
            OSCSendStop();
            break;

          case midi::ActiveSensing:
            MIDI.sendRealTime(midi::ActiveSensing);
            BLESendActiveSensing();
            AppleMidiSendActiveSensing();
            OSCSendActiveSensing();
            break;

          case midi::SystemReset:
            MIDI.sendRealTime(midi::SystemReset);
            BLESendSystemReset();
            AppleMidiSendSystemReset();
            OSCSendSystemReset();
            break;
        }
        break;

      default:
        ipMIDI.read(data,2);
    }
    DPRINTMIDI(ipMIDI.remoteIP().toString().c_str(), status, data);
  }
}


#ifdef ARDUINO_ARCH_ESP32
String translateEncryptionType(wifi_auth_mode_t encryptionType) {

  switch (encryptionType) {
    case (WIFI_AUTH_OPEN):
      return "Open";
    case (WIFI_AUTH_WEP):
      return "WEP";
    case (WIFI_AUTH_WPA_PSK):
      return "WPA_PSK";
    case (WIFI_AUTH_WPA2_PSK):
      return "WPA2_PSK";
    case (WIFI_AUTH_WPA_WPA2_PSK):
      return "WPA_WPA2_PSK";
    case (WIFI_AUTH_WPA2_ENTERPRISE):
      return "WPA2_ENTERPRISE";
    case (WIFI_AUTH_MAX):
      return "";
  }

  return "";
}
#endif

#ifndef NOWEBCONFIG
String  theme = "bootstrap";
String  bank  = "1";

String get_top_page(byte p = 0) {
  
  String page = "";
  
  page += F("<!doctype html>");
  page += F("<html lang='en'>");
  page += F("<head>");
  page += F("<title>Pedalino&trade;</title>");
  page += F("<meta charset='utf-8'>");
  page += F(" <meta name='viewport' content='width=device-width, initial-scale=1, shrink-to-fit=no'>");
  if ( theme == "bootstrap" ) {
    page += F("<link rel='stylesheet' href='https://stackpath.bootstrapcdn.com/bootstrap/4.1.3/css/bootstrap.min.css' integrity='sha384-MCw98/SFnGE8fJT3GXwEOngsV7Zt27NXFoaoApmYm81iuXoPkFOJwJ8ERdknLPMO' crossorigin='anonymous'>");
  } else {
    page += F("<link rel='stylesheet' href='https://maxcdn.bootstrapcdn.com/bootswatch/4.1.3/");
    page += theme;
    page += F("/bootstrap.min.css' crossorigin='anonymous'>");
  }
  page += F("</head>");

  page += F("<body>");
  page += F("<p></p>");
  page += F("<div class='container-fluid'>");

  page += F("<nav class='navbar navbar-expand navbar-light bg-light'>");
  page += F("<a class='navbar-brand' href='/'>Pedalino&trade;</a>");
  page += F("<button class='navbar-toggler' type='button' data-toggle='collapse' data-target='#navbarNavDropdown' aria-controls='navbarNavDropdown' aria-expanded='false' aria-label='Toggle navigation'>");
  page += F("<span class='navbar-toggler-icon'></span>");
  page += F("</button>");
  page += F("<div class='collapse navbar-collapse' id='navbarNavDropdown'>");
  page += F("<ul class='navbar-nav mr-auto'>");
  page += F("<li class='nav-item");
  page += (p == 1 ? F(" active'>") : F("'>"));
  page += F("<a class='nav-link' href='/live'>Live</a>");
  page += F("</li>");
  page += F("<li class='nav-item");
  page += (p == 2 ? F(" active'>") : F("'>"));
  page += F("<a class='nav-link' href='/banks'>Banks</a>");
  page += F("</li>");
  page += F("<li class='nav-item");
  page += (p == 3 ? F(" active'>") : F("'>"));
  page += F("<a class='nav-link' href='/pedals'>Pedals</a>");
  page += F("</li>");
  page += F("<li class='nav-item");
  page += (p == 4 ? F(" active'>") : F("'>"));
  page += F("<a class='nav-link' href='/interfaces'>Interfaces</a>");
  page += F("</li>");
  page += F("<li class='nav-item");
  page += (p == 5 ? F(" active'>") : F("'>"));
  page += F("<a class='nav-link' href='/options'>Options</a>");
  page += F("</li>");
  page += F("</ul>");
  page += F("<form class='form-inline my-2 my-lg-0'>");
  page += F("<button class='btn btn-primary my-2 my-sm-0' type='button'>Save</button>");
  page += F("</form>");
  page += F("</div>");
  page += F("</nav>");

  return page;
}

String get_footer_page() {
  
  String page = "";
  page += F("<nav class='navbar fixed-bottom navbar-light bg-light'>");
  page += F("<a class='navbar-text' href='https://github.com/alf45tar/Pedalino'>https://github.com/alf45tar/Pedalino</a>");
  page += F("</nav>");

  page += F("</div>");
  page += F("<script src='https://code.jquery.com/jquery-3.3.1.slim.min.js' integrity='sha384-q8i/X+965DzO0rT7abK41JStQIAqVgRVzpbzo5smXKp4YfRvH+8abtTE1Pi6jizo' crossorigin='anonymous'></script>");
  page += F("<script src='https://cdnjs.cloudflare.com/ajax/libs/popper.js/1.14.3/umd/popper.min.js' integrity='sha384-ZMP7rVo3mIykV+2+9J3UJ46jBk0WLaUAdn689aCwoqbBJiSnjAK/l8WvCWPIPm49' crossorigin='anonymous'></script>");
  page += F("<script src='https://stackpath.bootstrapcdn.com/bootstrap/4.1.3/js/bootstrap.min.js' integrity='sha384-ChfqqxuZUCnJSK3+MXmPNIyE6ZbWh2IMqE241rYiqJxyMiZ6OW/JmZQ5stwEULTy' crossorigin='anonymous'></script>");
  page += F("</body>");
  page += F("</html>");

  return page;
}

String get_root_page() {
  
  String page = "";
  
  page += get_top_page();
  
  page += F("<p></p>");
  page += F("<h6>Smart wireless MIDI foot controller for guitarists and more</h6>");
  page += F("<p></p>");

  page += get_footer_page();

  return page;
}

String get_live_page() {

  String page = "";

  page += get_top_page(1);

  page += F("<p></p>");
  
  page += get_footer_page();

  return page;
}

String get_banks_page() {
  
  String page = "";
  
  page += get_top_page(2);
/*
  page += "<div class='container-fluid'>";
  
  page += "<h1>Pedalino&trade;</h1>";
  page += "<div class='row'>";
  page += "<div class='col'>";
  page += "<ul class='nav nav-pills'>";
  page += "<li class='nav-item'><a class='nav-link' href='/live'>Live</a></li>";
  page += "<li class='nav-item active'><a class='nav-link' href='/banks'>Banks</a></li>";
  page += "<li class='nav-item'><a class='nav-link' href='/pedals'>Pedals</a></li>";
  page += "<li class='nav-item'><a class='nav-link' href='/interfaces'>Interfaces</a></li>";
  page += "<li class='nav-item dropdown'>";
  page +=       "<form method='POST' name='selecttheme' id='selecttheme'/>"; 
  page +=       "<input class='span' id='choixtheme' name='theme' type='hidden'>";
  page += "<a class='nav-link dropdown-toggle' data-toggle='dropdown' href='#'>Theme</a>";
  //page += "<div class='dropdown-menu'>";
  //page += "<a class='dropdown-item' href='#''>Link 1</a>";
  page += "<ul class='dropdown-menu'>";
  page += "<li onclick='$(\"#choixtheme\").val(\"bootstrap\"); $(\"#selecttheme\").submit()'><a href='#'>Boostrap</a></li>";
  page += "<li onclick='$(\"#choixtheme\").val(\"cerulean\"); $(\"#selecttheme\").submit()'><a href='#'>Cerulean</a></li>";
  page += "<li onclick='$(\"#choixtheme\").val(\"cosmo\"); $(\"#selecttheme\").submit()'><a href='#'>Cosmo</a></li>";
  page += "<li onclick='$(\"#choixtheme\").val(\"cyborg\"); $(\"#selecttheme\").submit()'><a href='#'>Cyborg</a></li>";
  page += "<li onclick='$(\"#choixtheme\").val(\"darkly\"); $(\"#selecttheme\").submit()'><a href='#'>Darkly</a></li>";
  page += "<li onclick='$(\"#choixtheme\").val(\"flatly\"); $(\"#selecttheme\").submit()'><a href='#'>Flatly</a></li>";
  page += "<li onclick='$(\"#choixtheme\").val(\"journal\"); $(\"#selecttheme\").submit()'><a href='#'>Journal</a></li>";
  page += "<li onclick='$(\"#choixtheme\").val(\"lumen\"); $(\"#selecttheme\").submit()'><a href='#'>Lumen</a></li>";
  page += "<li onclick='$(\"#choixtheme\").val(\"paper\"); $(\"#selecttheme\").submit()'><a href='#'>Paper</a></li>";
  page += "<li onclick='$(\"#choixtheme\").val(\"readable\"); $(\"#selecttheme\").submit()'><a href='#'>Readable</a></li>";
  page += "<li onclick='$(\"#choixtheme\").val(\"sandstone\"); $(\"#selecttheme\").submit()'><a href='#'>Sandstone</a></li>";
  page += "<li onclick='$(\"#choixtheme\").val(\"simplex\"); $(\"#selecttheme\").submit()'><a href='#'>Simplex</a></li>";
  page += "<li onclick='$(\"#choixtheme\").val(\"slate\"); $(\"#selecttheme\").submit()'><a href='#'>Slate</a></li>";
  page += "<li onclick='$(\"#choixtheme\").val(\"spacelab\"); $(\"#selecttheme\").submit()'><a href='#'>Spacelab</a></li>";
  page += "<li onclick='$(\"#choixtheme\").val(\"superhero\"); $(\"#selecttheme\").submit()'><a href='#'>Superhero</a></li>";
  page += "<li onclick='$(\"#choixtheme\").val(\"united\"); $(\"#selecttheme\").submit()'><a href='#'>United</a></li>";
  page += "<li onclick='$(\"#choixtheme\").val(\"yeti\"); $(\"#selecttheme\").submit()'><a href='#'>Yeti</a></li>";
  page += "</ul>";
  //page += "</div>";
  page +=       "</form>";
  page += "</li>";
  page += "</ul>";
  page += "</div>";
  page += "<div class='col' align='right'><button type='button' class='btn btn-primary'>Apply</button> <button type='button' class='btn btn-primary'>Save</button></div>";
  page += "</div>";
*/
/*
  page += "<nav class='navbar navbar-expand-lg navbar-light bg-light'>";
  page += "<a class='navbar-brand' href='#'>Pedalino&trade;</a>";
  page += "<button class='navbar-toggler' type='button' data-toggle='collapse' data-target='#navbarNavDropdown' aria-controls='navbarNavDropdown' aria-expanded='false' aria-label='Toggle navigation'>";
  page += "<span class='navbar-toggler-icon'></span>";
  page += "</button>";
  page += "<div class='collapse navbar-collapse' id='navbarNavDropdown'>";
  page += "<ul class='navbar-nav'>";
  page += "<li class='nav-item'>";
  page += "<a class='nav-link' href='/'>Home <span class='sr-only'>(current)</span></a>";
  page += "</li>";
  page += "<li class='nav-item'>";
  page += "<a class='nav-link' href='/live'>Live</a>";
  page += "</li>";
  page += "<li class='nav-item active'>";
  page += "<a class='nav-link' href='/banks'>Banks</a>";
  page += "</li>";
  page += "<li class='nav-item'>";
  page += "<a class='nav-link' href='/pedals'>Pedals</a>";
  page += "</li>";
  page += "<li class='nav-item dropdown'>";
  page += "<form method='POST' name='selecttheme' id='selecttheme'/>"; 
  page += "<input class='span' id='choixtheme' name='theme' type='hidden'>";
  page += "<a class='nav-link dropdown-toggle' href='#' id='navbarDropdownMenuLink' data-toggle='dropdown' aria-haspopup='true' aria-expanded='false'>Theme</a>";
  page += "<div class='dropdown-menu' aria-labelledby='navbarDropdownMenuLink'>";
  page += "<a class='dropdown-item' onclick='$(\"#choixtheme\").val(\"bootstrap\"); $(\"#selecttheme\").submit()' href='#'>Boostrap</a>";
  page += "<a class='dropdown-item' onclick='$(\"#choixtheme\").val(\"cerulean\"); $(\"#selecttheme\").submit()' href='#'>Cerulean</a>";
  page += "<a class='dropdown-item' onclick='$(\"#choixtheme\").val(\"cosmo\"); $(\"#selecttheme\").submit()' href='#'>Cosmo</a>";
  page += "<a class='dropdown-item' onclick='$(\"#choixtheme\").val(\"cyborg\"); $(\"#selecttheme\").submit()' href='#'>Cyborg</a>";
  page += "<a class='dropdown-item' onclick='$(\"#choixtheme\").val(\"darkly\"); $(\"#selecttheme\").submit()' href='#'>Darkly</a>";
  page += "<a class='dropdown-item' onclick='$(\"#choixtheme\").val(\"flatly\"); $(\"#selecttheme\").submit()' href='#'>Flatly</a>";
  page += "<a class='dropdown-item' onclick='$(\"#choixtheme\").val(\"journal\"); $(\"#selecttheme\").submit()' href='#'>Journal</a>";
  page += "<a class='dropdown-item' onclick='$(\"#choixtheme\").val(\"lumen\"); $(\"#selecttheme\").submit()' href='#'>Lumen</a>";
  page += "<a class='dropdown-item' onclick='$(\"#choixtheme\").val(\"paper\"); $(\"#selecttheme\").submit()' href='#'>Paper</a>";
  page += "<a class='dropdown-item' onclick='$(\"#choixtheme\").val(\"readable\"); $(\"#selecttheme\").submit()' href='#'>Readable</a>";
  page += "<a class='dropdown-item' onclick='$(\"#choixtheme\").val(\"sandstone\"); $(\"#selecttheme\").submit()' href='#'>Sandstone</a>";
  page += "<a class='dropdown-item' onclick='$(\"#choixtheme\").val(\"simplex\"); $(\"#selecttheme\").submit()' href='#'>Simplex</a>";
  page += "<a class='dropdown-item' onclick='$(\"#choixtheme\").val(\"slate\"); $(\"#selecttheme\").submit()' href='#'>Slate</a>";
  page += "<a class='dropdown-item' onclick='$(\"#choixtheme\").val(\"spacelab\"); $(\"#selecttheme\").submit()' href='#'>Spacelab</a>";
  page += "<a class='dropdown-item' onclick='$(\"#choixtheme\").val(\"superhero\"); $(\"#selecttheme\").submit()' href='#'>Superhero</a>";
  page += "<a class='dropdown-item' onclick='$(\"#choixtheme\").val(\"united\"); $(\"#selecttheme\").submit()' href='#'>United</a>";
  page += "<a class='dropdown-item' onclick='$(\"#choixtheme\").val(\"yeti\"); $(\"#selecttheme\").submit()' href='#'>Yeti</a>";
  page += "</div>";
  page += "</form>";
  page += "</li>";
  page += "</ul>";
  page += "<form class='form-inline'>";
  page += "<button class='btn btn-primary' type='button'>Apply</button>";
  page += "<button class='btn btn-primary' type='button'>Save</button>";
  page += "</form>";
  page += "</div>";
  page += "</nav>";
 */ 

  page += F( "<div class='btn-group'>");
  page += F( "<form><button type='button submit' class='btn ");
  page += (bank == "1" ? String("btn-primary") : String("")) + F("' name='bank' value='1'>1</button></form>");
  page += F( "<form><button type='button submit' class='btn ");
  page += (bank == "2" ? String("btn-primary") : String("")) + F("' name='bank' value='2'>2</button></form>");
  page += F( "<form><button type='button submit' class='btn ");
  page += (bank == "3" ? String("btn-primary") : String("")) + F("' name='bank' value='3'>3</button></form>");
  page += F( "<form><button type='button submit' class='btn ");
  page += (bank == "4" ? String("btn-primary") : String("")) + F("' name='bank' value='4'>4</button></form>");
  page += F( "<form><button type='button submit' class='btn ");
  page += (bank == "5" ? String("btn-primary") : String("")) + F("' name='bank' value='5'>5</button></form>");
  page += F( "<form><button type='button submit' class='btn ");
  page += (bank == "6" ? String("btn-primary") : String("")) + F("' name='bank' value='6'>6</button></form>");
  page += F( "<form><button type='button submit' class='btn ");
  page += (bank == "7" ? String("btn-primary") : String("")) + F("' name='bank' value='7'>7</button></form>");
  page += F("<form><button type='button submit' class='btn ");
  page += (bank == "8" ? String("btn-primary") : String("")) + F("' name='bank' value='8'>8</button></form>");
  page += F("<form><button type='button submit' class='btn ");
  page += (bank == "9" ? String("btn-primary") : String("")) + F("' name='bank' value='9'>9</button></form>");
  page += F("<form><button type='button submit' class='btn ");
  page += (bank =="10" ? String("btn-primary") : String("")) + F("' name='bank' value='10'>10</button></form>");
  page += F("</div>");

  page += F("<table class='table-responsive-sm table-borderless'>");
  page += F("<tbody><tr><td>Pedal</td><td>Message</td><td>Channel</td><td>Code</td><td>Value 1</td><td>Value 2</td><td>Value 3</td></tr>");
  for (unsigned int i = 1; i <= 16; i++) {
    page += F("<tr align='center' valign='center'>");

    page += F("<td>");
    page += String(i);
    page += F("</td>");

    page += F("<td><div class='form-group'>");
	  page += F("<select class='custom-select-sm' id='message");
    page += String(i);
    page += F("'>");
	  page += F("<option>Program Change</option>");
	  page += F("<option>Control Change</option>");
	  page += F("<option>Note On/Off</option>");
	  page += F("<option>Pitch Bend</option>");
	  page += F("</select>");
	  page += F("</div></td>");
		
    page += F("<td><div class='form-group'>");
		page += F("<select class='custom-select-sm' id='channel");
    page += String(i);
    page += F("'>");
		page += F("<option>1</option>");
		page += F("<option>2</option>");
		page += F("<option>3</option>");
		page += F("<option>4</option>");
		page += F("<option>5</option>");
		page += F("<option>6</option>");
		page += F("<option>7</option>");
		page += F("<option>8</option>");
		page += F("<option>9</option>");
		page += F("<option>10</option>");
		page += F("<option>11</option>");
		page += F("<option>12</option>");
		page += F("<option>13</option>");
		page += F("<option>14</option>");
		page += F("<option>15</option>");
		page += F("<option>16</option>");
		page += F("</select>");
		page += F("</div></td>");
		
    page += F("<td><div class='form-group'>");
		page += F("<input type='number' class='custom-select-sm' name='code' min='0' max='127'>");
		page += F("</div></td>");

    page += F("<td><div class='form-group'>");
		page += F("<input type='number' class='custom-select-sm' name='code1' min='0' max='127'>");
		page += F("</div></td>");

    page += F("<td><div class='form-group'>");
		page += F("<input type='number' class='custom-select-sm' name='code2' min='0' max='127'>");
		page += F("</div></td>");

    page += F("<td><div class='form-group'>");
		page += F("<input type='number' class='custom-select-sm' name='code3' min='0' max='127'>");
		page += F("</div></td>");
		        
    page += F("</tr>");
  }  
  page += F("</tbody>");
  page += F("</table>");

  page += get_footer_page();

  return page;
}

String get_pedals_page() {

  String page = "";

  page += get_top_page(3);

  page += F("<p></p>");
  page += F("<div class='row'>");
  page += F("<div class='col-1'>");
  page += F("<span class='badge badge-primary'>Pedal</span>");
  page += F("</div>");
  page += F("<div class='col-1'>");
  page += F("<span class='badge badge-primary'>Autosensing</span>");
  page += F("</div>");
  page += F("<div class='col-2'>");
  page += F("<span class='badge badge-primary'>Mode</span>");
  page += F("</div>");
  page += F("<div class='col-2'>");
  page += F("<span class='badge badge-primary'>Function</span>");
  page += F("</div>");
  page += F("<div class='col-2'>");
  page += F("<span class='badge badge-primary'>Single Press</span>");
  page += F("</div>");
  page += F("<div class='col-2'>");
  page += F("<span class='badge badge-primary'>Double Press</span>");
  page += F("</div>");
  page += F("<div class='col-2'>");
  page += F("<span class='badge badge-primary'>Long Press</span>");
  page += F("</div>");
  page += F("</div>");
  page += F("<p></p>");

  for (unsigned int i = 1; i <= 16; i++) {
    page += F("<div class='row'>");

    page += F("<div class='col-1'>");
    page += String(i);
    page += F("</div>");

    page += F("<div class='col-1'>");
		page += F("<div class='custom-control custom-checkbox'>");
		page += F("<input type='checkbox' class='custom-control-input' id='autoCheck");
    page += String(i) + F("' name='autosensing") + String(i) + F("'>");
		page += F("<label class='custom-control-label' for='autoCheck");
    page += String(i) + F("'></label>");
		page += F("</div>");
    page += F("</div>");

    page += F("<div class='col-2'>");
    page += F("<div class='form-group'>");
	  page += F("<select class='custom-select-sm' id='mode");
    page += String(i);
    page += F("'>");
	  page += F("<option>None</option>");
	  page += F("<option>Momentary</option>");
	  page += F("<option>Latch</option>");
	  page += F("<option>Analog</option>");
    page += F("<option>Jog Wheel</option>");
    page += F("<option>Momentary 3</option>");
    page += F("<option>Momentary 2</option>");
    page += F("<option>Latch 2</option>");
    page += F("<option>Ladder</option>");
	  page += F("</select>");
	  page += F("</div>");
    page += F("</div>");

    page += F("<div class='col-2'>");
    page += F("<div class='form-group'>");
	  page += F("<select class='custom-select-sm' id='function");
    page += String(i);
    page += F("'>");
	  page += F("<option>MIDI</option>");
	  page += F("<option>Bank+</option>");
	  page += F("<option>Bank-</option>");
	  page += F("<option>Start</option>");
    page += F("<option>Stop</option>");
    page += F("<option>Continue</option>");
    page += F("<option>Tap</option>");
    page += F("<option>Menu</option>");
    page += F("<option>Confirm</option>");
    page += F("<option>Excape</option>");
    page += F("<option>Next</option>");
    page += F("<option>Previous</option>");
	  page += F("</select>");
	  page += F("</div>");
    page += F("</div>");

    page += F("<div class='col-2'>");
    page += F("<div class='custom-control custom-checkbox'>");
		page += F("<input type='checkbox' class='custom-control-input' id='singleCheck");
    page += String(i) + F("' name='singlepress") + String(i) + F("'>");
		page += F("<label class='custom-control-label' for='singleCheck");
    page += String(i) + F("'></label>");
		page += F("</div>");
    page += F("</div>");

    page += F("<div class='col-2'>");
    page += F("<div class='custom-control custom-checkbox'>");
		page += F("<input type='checkbox' class='custom-control-input' id='doubleCheck");
    page += String(i) + F("' name='doublepress") + String(i) + F("'>");
		page += F("<label class='custom-control-label' for='doubleCheck");
    page += String(i) + F("'></label>");
		page += F("</div>");
    page += F("</div>");

    page += F("<div class='col-2'>");
    page += F("<div class='custom-control custom-checkbox'>");
		page += F("<input type='checkbox' class='custom-control-input' id='longCheck");
    page += String(i) + F("' name='longpress") + String(i) + F("'>");
		page += F("<label class='custom-control-label' for='longCheck");
    page += String(i) + F("'></label>");
		page += F("</div>");
    page += F("</div>");

    page += F("</div>");
  }

  page += get_footer_page();

  return page;
}

String get_interfaces_page() {

  String page = "";

  page += get_top_page(4);

  page += F("<p></p>");
  page += F("<div class='row'>");
  page += F("<div class='col-2'>");
  page += F("<span class='badge badge-primary'>USB MIDI</span>");
  page += F("</div>");
  page += F("<div class='col-2'>");
  page += F("<span class='badge badge-primary'>Legacy MIDI</span>");
  page += F("</div>");
  page += F("<div class='col-2'>");
  page += F("<span class='badge badge-primary'>RTP-MIDI</span>");
  page += F("</div>");
  page += F("<div class='col-2'>");
  page += F("<span class='badge badge-primary'>IPMIDI</span>");
  page += F("</div>");
  page += F("<div class='col-2'>");
  page += F("<span class='badge badge-primary'>BLE MIDI</span>");
  page += F("</div>");
  page += F("<div class='col-2'>");
  page += F("<span class='badge badge-primary'>OSC</span>");
  page += F("</div>");
  page += F("</div>");
  page += F("<p></p>");
  
  page += F("<div class='row'>");
  for (unsigned int i = 1; i <= 6; i++) {
    page += F("<div class='col-2'>");
    page += F("<div class='custom-control custom-checkbox'>");
	  page += F("<input type='checkbox' class='custom-control-input' id='inCheck");
    page += String(i) + F("' name='in") + String(i) + F("'>");
	  page += F("<label class='custom-control-label' for='inCheck");
    page += String(i) + F("'>In</label>");
	  page += F("</div>");
    page += F("</div>");
  }
  page += F("</div>");
  page += F("<div class='row'>");
  for (unsigned int i = 1; i <= 6; i++) {
    page += F("<div class='col-2'>");
    page += F("<div class='custom-control custom-checkbox'>");
	  page += F("<input type='checkbox' class='custom-control-input' id='outCheck");
    page += String(i) + F("' name='out") + String(i) + F("'>");
	  page += F("<label class='custom-control-label' for='outCheck");
    page += String(i) + F("'>Out</label>");
	  page += F("</div>");
    page += F("</div>");
  }
  page += F("</div>");
  page += F("<div class='row'>");
  for (unsigned int i = 1; i <= 6; i++) {
    page += F("<div class='col-2'>");
    page += F("<div class='custom-control custom-checkbox'>");
	  page += F("<input type='checkbox' class='custom-control-input' id='thruCheck");
    page += String(i) + F("' name='thru") + String(i) + F("'>");
	  page += F("<label class='custom-control-label' for='thruCheck");
    page += String(i) + F("'>Thru</label>");
	  page += F("</div>");
    page += F("</div>");
  }
  page += F("</div>");
  page += F("<div class='row'>");
  for (unsigned int i = 1; i <= 6; i++) {
    page += F("<div class='col-2'>");
    page += F("<div class='custom-control custom-checkbox'>");
	  page += F("<input type='checkbox' class='custom-control-input' id='routingCheck");
    page += String(i) + F("' name='routing") + String(i) + F("'>");
	  page += F("<label class='custom-control-label' for='routingCheck");
    page += String(i) + F("'>Routing</label>");
	  page += F("</div>");
    page += F("</div>");
  }
  page += F("</div>");
  page += F("<div class='row'>");
  for (unsigned int i = 1; i <= 6; i++) {
    page += F("<div class='col-2'>");
    page += F("<div class='custom-control custom-checkbox'>");
	  page += F("<input type='checkbox' class='custom-control-input' id='clockCheck");
    page += String(i) + F("' name='clock") + String(i) + F("'>");
	  page += F("<label class='custom-control-label' for='clockCheck");
    page += String(i) + F("'>Clock</label>");
	  page += F("</div>");
    page += F("</div>");
  }
  page += F("</div>");
  
  page += get_footer_page();

  return page;
}

String get_options_page() {

  String page = "";

  page += get_top_page(5);

  page += F("<p></p>");
  
  page += get_footer_page();

  return page;
}

void http_handle_root() { 
  
  if (httpServer.hasArg("theme")) {
    theme = httpServer.arg("theme");
    httpServer.send(200, "text/html", get_root_page());
  } else if (httpServer.hasArg("bank")) {
    bank = httpServer.arg("bank");
    httpServer.send(200, "text/html", get_banks_page());
  } else {
    httpServer.send(200, "text/html", get_root_page());
  }
  return; 
}

void http_handle_live() {
  if (httpServer.hasArg("theme")) theme = httpServer.arg("theme");
  httpServer.send(200, "text/html", get_live_page());
}

void http_handle_banks() {
  if (httpServer.hasArg("theme")) theme = httpServer.arg("theme");
  if (httpServer.hasArg("bank"))  bank  = httpServer.arg("bank");
  httpServer.send(200, "text/html", get_banks_page());
}

void http_handle_pedals() {
  if (httpServer.hasArg("theme")) theme = httpServer.arg("theme");
  httpServer.send(200, "text/html", get_pedals_page());
}

void http_handle_interfaces() {
  if (httpServer.hasArg("theme")) theme = httpServer.arg("theme");
  httpServer.send(200, "text/html", get_interfaces_page());
}

void http_handle_options() {
  if (httpServer.hasArg("theme")) theme = httpServer.arg("theme");
  httpServer.send(200, "text/html", get_options_page());
}

void http_handle_not_found() {

  String message = "File Not Found\n\n";

  message += "URI: ";
  message += httpServer.uri();
  message += "\nMethod: ";
  message += (httpServer.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += httpServer.args();
  message += "\n";

  for (uint8_t i = 0; i < httpServer.args(); i++) {
    message += " " + httpServer.argName(i) + ": " + httpServer.arg(i) + "\n";
  }

  httpServer.send(404, "text/plain", message);
}
#endif


void WiFiEvent(WiFiEvent_t event) {

  IPAddress localMask;

/*
    ESP8266 events

typedef enum WiFiEvent 
{
    WIFI_EVENT_STAMODE_CONNECTED = 0,
    WIFI_EVENT_STAMODE_DISCONNECTED,
    WIFI_EVENT_STAMODE_AUTHMODE_CHANGE,
    WIFI_EVENT_STAMODE_GOT_IP,
    WIFI_EVENT_STAMODE_DHCP_TIMEOUT,
    WIFI_EVENT_SOFTAPMODE_STACONNECTED,
    WIFI_EVENT_SOFTAPMODE_STADISCONNECTED,
    WIFI_EVENT_SOFTAPMODE_PROBEREQRECVED,
    WIFI_EVENT_MAX,
    WIFI_EVENT_ANY = WIFI_EVENT_MAX,
    WIFI_EVENT_MODE_CHANGE
} WiFiEvent_t;
*/

#ifdef ARDUINO_ARCH_ESP8266
    switch(event) {
        case WIFI_EVENT_STAMODE_CONNECTED:
          uint8_t macAddr[6];
          WiFi.macAddress(macAddr);
          DPRINTLN("BSSID       : %s", WiFi.BSSIDstr().c_str());
          DPRINTLN("RSSI        : %d dBm", WiFi.RSSI());
          DPRINTLN("Channel     : %d", WiFi.channel());
          DPRINTLN("STA         : %02X:%02X:%02X:%02X:%02X:%02X", macAddr[0], macAddr[1], macAddr[2], macAddr[3], macAddr[4], macAddr[5]);
          WiFi.hostname(host);
          break;

        case WIFI_EVENT_STAMODE_GOT_IP:
          DPRINTLN("Hostname    : %s", WiFi.hostname().c_str());
          DPRINTLN("IP address  : %s", WiFi.localIP().toString().c_str());
          DPRINTLN("Subnet mask : %s", WiFi.subnetMask().toString().c_str());
          DPRINTLN("Gataway IP  : %s", WiFi.gatewayIP().toString().c_str());
          DPRINTLN("DNS 1       : %s", WiFi.dnsIP(0).toString().c_str());
          DPRINTLN("DNS 2       : %s", WiFi.dnsIP(1).toString().c_str());

          // Start LLMNR (Link-Local Multicast Name Resolution) responder
          LLMNR.begin(host);
          DPRINT("LLMNR responder started\n");

          // Start mDNS (Multicast DNS) responder (ping pedalino.local)
          if (MDNS.begin(host)) {
            DPRINTLN("mDNS responder started");
            // service name is lower case
            // ESP8266 only: do not add '_' to service name and protocol
            MDNS.addService("apple-midi", "udp", 5004);
            MDNS.addService("osc",        "udp", oscLocalPort);
#ifdef PEDALINO_TELNET_DEBUG
            MDNS.addService("telnet", "tcp", 23);
#endif
          }

          // Start firmawre update via HTTP (connect to http://pedalino.local/update)
          httpUpdater.setup(&httpServer);

#ifndef NOWEBCONFIG
          httpServer.on("/", http_handle_root);
          httpServer.on("/live", http_handle_live);
          httpServer.on("/banks", http_handle_banks);
          httpServer.on("/pedals", http_handle_pedals);
          httpServer.on("/interfaces", http_handle_interfaces);
          httpServer.on("/options", http_handle_options);
          httpServer.onNotFound(http_handle_not_found);
#endif
          httpServer.begin();
          MDNS.addService("http", "tcp", 80);
          DPRINTLN("HTTP server started");
          DPRINTLN("Connect to http://pedalino.local/update for firmware update");

          // ipMIDI
          ipMIDI.beginMulticast(WiFi.localIP(), ipMIDImulticast, ipMIDIdestPort);
          DPRINTLN("ipMIDI server started");

          // RTP-MDI
          apple_midi_start();
          DPRINTLN("RTP-MIDI started");

          // Calculate the broadcast address of local WiFi to broadcast OSC messages
          oscRemoteIp = WiFi.localIP();
          localMask = WiFi.subnetMask();
          for (int i = 0; i < 4; i++)
            oscRemoteIp[i] |= (localMask[i] ^ B11111111);

          // Set incoming OSC messages port
          oscUDP.begin(oscLocalPort);
          DPRINTLN("OSC server started");
          break;

        case WIFI_EVENT_STAMODE_DHCP_TIMEOUT:
          DPRINTLN("WIFI_EVENT_STAMODE_DHCP_TIMEOUT");  
          break;

        case WIFI_EVENT_STAMODE_DISCONNECTED:
          DPRINTLN("WIFI_EVENT_STAMODE_DISCONNECTED");
          httpServer.stop();
          ipMIDI.stop();
          oscUDP.stop();   
          break;
        
        case WIFI_EVENT_SOFTAPMODE_STACONNECTED:
          break;

        case WIFI_EVENT_SOFTAPMODE_STADISCONNECTED:
          break;

        default:
          DPRINTLN("Event: %d", event);
          break;
    }
#endif

/* 
    ESP32 events

SYSTEM_EVENT_WIFI_READY               < ESP32 WiFi ready
SYSTEM_EVENT_SCAN_DONE                < ESP32 finish scanning AP
SYSTEM_EVENT_STA_START                < ESP32 station start
SYSTEM_EVENT_STA_STOP                 < ESP32 station stop
SYSTEM_EVENT_STA_CONNECTED            < ESP32 station connected to AP
SYSTEM_EVENT_STA_DISCONNECTED         < ESP32 station disconnected from AP
SYSTEM_EVENT_STA_AUTHMODE_CHANGE      < the auth mode of AP connected by ESP32 station changed
SYSTEM_EVENT_STA_GOT_IP               < ESP32 station got IP from connected AP
SYSTEM_EVENT_STA_LOST_IP              < ESP32 station lost IP and the IP is reset to 0
SYSTEM_EVENT_STA_WPS_ER_SUCCESS       < ESP32 station wps succeeds in enrollee mode
SYSTEM_EVENT_STA_WPS_ER_FAILED        < ESP32 station wps fails in enrollee mode
SYSTEM_EVENT_STA_WPS_ER_TIMEOUT       < ESP32 station wps timeout in enrollee mode
SYSTEM_EVENT_STA_WPS_ER_PIN           < ESP32 station wps pin code in enrollee mode
SYSTEM_EVENT_AP_START                 < ESP32 soft-AP start
SYSTEM_EVENT_AP_STOP                  < ESP32 soft-AP stop
SYSTEM_EVENT_AP_STACONNECTED          < a station connected to ESP32 soft-AP
SYSTEM_EVENT_AP_STADISCONNECTED       < a station disconnected from ESP32 soft-AP
SYSTEM_EVENT_AP_PROBEREQRECVED        < Receive probe request packet in soft-AP interface
SYSTEM_EVENT_GOT_IP6                  < ESP32 station or ap or ethernet interface v6IP addr is preferred
SYSTEM_EVENT_ETH_START                < ESP32 ethernet start
SYSTEM_EVENT_ETH_STOP                 < ESP32 ethernet stop
SYSTEM_EVENT_ETH_CONNECTED            < ESP32 ethernet phy link up
SYSTEM_EVENT_ETH_DISCONNECTED         < ESP32 ethernet phy link down
SYSTEM_EVENT_ETH_GOT_IP               < ESP32 ethernet got IP from connected AP
SYSTEM_EVENT_MAX
*/
#ifdef ARDUINO_ARCH_ESP32
    switch(event) {
        case SYSTEM_EVENT_STA_START:
          DPRINTLN("SYSTEM_EVENT_STA_START");
          break;

        case SYSTEM_EVENT_STA_STOP:
          DPRINTLN("SYSTEM_EVENT_STA_STOP");
          break;

        case SYSTEM_EVENT_WIFI_READY:
          DPRINTLN("SYSTEM_EVENT_WIFI_READY");
          break;

        case SYSTEM_EVENT_STA_CONNECTED:
          DPRINTLN("SYSTEM_EVENT_STA_CONNECTED");
          uint8_t macAddr[6];
          WiFi.macAddress(macAddr);
          DPRINTLN("BSSID       : %s", WiFi.BSSIDstr().c_str());
          DPRINTLN("RSSI        : %d dBm", WiFi.RSSI());
          DPRINTLN("Channel     : %d", WiFi.channel());
          DPRINTLN("STA         : %02X:%02X:%02X:%02X:%02X:%02X", macAddr[0], macAddr[1], macAddr[2], macAddr[3], macAddr[4], macAddr[5]);
          WiFi.setHostname(host);
          break;

        case SYSTEM_EVENT_STA_GOT_IP:
          DPRINTLN("SYSTEM_EVENT_STA_GOT_IP");
          DPRINTLN("Hostname    : %s", WiFi.getHostname());   
          DPRINTLN("IP address  : %s", WiFi.localIP().toString().c_str());
          DPRINTLN("Subnet mask : %s", WiFi.subnetMask().toString().c_str());
          DPRINTLN("Gataway IP  : %s", WiFi.gatewayIP().toString().c_str());
          DPRINTLN("DNS 1       : %s", WiFi.dnsIP(0).toString().c_str());
          DPRINTLN("DNS 2       : %s", WiFi.dnsIP(1).toString().c_str());
          
          serialize_wifi_status(true);

          // Start mDNS (Multicast DNS) responder (ping pedalino.local)
          if (MDNS.begin(host)) {
            DPRINTLN("mDNS responder started");
            // service name is lower case
            // service name and protocol starts with an '_' e.g. '_udp'
            MDNS.addService("_apple-midi", "_udp", 5004);
            MDNS.addService("_osc",        "_udp", oscLocalPort);
#ifdef PEDALINO_TELNET_DEBUG
            MDNS.addService("_telnet", "_tcp", 23);
#endif            
          }

          ipMIDI.beginMulticast(ipMIDImulticast, ipMIDIdestPort);
          DPRINTLN("ipMIDI server started");

          // RTP-MDI
          apple_midi_start();
          DPRINTLN("RTP-MIDI started");

          // Calculate the broadcast address of local WiFi to broadcast OSC messages
          oscRemoteIp = WiFi.localIP();
          localMask = WiFi.subnetMask();
          for (int i = 0; i < 4; i++)
            oscRemoteIp[i] |= (localMask[i] ^ B11111111);

          // Set incoming OSC messages port
          oscUDP.begin(oscLocalPort);
          DPRINTLN("OSC server started");
          break;

        case SYSTEM_EVENT_STA_LOST_IP:
          DPRINTLN("SYSTEM_EVENT_STA_LOST_IP");
          MDNS.end();
          ipMIDI.stop();
          oscUDP.stop();
          serialize_wifi_status(false);
          break;

        case SYSTEM_EVENT_STA_DISCONNECTED:
          DPRINTLN("SYSTEM_EVENT_STA_DISCONNECTED");
          MDNS.end();
          ipMIDI.stop();
          oscUDP.stop();
          serialize_wifi_status(false);
          break;
        
        case SYSTEM_EVENT_AP_START:
          DPRINTLN("SYSTEM_EVENT_AP_START");
          break;

        case SYSTEM_EVENT_AP_STOP:
          DPRINTLN("SYSTEM_EVENT_AP_STOP");
          break;

        case SYSTEM_EVENT_AP_STACONNECTED:
          DPRINTLN("SYSTEM_EVENT_AP_STACONNECTED");
          break;

        case SYSTEM_EVENT_AP_STADISCONNECTED:
          DPRINTLN("SYSTEM_EVENT_AP_STADISCONNECTED");
          break;

        case SYSTEM_EVENT_AP_PROBEREQRECVED:
          DPRINTLN("SYSTEM_EVENT_AP_PROBEREQRECVED");
          break;

        default:
          DPRINTLN("Event: %d", event);
          break;
    }
#endif
}


void status_blink()
{
  WIFI_LED_ON();
  delay(50);
  WIFI_LED_OFF();
}

void ap_mode_start()
{
  WIFI_LED_OFF();

  WiFi.mode(WIFI_AP);
  if (WiFi.softAP("Pedalino")) {
    DPRINTLN("AP mode started");
    DPRINTLN("Connect to 'Pedalino' wireless with no password");
  }
  else {
    DPRINTLN("AP mode failed");
  }
}

void ap_mode_stop()
{
  WIFI_LED_OFF();

  if (WiFi.getMode() == WIFI_AP) {
    if (WiFi.softAPdisconnect())
      DPRINTLN("AP mode disconnected");
    else 
      DPRINTLN("AP mode disconnected failed");
  }
}

bool smart_config()
{
  // Return 'true' if SSID and password received within SMART_CONFIG_TIMEOUT seconds

  // Re-establish lost connection to the AP
  WiFi.setAutoReconnect(true);

  // Automatically connect on power on to the last used access point
  WiFi.setAutoConnect(true);

  // Waiting for SSID and password from from mobile app
  // SmartConfig works only in STA mode
  WiFi.mode(WIFI_STA);
  WiFi.beginSmartConfig();

  DPRINT("SmartConfig started");

  for (int i = 0; i < SMART_CONFIG_TIMEOUT && !WiFi.smartConfigDone(); i++) {
    status_blink();
    delay(950);
  }

  if (WiFi.smartConfigDone())
  {
    DPRINTLN("SSID        : %s", WiFi.SSID().c_str());
    DPRINTLN("Password    : %s", WiFi.psk().c_str());

    save_wifi_credentials(WiFi.SSID(), WiFi.psk());
  }
  else
    DPRINT("SmartConfig timeout");

  if (WiFi.smartConfigDone())
  {
    WiFi.stopSmartConfig();
    return true;
  }
  else
  {
    WiFi.stopSmartConfig();
    return false;
  }
}

bool ap_connect(String ssid = "", String password = "")
{
  // Return 'true' if connected to the access point within WIFI_CONNECT_TIMEOUT seconds

  DPRINTLN("Connecting to");
  DPRINTLN("SSID        : %s", ssid.c_str());
  DPRINTLN("Password    : %s", password.c_str());

  if (ssid.length() == 0) return false;

  WiFi.disconnect();
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid.c_str(), password.c_str());
  for (byte i = 0; i < WIFI_CONNECT_TIMEOUT * 2 && !WiFi.isConnected(); i++) {
    status_blink();
    delay(100);
    status_blink();
    delay(300);
  }

  WiFi.isConnected() ? WIFI_LED_ON() : WIFI_LED_OFF();

  return WiFi.isConnected();
}


bool auto_reconnect(String ssid = "", String password = "")
{
  // Return 'true' if connected to the (last used) access point within WIFI_CONNECT_TIMEOUT seconds

  if (ssid.length() == 0) {

#ifdef ARDUINO_ARCH_ESP8266
    ssid = WiFi.SSID();
    password = WiFi.psk();
#endif

#ifdef ARDUINO_ARCH_ESP32
    int address = 32;
    ssid = EEPROM.readString(address);
    address += ssid.length() + 1;
    password = EEPROM.readString(address);
#endif
  }

  if (ssid.length() == 0) return false;

  DPRINTLN("Connecting to");
  DPRINTLN("SSID        : %s", ssid.c_str());
  DPRINTLN("Password    : %s", password.c_str());

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid.c_str(), password.c_str());
  for (byte i = 0; i < WIFI_CONNECT_TIMEOUT * 2 && !WiFi.isConnected(); i++) {
    status_blink();
    delay(100);
    status_blink();
    delay(300);
  }

  WiFi.isConnected() ? WIFI_LED_ON() : WIFI_LED_OFF();

  return WiFi.isConnected();
}

void wifi_connect()
{
  if (!auto_reconnect())       // WIFI_CONNECT_TIMEOUT seconds to reconnect to last used access point
    if (smart_config())        // SMART_CONFIG_TIMEOUT seconds to receive SmartConfig parameters
      auto_reconnect();        // WIFI_CONNECT_TIMEOUT seconds to connect to SmartConfig access point
  if (!WiFi.isConnected())
    ap_mode_start();           // switch to AP mode until next reboot
}
#endif  // NOWIFI


void serial_midi_connect()
{
  // Connect the handle function called upon reception of a MIDI message from serial MIDI interface
  MIDI.setHandleNoteOn(OnSerialMidiNoteOn);
  MIDI.setHandleNoteOff(OnSerialMidiNoteOff);
  MIDI.setHandleAfterTouchPoly(OnSerialMidiAfterTouchPoly);
  MIDI.setHandleControlChange(OnSerialMidiControlChange);
  MIDI.setHandleProgramChange(OnSerialMidiProgramChange);
  MIDI.setHandleAfterTouchChannel(OnSerialMidiAfterTouchChannel);
  MIDI.setHandlePitchBend(OnSerialMidiPitchBend);
  MIDI.setHandleSystemExclusive(OnSerialMidiSystemExclusive);
  MIDI.setHandleTimeCodeQuarterFrame(OnSerialMidiTimeCodeQuarterFrame);
  MIDI.setHandleSongPosition(OnSerialMidiSongPosition);
  MIDI.setHandleSongSelect(OnSerialMidiSongSelect);
  MIDI.setHandleTuneRequest(OnSerialMidiTuneRequest);
  MIDI.setHandleClock(OnSerialMidiClock);
  MIDI.setHandleStart(OnSerialMidiStart);
  MIDI.setHandleContinue(OnSerialMidiContinue);
  MIDI.setHandleStop(OnSerialMidiStop);
  MIDI.setHandleActiveSensing(OnSerialMidiActiveSensing);
  MIDI.setHandleSystemReset(OnSerialMidiSystemReset);

  // Initiate serial MIDI communications, listen to all channels
  MIDI.begin(MIDI_CHANNEL_OMNI);
  MIDI.turnThruOff();
}

#ifdef BLYNK
#define BLYNK_SCANWIFI      V91
#define BLYNK_SSID          V92
#define BLYNK_PASSWORD      V93
#define BLYNK_WIFICONNECT   V94
#define BLYNK_SMARTCONFIG   V95

String ssid;
String password;

BLYNK_CONNECTED() {
  // This function is called when hardware connects to Blynk Cloud or private server.
  DPRINTLN("Connected to Blynk Cloud");
  blynkLCD.clear();
  Blynk.virtualWrite(BLYNK_WIFICONNECT, 0);
  Blynk.virtualWrite(BLYNK_SCANWIFI, 0);
  Blynk.setProperty(BLYNK_SSID, "labels", "");
  Blynk.virtualWrite(BLYNK_PASSWORD, "");
  Blynk.virtualWrite(BLYNK_SMARTCONFIG, 0);
}

BLYNK_APP_CONNECTED() {
  //  This function is called every time Blynk app client connects to Blynk server.
  DPRINTLN("Blink App connected");
}

BLYNK_APP_DISCONNECTED() {
  // This function is called every time the Blynk app disconnects from Blynk Cloud or private server.
  DPRINTLN("Blink App disconnected");
}


BLYNK_WRITE(BLYNK_WIFICONNECT) {
  WiFi.scanDelete();
  if (ap_connect(ssid, password))
    Blynk.connect();
  else if (auto_reconnect())
    Blynk.connect();
  Blynk.virtualWrite(BLYNK_WIFICONNECT, 0);
}

/*
  BLYNK_WRITE(BLYNK_WIFISTATUS) {
  int wifiOnOff = param.asInt();
  switch (wifiOnOff) {
    case 0: // OFF
      WiFi.mode(WIFI_OFF);
      DPRINTLN("WiFi Off Mode");
      break;

    case 1: // ON
      wifi_connect();
      Blynk.connect();
      break;
  }
  }
*/

BLYNK_WRITE(BLYNK_SCANWIFI) {
  int scan = param.asInt();
  if (scan) {
    DPRINTLN("WiFi Scan started");
    int networksFound = WiFi.scanNetworks();
    DPRINTLN("WiFi Scan done");
    if (networksFound == 0) {
      DPRINTLN("No networks found");
    } else {
      DPRINTLN("%d network(s) found", networksFound);
      BlynkParamAllocated items(512); // list length, in bytes
      for (int i = 0; i < networksFound; i++) {
#ifdef ARDUINO_ARCH_ESP32
        DPRINTLN("%2d.\n BSSID: %s\n SSID: %s\n Channel: %d\n Signal: %d dBm\n Auth Mode: %s", i + 1, WiFi.BSSIDstr(i).c_str(), WiFi.SSID(i).c_str(), WiFi.channel(i), WiFi.RSSI(i), translateEncryptionType(WiFi.encryptionType(i)).c_str());
#endif
        items.add(WiFi.SSID(i).c_str());
      }
      Blynk.setProperty(BLYNK_SSID, "labels", items);
      if (networksFound > 0) {
        Blynk.virtualWrite(BLYNK_SSID, 1);
        ssid = WiFi.SSID(0);
      }
      Blynk.virtualWrite(BLYNK_SCANWIFI, 0);
    }
  }
}

BLYNK_WRITE(BLYNK_SSID) {
  int i = param.asInt();
  ssid = WiFi.SSID(i - 1);
  DPRINTLN("SSID     : %s", ssid.c_str());
}

BLYNK_WRITE(BLYNK_PASSWORD) {
  password = param.asStr();
  DPRINTLN("Password : %s", password.c_str());
}

BLYNK_WRITE(BLYNK_SMARTCONFIG) {
  int smartconfig = param.asInt();
  if (smartconfig) {
    smart_config();
    if (auto_reconnect())
      Blynk.connect();
  }
}
#endif


void setup()
{
#ifdef SERIALDEBUG
  SERIALDEBUG.begin(115200);
  SERIALDEBUG.setDebugOutput(true);   // enable diagnostic output and printf() output
#endif

  DPRINTLN("  __________           .___      .__  .__                   ___ ________________    ___");
  DPRINTLN("  \\______   \\ ____   __| _/____  |  | |__| ____   ____     /  / \\__    ___/     \\   \\  \\");
  DPRINTLN("   |     ___// __ \\ / __ |\\__  \\ |  | |  |/    \\ /  _ \\   /  /    |    | /  \\ /  \\   \\  \\");
  DPRINTLN("   |    |   \\  ___// /_/ | / __ \\|  |_|  |   |  (  <_> ) (  (     |    |/    Y    \\   )  )");
  DPRINTLN("   |____|    \\___  >____ |(____  /____/__|___|  /\\____/   \\  \\    |____|\\____|__  /  /  /");
  DPRINTLN("                 \\/     \\/     \\/             \\/           \\__\\                 \\/  /__/");
  DPRINTLN("                                                                (c) 2018 alf45star");
  DPRINTLN("                                                        https://github.com/alf45tar/Pedalino");

#ifdef ARDUINO_ARCH_ESP32
  esp_log_level_set("*",      ESP_LOG_ERROR);
  //esp_log_level_set("wifi",   ESP_LOG_WARN);
  //esp_log_level_set("BLE*",   ESP_LOG_ERROR);
  esp_log_level_set(LOG_TAG,  ESP_LOG_INFO);
#endif

#ifdef ARDUINO_ARCH_ESP32
  if (!EEPROM.begin(128)) {
    DPRINTLN("Failed to initialise EEPROM");
    DPRINTLN("Restarting...");
    delay(1000);
    ESP.restart();
  }
#else
  EEPROM.begin(128);
#endif
  DPRINTLN("Reading EEPROM");
  int address = 0;
  for (byte i = 0; i < INTERFACES; i++)
    {
      interfaces[i].midiIn = EEPROM.read(address);
      address += sizeof(byte);
      interfaces[i].midiOut = EEPROM.read(address);
      address += sizeof(byte);
      interfaces[i].midiThru = EEPROM.read(address);
      address += sizeof(byte);
      interfaces[i].midiRouting = EEPROM.read(address);
      address += sizeof(byte);
      interfaces[i].midiClock = EEPROM.read(address);
      address += sizeof(byte);
      DPRINTLN("Interface %s:  %s  %s  %s  %s  %s",
                interfaces[i].name, 
                interfaces[i].midiIn      ? "IN"      : "  ",
                interfaces[i].midiOut     ? "OUT"     : "   ",
                interfaces[i].midiThru    ? "THRU"    : "    ",
                interfaces[i].midiRouting ? "ROUTING" : "       ",
                interfaces[i].midiClock   ? "CLOCK"   : "     ");

    }
#if defined(ARDUINO_ARCH_ESP32) && !defined(NOWIFI)
    address = 32;
    String ssid = EEPROM.readString(address);
    address += ssid.length() + 1;
    String password = EEPROM.readString(address);
    DPRINTLN("SSID     : %s", ssid.c_str());
    DPRINTLN("Password : %s", password.c_str());
#endif

#ifdef ARDUINO_ARCH_ESP8266
  SerialMIDI.begin(SERIALMIDI_BAUD_RATE);
#endif
#ifdef ARDUINO_ARCH_ESP32
  SerialMIDI.begin(SERIALMIDI_BAUD_RATE, SERIAL_8N1, SERIALMIDI_RX, SERIALMIDI_TX);
#endif

  serial_midi_connect();              // On receiving MIDI data callbacks setup
  DPRINTLN("Serial MIDI started");

  serialize_on();
  serialize_wifi_status(false);
  serialize_ble_status(false);
  
  pinMode(WIFI_LED, OUTPUT);

#ifdef ARDUINO_ARCH_ESP32
  pinMode(BLE_LED, OUTPUT);
#endif

#ifndef NOBLE
  // BLE MIDI service advertising
  ble_midi_start_service();
  DPRINT("BLE MIDI service advertising started");
#endif

#ifndef NOWIFI
  // Write SSID/password to flash only if currently used values do not match what is already stored in flash
  WiFi.persistent(false);
  WiFi.onEvent(WiFiEvent);
  wifi_connect();
#endif

#ifdef BLYNK
  // Connect to Blynk
  Blynk.config(blynkAuthToken);
  Blynk.connect();
#endif

#ifdef PEDALINO_TELNET_DEBUG
  // Initialize the telnet server of RemoteDebug
  Debug.begin(host);              // Initiaze the telnet server
  Debug.setResetCmdEnabled(true); // Enable the reset command
#endif
}


void loop()
{
  if (!appleMidiConnected) WIFI_LED_OFF();
  if (!bleMidiConnected)  BLE_LED_OFF();
  if (appleMidiConnected ||  bleMidiConnected) {
    // led fast blinking (5 times per second)
    if (millis() - wifiLastOn > 200) {
      if (bleMidiConnected) BLE_LED_ON();
      if (appleMidiConnected) WIFI_LED_ON();
      wifiLastOn = millis();
    }
    else if (millis() - wifiLastOn > 100) {
      BLE_LED_OFF();
      WIFI_LED_OFF();
    }
  }
#ifndef NOWIFI
  else
    // led always on if connected to an AP or one or more client connected the the internal AP
    switch (WiFi.getMode()) {
      case WIFI_STA:
        WiFi.isConnected() ? WIFI_LED_ON() : WIFI_LED_OFF();
        break;
      case WIFI_AP:
        WiFi.softAPgetStationNum() > 0 ? WIFI_LED_ON() : WIFI_LED_OFF();
        break;
      default:
        WIFI_LED_OFF();
        break;
    }
#endif

  // Listen to incoming messages from Arduino
  if (MIDI.read())
    DPRINTMIDI("Serial MIDI", MIDI.getType(), MIDI.getChannel(), MIDI.getData1(), MIDI.getData2());

  // Listen to incoming AppleMIDI messages from WiFi
  rtpMIDI_listen();

  // Listen to incoming ipMIDI messages from WiFi
  ipMIDI_listen();

  // Listen to incoming OSC UDP messages from WiFi
  oscUDP_listen();

#ifdef ARDUINO_ARCH_ESP8266
  // Run HTTP Updater
  httpServer.handleClient();
#endif

#ifdef BLYNK
  if (WiFi.isConnected())
    Blynk.run();
#endif

#ifdef PEDALINO_TELNET_DEBUG
  // Remote debug over telnet
  Debug.handle();
#endif
}