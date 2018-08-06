// ESP8266/ESP32 MIDI Gateway between Serial MIDI <-> WiFi AppleMIDI <-> Bluetooth LE MIDI <-> WiFi OSC

#include <Arduino.h>

#ifdef ARDUINO_ARCH_ESP8266
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ESP8266LLMNR.h>
#include <ESP8266HTTPUpdateServer.h>
#endif

#ifdef ARDUINO_ARCH_ESP32
#include <WiFi.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <Update.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#endif

#include <WiFiClient.h>
#include <WiFiUdp.h>

#include <MIDI.h>
#include <AppleMidi.h>
#include <OSCMessage.h>
#include <OSCBundle.h>
#include <OSCData.h>

//#define PEDALINO_SERIAL_DEBUG
//#define PEDALINO_TELNET_DEBUG

#ifdef PEDALINO_TELNET_DEBUG
#include "RemoteDebug.h"          // Remote debug over telnet - not recommended for production, only for development    
RemoteDebug Debug;
#endif

#define WIFI_CONNECT_TIMEOUT    10
#define SMART_CONFIG_TIMEOUT    30

#ifndef LED_BUILTIN
#define LED_BUILTIN    2
#endif

#define WIFI_LED       LED_BUILTIN  // onboard LED, used as status indicator

#if defined(ARDUINO_ARCH_ESP8266) && defined(PEDALINO_SERIAL_DEBUG)
#define SERIALDEBUG       Serial1
#define WIFI_LED       0  // ESP8266 only: onboard LED on GPIO2 is shared with Serial1 TX
#endif

#if defined(ARDUINO_ARCH_ESP32) && defined(PEDALINO_SERIAL_DEBUG)
#define SERIALDEBUG       Serial
#endif

#ifdef PEDALINO_SERIAL_DEBUG
#define DPRINT(...)       SERIALDEBUG.printf(__VA_ARGS__)    //DPRINT is a macro, debug print
#define DPRINTLN(...)     SERIALDEBUG.println(__VA_ARGS__)   //DPRINTLN is a macro, debug print with new line
#else
#define DPRINT(...)     //now defines a blank line
#define DPRINTLN(...)   //now defines a blank line
#endif

#ifdef ARDUINO_ARCH_ESP8266
#define BLE_LED_OFF()
#define BLE_LED_ON()
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

const char host[]     = "pedalino";

#ifdef ARDUINO_ARCH_ESP8266
ESP8266WebServer        httpServer(80);
ESP8266HTTPUpdateServer httpUpdater;
#endif

#ifdef ARDUINO_ARCH_ESP32
WebServer               httpServer(80);
HTTPUpload              httpUpdater;
#endif

// Bluetooth LE MIDI interface

#ifdef ARDUINO_ARCH_ESP32

#define MIDI_SERVICE_UUID        "03b80e5a-ede8-4b33-a751-6ce34ec4c700"
#define MIDI_CHARACTERISTIC_UUID "7772e5db-3868-4112-a1a9-f2669d106bf3"

BLEServer             *pServer;
BLEService            *pService;
BLECharacteristic     *pCharacteristic;
BLESecurity           *pSecurity;
unsigned long         msOffset = 0;
#define MAX_TIMESTAMP 0x01FFF         //13 bits, 8192 dec
#endif
bool                  bleMidiConnected = false;
unsigned long         bleLastOn        = 0;

// WiFi MIDI interface to comunicate with AppleMIDI/RTP-MDI devices

APPLEMIDI_CREATE_INSTANCE(WiFiUDP, AppleMIDI); // see definition in AppleMidi_Defs.h

bool          appleMidiConnected = false;
unsigned long wifiLastOn         = 0;

// Serial MIDI interface to comunicate with Arduino

#define SERIALMIDI_BAUD_RATE  115200

struct SerialMIDISettings : public midi::DefaultSettings
{
  static const long BaudRate = SERIALMIDI_BAUD_RATE;
};

#ifdef ARDUINO_ARCH_ESP8266
MIDI_CREATE_CUSTOM_INSTANCE(HardwareSerial, Serial, MIDI, SerialMIDISettings);
#endif

#ifdef ARDUINO_ARCH_ESP32
#define SERIALMIDI_RX         16
#define SERIALMIDI_TX         17
HardwareSerial                SerialMIDI(2);
MIDI_CREATE_CUSTOM_INSTANCE(HardwareSerial, SerialMIDI, MIDI, SerialMIDISettings);
#endif


// WiFi OSC comunication

WiFiUDP                 oscUDP;                  // A UDP instance to let us send and receive packets over UDP
IPAddress               oscRemoteIp;             // remote IP of an external OSC device or broadcast address
const unsigned int      oscRemotePort = 9000;    // remote port of an external OSC device
const unsigned int      oscLocalPort = 8000;     // local port to listen for OSC packets (actually not used for sending)
OSCMessage              oscMsg;
OSCErrorCode            oscError;


#ifdef ARDUINO_ARCH_ESP32

void BLEmidiReceive(uint8_t *, uint8_t);

class BLESCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      bleMidiConnected = true;
      DPRINTLN("BLE client connected");
    };

    void onDisconnect(BLEServer* pServer) {
      bleMidiConnected = false;
      DPRINTLN("BLE client disconnected");
    }
};

class BLECCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      std::string rxValue = pCharacteristic->getValue();
      if (rxValue.length() > 0) {
        BLEmidiReceive((uint8_t *)(rxValue.c_str()), rxValue.length());
      }
    }
};

void BLEmidiStart ()
{
  BLEDevice::init("Pedal");

  // Create the BLE Server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new BLESCallbacks());
  BLEDevice::setEncryptionLevel((esp_ble_sec_act_t)ESP_LE_AUTH_REQ_SC_BOND);

  // Create the BLE Service
  pService = pServer->createService(BLEUUID(MIDI_SERVICE_UUID));

  // Create a BLE Characteristic
  pCharacteristic = pService->createCharacteristic(
                      BLEUUID(MIDI_CHARACTERISTIC_UUID),
                      BLECharacteristic::PROPERTY_READ   |
                      BLECharacteristic::PROPERTY_NOTIFY |
                      BLECharacteristic::PROPERTY_WRITE_NR
                    );
  pCharacteristic->setAccessPermissions(ESP_GATT_PERM_READ_ENCRYPTED | ESP_GATT_PERM_WRITE_ENCRYPTED);
  pCharacteristic->setCallbacks(new BLECCallbacks());

  // https://www.bluetooth.com/specifications/gatt/viewer?attributeXmlFile=org.bluetooth.descriptor.gatt.client_characteristic_configuration.xml
  // Create a BLE Descriptor
  pCharacteristic->addDescriptor(new BLE2902());

  // Start the service
  pService->start();

  // Start advertising
  pSecurity = new BLESecurity();
  pSecurity->setAuthenticationMode(ESP_LE_AUTH_REQ_SC_BOND);
  pSecurity->setCapability(ESP_IO_CAP_NONE);
  pSecurity->setInitEncryptionKey(ESP_BLE_ENC_KEY_MASK | ESP_BLE_ID_KEY_MASK);

  pServer->getAdvertising()->addServiceUUID(MIDI_SERVICE_UUID);
  pServer->getAdvertising()->start();
  DPRINTLN("BLE MIDI advertising started");
}

void BLEmidiTimestamp (uint8_t *a, uint8_t *b)
{
  unsigned long currentMillis = millis();
  if (currentMillis < 5000) {
    if (msOffset > 5000) {
      //it's been 49 days! millis rolled.
      while (msOffset > 5000) {
        //roll msOffset - this should preserve current ~8 second count.
        msOffset += MAX_TIMESTAMP;
      }
    }
  }
  //if the offset is more than 2^13 ms away, move it up in 2^13 ms intervals
  while (currentMillis >= (unsigned long)(msOffset + MAX_TIMESTAMP)) {
    msOffset += MAX_TIMESTAMP;
  }
  unsigned long currentTimeStamp = currentMillis - msOffset;
  *a = ((currentTimeStamp >> 7) & 0x3F) | 0x80;     // 6 bits plus MSB
  *b = (currentTimeStamp & 0x7F) | 0x80;            // 7 bits plus MSB
}

// Check if MIDI data has come in through the serial port.  If found, it builds a characteristic buffer and sends it over BLE.
// https://learn.sparkfun.com/tutorials/midi-ble-tutorial

void BLEmidiSend()
{
  uint8_t midiPacket[5];

  BLEmidiTimestamp(&midiPacket[0], &midiPacket[1]);

  uint8_t statusByte = ((uint8_t)MIDI.getType() | ((MIDI.getChannel() - 1) & 0x0f));
  switch (MIDI.getType())
  {
    //2 Byte Channel Messages
    case midi::NoteOff :
    case midi::NoteOn :
    case midi::AfterTouchPoly :
    case midi::ControlChange :
    case midi::PitchBend :
      midiPacket[2] = statusByte;
      midiPacket[3] = MIDI.getData1();
      midiPacket[4] = MIDI.getData2();
      pCharacteristic->setValue(midiPacket, 5);
      pCharacteristic->notify();
      break;
    //1 Byte Channel Messages
    case midi::ProgramChange :
    case midi::AfterTouchChannel :
      midiPacket[2] = statusByte;
      midiPacket[3] = MIDI.getData1();
      pCharacteristic->setValue(midiPacket, 4);
      pCharacteristic->notify();
      break;
    //System Common Messages
    case midi::TimeCodeQuarterFrame :
      midiPacket[2] = 0xF1;
      midiPacket[3] = MIDI.getData1();
      pCharacteristic->setValue(midiPacket, 4);
      pCharacteristic->notify();
      break;
    case midi::SongPosition :
      midiPacket[2] = 0xF2;
      midiPacket[3] = MIDI.getData1();
      midiPacket[4] = MIDI.getData2();
      pCharacteristic->setValue(midiPacket, 5);
      pCharacteristic->notify();
      break;
    case midi::SongSelect :
      midiPacket[2] = 0xF3;
      midiPacket[3] = MIDI.getData1();
      pCharacteristic->setValue(midiPacket, 4);
      pCharacteristic->notify();
      break;
    case midi::TuneRequest :
      midiPacket[2] = 0xF6;
      pCharacteristic->setValue(midiPacket, 3);
      pCharacteristic->notify();
      break;
    //Real-time Messages
    case midi::Clock :
      midiPacket[2] = 0xF8;
      pCharacteristic->setValue(midiPacket, 3);
      pCharacteristic->notify();
      break;
    case midi::Start :
      midiPacket[2] = 0xFA;
      pCharacteristic->setValue(midiPacket, 3);
      pCharacteristic->notify();
      break;
    case midi::Continue :
      midiPacket[2] = 0xFB;
      pCharacteristic->setValue(midiPacket, 3);
      pCharacteristic->notify();
      break;
    case midi::Stop :
      midiPacket[2] = 0xFC;
      pCharacteristic->setValue(midiPacket, 3);
      pCharacteristic->notify();
      break;
    case midi::ActiveSensing :
      midiPacket[2] = 0xFE;
      pCharacteristic->setValue(midiPacket, 3);
      pCharacteristic->notify();
      break;
    case midi::SystemReset :
      midiPacket[2] = 0xFF;
      pCharacteristic->setValue(midiPacket, 3);
      pCharacteristic->notify();
      break;
    //SysEx
    case midi::SystemExclusive :
      //              {
      //                  // Sysex is special.
      //                  // could contain very long data...
      //                  // the data bytes form the length of the message,
      //                  // with data contained in array member
      //                  uint16_t length;
      //                  const uint8_t  * data_p;
      //
      //                  Serial.print("SysEx, chan: ");
      //                  Serial.print(MIDI.getChannel());
      //                  length = MIDI.getSysExArrayLength();
      //
      //                  Serial.print(" Data: 0x");
      //                  data_p = MIDI.getSysExArray();
      //                  for (uint16_t idx = 0; idx < length; idx++)
      //                  {
      //                      Serial.print(data_p[idx], HEX);
      //                      Serial.print(" 0x");
      //                  }
      //                  Serial.println();
      //              }
      break;
    case midi::InvalidType :
    default:
      break;
  }
}

// Decodes the BLE characteristics and calls MIDI.send if the packet contains sendable MIDI data

void BLEmidiReceive(uint8_t *buffer, uint8_t bufferSize)
{
  midi::Channel   channel;
  midi::MidiType  command;

  //Pointers used to search through payload.
  uint8_t lPtr = 0;
  uint8_t rPtr = 0;
  //lastStatus used to capture runningStatus
  uint8_t lastStatus;
  //Decode first packet -- SHALL be "Full MIDI message"
  lPtr = 2; //Start at first MIDI status -- SHALL be "MIDI status"
  //While statement contains incrementing pointers and breaks when buffer size exceeded.
  while (1) {
    lastStatus = buffer[lPtr];
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

  BLEmidiTimestamp(&midiPacket[0], &midiPacket[1]);
  midiPacket[2] = (type & 0xf0) | (channel & 0x0f);
  midiPacket[3] = data1;
  pCharacteristic->setValue(midiPacket, 4);
  pCharacteristic->notify();
}

void BLESendChannelMessage2(byte type, byte channel, byte data1, byte data2)
{
  uint8_t midiPacket[5];

  BLEmidiTimestamp(&midiPacket[0], &midiPacket[1]);
  midiPacket[2] = (type & 0xf0) | (channel & 0x0f);
  midiPacket[3] = data1;
  midiPacket[4] = data2;
  pCharacteristic->setValue(midiPacket, 5);
  pCharacteristic->notify();
}

void BLESendSystemCommonMessage1(byte type, byte data1)
{
  uint8_t midiPacket[4];

  BLEmidiTimestamp(&midiPacket[0], &midiPacket[1]);
  midiPacket[2] = type;
  midiPacket[3] = data1;
  pCharacteristic->setValue(midiPacket, 4);
  pCharacteristic->notify();
}

void BLESendSystemCommonMessage2(byte type, byte data1, byte data2)
{
  uint8_t midiPacket[5];

  BLEmidiTimestamp(&midiPacket[0], &midiPacket[1]);
  midiPacket[2] = type;
  midiPacket[3] = data1;
  midiPacket[4] = data2;
  pCharacteristic->setValue(midiPacket, 5);
  pCharacteristic->notify();
}

void BLESendRealTimeMessage(byte type)
{
  uint8_t midiPacket[3];

  BLEmidiTimestamp(&midiPacket[0], &midiPacket[1]);
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
#else
#define BLEmidiStart(...)
#define BLEmidiSend(...)
#define BLEmidiReceive(...)
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
#endif


// Send messages to WiFI OSC interface

void OSCSendNoteOn(byte note, byte velocity, byte channel)
{
  String msg = "/pedalino/midi/note/";
  msg += note;
  OSCMessage oscMsg(msg.c_str());
  oscUDP.beginPacket(oscRemoteIp, oscRemotePort);
  oscMsg.add((float)(velocity / 127.0)).add((int32_t)channel).send(oscUDP).empty();
  oscUDP.endPacket();
}

void OSCSendNoteOff(byte note, byte velocity, byte channel)
{
  String msg = "/pedalino/midi/note/";
  msg += note;
  OSCMessage oscMsg(msg.c_str());
  oscUDP.beginPacket(oscRemoteIp, oscRemotePort);
  oscMsg.add((float)0).add((int32_t)channel).send(oscUDP).empty();
  oscUDP.endPacket();
}

void OSCSendAfterTouchPoly(byte note, byte pressure, byte channel)
{
  String msg = "/pedalino/midi/aftertouchpoly/";
  msg += note;
  OSCMessage oscMsg(msg.c_str());
  oscUDP.beginPacket(oscRemoteIp, oscRemotePort);
  oscMsg.add((float)(pressure / 127.0)).add((int32_t)channel).send(oscUDP).empty();
  oscUDP.endPacket();
}

void OSCSendControlChange(byte number, byte value, byte channel)
{
  String msg = "/pedalino/midi/cc/";
  msg += number;
  OSCMessage oscMsg(msg.c_str());
  oscUDP.beginPacket(oscRemoteIp, oscRemotePort);
  oscMsg.add((float)(value / 127.0)).add((int32_t)channel).send(oscUDP).empty();
  oscUDP.endPacket();
}

void OSCSendProgramChange(byte number, byte channel)
{
  String msg = "/pedalino/midi/pc/";
  msg += number;
  OSCMessage oscMsg(msg.c_str());
  oscUDP.beginPacket(oscRemoteIp, oscRemotePort);
  oscMsg.add((int32_t)channel).send(oscUDP).empty();
  oscUDP.endPacket();
}

void OSCSendAfterTouch(byte pressure, byte channel)
{
  String msg = "/pedalino/midi/aftertouchchannel/";
  msg += channel;
  OSCMessage oscMsg(msg.c_str());
  oscUDP.beginPacket(oscRemoteIp, oscRemotePort);
  oscMsg.add((float)(pressure / 127.0)).add((int32_t)channel).send(oscUDP).empty();
  oscUDP.endPacket();
}

void OSCSendPitchBend(int bend, byte channel)
{
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
  String msg = "/pedalino/midi/songpostion/";
  msg += beats;
  OSCMessage oscMsg(msg.c_str());
  oscUDP.beginPacket(oscRemoteIp, oscRemotePort);
  oscMsg.add((int32_t)beats).send(oscUDP).empty();
  oscUDP.endPacket();
}

void OSCSendSongSelect(byte songnumber)
{
  String msg = "/pedalino/midi/songselect/";
  msg += songnumber;
  OSCMessage oscMsg(msg.c_str());
  oscUDP.beginPacket(oscRemoteIp, oscRemotePort);
  oscMsg.add((int32_t)songnumber).send(oscUDP).empty();
  oscUDP.endPacket();
}

void OSCSendTuneRequest(void)
{
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
  OSCMessage oscMsg("/pedalino/midi/start/");
  oscUDP.beginPacket(oscRemoteIp, oscRemotePort);
  oscMsg.send(oscUDP).empty();
  oscUDP.endPacket();
}

void OSCSendContinue(void)
{
  OSCMessage oscMsg("/pedalino/midi/continue/");
  oscUDP.beginPacket(oscRemoteIp, oscRemotePort);
  oscMsg.send(oscUDP).empty();
  oscUDP.endPacket();
}

void OSCSendStop(void)
{
  OSCMessage oscMsg("/pedalino/midi/stop/");
  oscUDP.beginPacket(oscRemoteIp, oscRemotePort);
  oscMsg.send(oscUDP).empty();
  oscUDP.endPacket();
}

void OSCSendActiveSensing(void)
{
  OSCMessage oscMsg("/pedalino/midi/activesensing/");
  oscUDP.beginPacket(oscRemoteIp, oscRemotePort);
  oscMsg.send(oscUDP).empty();
  oscUDP.endPacket();
}

void OSCSendSystemReset(void)
{
  OSCMessage oscMsg("/pedalino/midi/reset/");
  oscUDP.beginPacket(oscRemoteIp, oscRemotePort);
  oscMsg.send(oscUDP).empty();
  oscUDP.endPacket();
}


// Forward messages received from serial MIDI interface to WiFI interface

void OnSerialMidiNoteOn(byte channel, byte note, byte velocity)
{
  BLEmidiSend();
  AppleMIDI.noteOn(note, velocity, channel);
  OSCSendNoteOn(note, velocity, channel);
}

void OnSerialMidiNoteOff(byte channel, byte note, byte velocity)
{
  BLEmidiSend();
  AppleMIDI.noteOff(note, velocity, channel);
  OSCSendNoteOff(note, velocity, channel);
}

void OnSerialMidiAfterTouchPoly(byte channel, byte note, byte pressure)
{
  BLEmidiSend();
  AppleMIDI.polyPressure(note, pressure, channel);
  OSCSendAfterTouchPoly(note, pressure, channel);
}

void OnSerialMidiControlChange(byte channel, byte number, byte value)
{
  BLEmidiSend();
  AppleMIDI.controlChange(number, value, channel);
  OSCSendControlChange(number, value, channel);
}

void OnSerialMidiProgramChange(byte channel, byte number)
{
  BLEmidiSend();
  AppleMIDI.programChange(number, channel);
  OSCSendProgramChange(number, channel);
}

void OnSerialMidiAfterTouchChannel(byte channel, byte pressure)
{
  BLEmidiSend();
  AppleMIDI.afterTouch(pressure, channel);
  OSCSendAfterTouch(pressure, channel);
}

void OnSerialMidiPitchBend(byte channel, int bend)
{
  BLEmidiSend();
  AppleMIDI.pitchBend(bend, channel);
  OSCSendPitchBend(bend, channel);
}

void OnSerialMidiSystemExclusive(byte* array, unsigned size)
{
  BLEmidiSend();
  AppleMIDI.sysEx(array, size);
  OSCSendSystemExclusive(array, size);
}

void OnSerialMidiTimeCodeQuarterFrame(byte data)
{
  BLEmidiSend();
  AppleMIDI.timeCodeQuarterFrame(data);
  OSCSendTimeCodeQuarterFrame(data);
}

void OnSerialMidiSongPosition(unsigned int beats)
{
  BLEmidiSend();
  AppleMIDI.songPosition(beats);
  OSCSendSongPosition(beats);
}

void OnSerialMidiSongSelect(byte songnumber)
{
  BLEmidiSend();
  AppleMIDI.songSelect(songnumber);
  OSCSendSongSelect(songnumber);
}

void OnSerialMidiTuneRequest(void)
{
  BLEmidiSend();
  AppleMIDI.tuneRequest();
  OSCSendTuneRequest();
}

void OnSerialMidiClock(void)
{
  BLEmidiSend();
  AppleMIDI.clock();
  OSCSendClock();
}

void OnSerialMidiStart(void)
{
  BLEmidiSend();
  AppleMIDI.start();
  OSCSendStart();
}

void OnSerialMidiContinue(void)
{
  BLEmidiSend();
  AppleMIDI._continue();
  OSCSendContinue();
}

void OnSerialMidiStop(void)
{
  BLEmidiSend();
  AppleMIDI.stop();
  OSCSendStop();
}

void OnSerialMidiActiveSensing(void)
{
  BLEmidiSend();
  AppleMIDI.activeSensing();
  OSCSendActiveSensing();
}

void OnSerialMidiSystemReset(void)
{
  BLEmidiSend();
  AppleMIDI.reset();
  OSCSendSystemReset();
}


// Forward messages received from WiFI MIDI interface to serial MIDI interface

void OnAppleMidiConnected(uint32_t ssrc, char* name)
{
  appleMidiConnected  = true;
#ifdef PEDALINO_TELNET_DEBUG
  DEBUG("AppleMIDI Connected Session %d %s\n", ssrc, name);
#endif
}

void OnAppleMidiDisconnected(uint32_t ssrc)
{
  appleMidiConnected  = false;
#ifdef PEDALINO_TELNET_DEBUG
  DEBUG("AppleMIDI Disonnected Session ID %d\n", ssrc);
#endif
}

void OnAppleMidiNoteOn(byte channel, byte note, byte velocity)
{
  MIDI.sendNoteOn(note, velocity, channel);
  BLESendNoteOn(note, velocity, channel);
  OSCSendNoteOn(note, velocity, channel);
}

void OnAppleMidiNoteOff(byte channel, byte note, byte velocity)
{
  MIDI.sendNoteOff(note, velocity, channel);
  BLESendNoteOff(note, velocity, channel);
  OSCSendNoteOff(note, velocity, channel);
}

void OnAppleMidiReceiveAfterTouchPoly(byte channel, byte note, byte pressure)
{
  MIDI.sendPolyPressure(note, pressure, channel);
  BLESendAfterTouchPoly(note, pressure, channel);
  OSCSendAfterTouchPoly(note, pressure, channel);
}

void OnAppleMidiReceiveControlChange(byte channel, byte number, byte value)
{
  MIDI.sendControlChange(number, value, channel);
  BLESendControlChange(number, value, channel);
  OSCSendControlChange(number, value, channel);
}

void OnAppleMidiReceiveProgramChange(byte channel, byte number)
{
  MIDI.sendProgramChange(number, channel);
  BLESendProgramChange(number, channel);
  OSCSendProgramChange(number, channel);
}

void OnAppleMidiReceiveAfterTouchChannel(byte channel, byte pressure)
{
  MIDI.sendAfterTouch(pressure, channel);
  BLESendAfterTouch(pressure, channel);
  OSCSendAfterTouch(pressure, channel);
}

void OnAppleMidiReceivePitchBend(byte channel, int bend)
{
  MIDI.sendPitchBend(bend, channel);
  BLESendPitchBend(bend, channel);
  OSCSendPitchBend(bend, channel);
}

void OnAppleMidiReceiveSysEx(const byte * data, uint16_t size)
{
  MIDI.sendSysEx(size, data);
  BLESendSystemExclusive(data, size);
  OSCSendSystemExclusive(data, size);
}

void OnAppleMidiReceiveTimeCodeQuarterFrame(byte data)
{
  MIDI.sendTimeCodeQuarterFrame(data);
  BLESendTimeCodeQuarterFrame(data);
  OSCSendTimeCodeQuarterFrame(data);
}

void OnAppleMidiReceiveSongPosition(unsigned short beats)
{
  MIDI.sendSongPosition(beats);
  BLESendSongPosition(beats);
  OSCSendSongPosition(beats);
}

void OnAppleMidiReceiveSongSelect(byte songnumber)
{
  MIDI.sendSongSelect(songnumber);
  BLESendSongSelect(songnumber);
  OSCSendSongSelect(songnumber);
}

void OnAppleMidiReceiveTuneRequest(void)
{
  MIDI.sendTuneRequest();
  BLESendTuneRequest();
  OSCSendTuneRequest();
}

void OnAppleMidiReceiveClock(void)
{
  MIDI.sendRealTime(midi::Clock);
  BLESendClock();
  OSCSendClock();
}

void OnAppleMidiReceiveStart(void)
{
  MIDI.sendRealTime(midi::Start);
  BLESendStart();
  OSCSendStart();
}

void OnAppleMidiReceiveContinue(void)
{
  MIDI.sendRealTime(midi::Continue);
  BLESendContinue();
  OSCSendContinue();
}

void OnAppleMidiReceiveStop(void)
{
  MIDI.sendRealTime(midi::Stop);
  BLESendStop();
  OSCSendStop();
}

void OnAppleMidiReceiveActiveSensing(void)
{
  MIDI.sendRealTime(midi::ActiveSensing);
  BLESendActiveSensing();
  OSCSendActiveSensing();
}

void OnAppleMidiReceiveReset(void)
{
  MIDI.sendRealTime(midi::SystemReset);
  BLESendSystemReset();
  OSCSendSystemReset();
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
  boolean result = WiFi.softAP("Pedalino");
  DPRINTLN("AP mode started");
  DPRINTLN("Connect to 'Pedalino' wireless with no password");
}

void ap_mode_stop()
{
  if (WiFi.getMode() == WIFI_AP) {
    WiFi.softAPdisconnect();
    WIFI_LED_OFF();
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
    DPRINT(".");
  }

  if (WiFi.smartConfigDone())
  {
    DPRINT("[SUCCESS]\n");
    DPRINT("SSID        : %s\n", WiFi.SSID().c_str());
    DPRINT("Password    : %s\n", WiFi.psk().c_str());
  }
  else
    DPRINTLN("[TIMEOUT]");

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

bool auto_reconnect()
{
  // Return 'true' if connected to the (last used) access point within WIFI_CONNECT_TIMEOUT seconds

  DPRINT("Connecting to last used AP\n");
  DPRINT("SSID        : %s\n", WiFi.SSID().c_str());
  DPRINT("Password    : %s\n", WiFi.psk().c_str());

  WiFi.mode(WIFI_STA);
  for (byte i = 0; i < WIFI_CONNECT_TIMEOUT * 2 && WiFi.status() != WL_CONNECTED; i++) {
    status_blink();
    delay(100);
    status_blink();
    delay(300);
    DPRINT(".");
  }

  WiFi.status() == WL_CONNECTED ? WIFI_LED_ON() : WIFI_LED_OFF();

  if (WiFi.status() == WL_CONNECTED)
    DPRINTLN("[SUCCESS]");
  else
    DPRINTLN("[TIMEOUT]");

  return WiFi.status() == WL_CONNECTED;
}

void wifi_connect()
{
  if (!auto_reconnect())       // WIFI_CONNECT_TIMEOUT seconds to reconnect to last used access point
    if (smart_config())        // SMART_CONFIG_TIMEOUT seconds to receive SmartConfig parameters
      auto_reconnect();        // WIFI_CONNECT_TIMEOUT seconds to connect to SmartConfig access point
  if (WiFi.status() != WL_CONNECTED) {
    ap_mode_start();          // switch to AP mode until next reboot
  }
  else
  {
    // connected to an AP

#ifdef ARDUINO_ARCH_ESP8266
    WiFi.hostname(host);
#endif
#ifdef ARDUINO_ARCH_ESP32
    WiFi.setHostname(host);
#endif

#if PEDALINO_DEBUG_SERIAL
    DPRINTLN("");
    WiFi.printDiag(SERIALDEBUG);
    DPRINTLN("");
#endif

    uint8_t macAddr[6];
    WiFi.macAddress(macAddr);
    DPRINT("BSSID       : %s\n", WiFi.BSSIDstr().c_str());
    DPRINT("RSSI        : %d dBm\n", WiFi.RSSI());
#ifdef ARDUINO_ARCH_ESP8266
    DPRINT("Hostname    : %s\n", WiFi.hostname().c_str());
#endif
#ifdef ARDUINO_ARCH_ESP32
    DPRINT("Hostname    : %s\n", WiFi.getHostname());
#endif
    DPRINT("STA         : %02X:%02X:%02X:%02X:%02X:%02X\n", macAddr[0], macAddr[1], macAddr[2], macAddr[3], macAddr[4], macAddr[5]);
    DPRINT("IP address  : %s\n", WiFi.localIP().toString().c_str());
    DPRINT("Subnet mask : %s\n", WiFi.subnetMask().toString().c_str());
    DPRINT("Gataway IP  : %s\n", WiFi.gatewayIP().toString().c_str());
    DPRINT("DNS 1       : %s\n", WiFi.dnsIP(0).toString().c_str());
    DPRINT("DNS 2       : %s\n", WiFi.dnsIP(1).toString().c_str());
    DPRINTLN("");
  }

#ifdef ARDUINO_ARCH_ESP8266
  // Start LLMNR (Link-Local Multicast Name Resolution) responder
  LLMNR.begin(host);
  DPRINTLN("LLMNR responder started");
#endif

  // Start mDNS (Multicast DNS) responder (ping pedalino.local)
  if (MDNS.begin(host)) {
    DPRINTLN("mDNS responder started");
    MDNS.addService("apple-midi", "udp", 5004);
    MDNS.addService("osc",        "udp", oscLocalPort);
  }

  // Start firmawre update via HTTP (connect to http://pedalino.local/update)
#ifdef ARDUINO_ARCH_ESP8266
  httpUpdater.setup(&httpServer);
#endif
  httpServer.begin();
  MDNS.addService("http", "tcp", 80);
#ifdef PEDALINO_TELNET_DEBUG
  MDNS.addService("telnet", "tcp", 23);
#endif
  DPRINTLN("HTTP server started");
  DPRINTLN("Connect to http://pedalino.local/update for firmware update");

  // Calculate the broadcast address of local WiFi to broadcast OSC messages
  oscRemoteIp = WiFi.localIP();
  IPAddress localMask = WiFi.subnetMask();
  for (int i = 0; i < 4; i++)
    oscRemoteIp[i] |= (localMask[i] ^ B11111111);

  // Set incoming OSC messages port
  oscUDP.begin(oscLocalPort);
  DPRINTLN("OSC server started");
#ifdef ARDUINO_ARCH_ESP8266
  DPRINT("Local port: ");
  DPRINTLN(oscUDP.localPort());
#endif
}


void midi_connect()
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

void setup()
{
#ifdef PEDALINO_SERIAL_DEBUG
  SERIALDEBUG.begin(115200);
  SERIALDEBUG.setDebugOutput(true);
#endif

  DPRINTLN("");
  DPRINTLN("****************************");
  DPRINTLN("***     Pedalino(TM)     ***");
  DPRINTLN("****************************");

  pinMode(WIFI_LED, OUTPUT);

#ifdef ARDUINO_ARCH_ESP32
  pinMode(BLE_LED, OUTPUT);
  SerialMIDI.begin(SERIALMIDI_BAUD_RATE, SERIAL_8N1, SERIALMIDI_RX, SERIALMIDI_TX);
#endif

  BLEmidiStart();

  // Write SSID/password to flash only if currently used values do not match what is already stored in flash
  WiFi.persistent(false);
  wifi_connect();

#ifdef PEDALINO_SERIAL_DEBUG
  SERIALDEBUG.flush();
#endif

#ifdef PEDALINO_TELNET_DEBUG
  // Initialize the telnet server of RemoteDebug
  Debug.begin(host);              // Initiaze the telnet server
  Debug.setResetCmdEnabled(true); // Enable the reset command
#endif

  midi_connect();
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

  // Listen to incoming messages from Arduino

#ifdef PEDALINO_TELNET_DEBUG
  if (MIDI.read()) {
    DEBUG("Received MIDI message:   Type 0x%02x   Data1 0x%02x   Data2 0x%02x   Channel 0x%02x\n", MIDI.getType(), MIDI.getData1(), MIDI.getData2(), MIDI.getChannel());
  }
#else
  MIDI.read();
#endif

  // Listen to incoming AppleMIDI messages from WiFi
  AppleMIDI.run();

  // Listen to incoming OSC messages from WiFi
  int size = oscUDP.parsePacket();

  if (size > 0) {
    while (size--) oscMsg.fill(oscUDP.read());
    if (!oscMsg.hasError()) {
      oscMsg.dispatch("/pedalino/midi/noteOn",        OnOscNoteOn);
      oscMsg.dispatch("/pedalino/midi/noteOff",       OnOscNoteOff);
      oscMsg.dispatch("/pedalino/midi/controlChange", OnOscControlChange);
    } else {
      oscError = oscMsg.getError();
#ifdef PEDALINO_TELNET_DEBUG
      DEBUG("OSC error: ");
      //DEBUG(oscError);
      DEBUG("\n");
#endif
    }
  }

  // Run HTTP Updater
  httpServer.handleClient();

#ifdef PEDALINO_TELNET_DEBUG
  // Remote debug over telnet
  Debug.handle();
#endif
}


