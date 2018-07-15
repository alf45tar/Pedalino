// ESP8266/ESP32 MIDI Gateway between Serial MIDI <-> WiFi AppleMIDI <-> WiFi OSC <-> Bluetooth LE MIDI

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
#endif

#include <WiFiClient.h>
#include <WiFiUdp.h>

#include <MIDI.h>
#include <AppleMidi.h>
#include <OSCMessage.h>
#include <OSCBundle.h>
#include <OSCData.h>

#define PEDALINO_SERIAL_DEBUG
//#define PEDALINO_TELNET_DEBUG

#ifdef PEDALINO_TELNET_DEBUG
#include "RemoteDebug.h"          // Remote debug over telnet - not recommended for production, only for development    
RemoteDebug Debug;
#endif

#define WIFI_CONNECT_TIMEOUT    10
#define SMART_CONFIG_TIMEOUT    30

#define BUILTIN_LED       LED_BUILTIN  // onboard LED, used as status indicator

#if defined(ARDUINO_ARCH_ESP8266) && defined(PEDALINO_SERIAL_DEBUG)
#define SERIALDEBUG       Serial1
#define BUILTIN_LED       0  // ESP8266 only: onboard LED on GPIO2 is shared with Serial1 TX
#endif

#if defined(ARDUINO_ARCH_ESP32) && defined(PEDALINO_SERIAL_DEBUG)
#define SERIALDEBUG       Serial
#endif

#define BUILTIN_LED_OFF() digitalWrite(BUILTIN_LED, HIGH)
#define BUILTIN_LED_ON()  digitalWrite(BUILTIN_LED, LOW)

const char host[]     = "pedalino";

#ifdef ARDUINO_ARCH_ESP8266
ESP8266WebServer        httpServer(80);
ESP8266HTTPUpdateServer httpUpdater;
#endif

#ifdef ARDUINO_ARCH_ESP32
WebServer               httpServer(80);
HTTPUpload              httpUpdater;
#endif

// WiFi MIDI interface to comunicate with AppleMIDI/RTP-MDI devices

APPLEMIDI_CREATE_INSTANCE(WiFiUDP, AppleMIDI); // see definition in AppleMidi_Defs.h

bool          appleMidiConnected = false;
unsigned long lastOn             = 0;

// Serial MIDI interface to comunicate with Arduino

struct SerialMIDISettings : public midi::DefaultSettings
{
  static const long BaudRate = 115200;
};

#ifdef ARDUINO_ARCH_ESP8266
MIDI_CREATE_CUSTOM_INSTANCE(HardwareSerial, Serial, MIDI, SerialMIDISettings);
#endif

#ifdef ARDUINO_ARCH_ESP32
MIDI_CREATE_CUSTOM_INSTANCE(HardwareSerial, Serial2, MIDI, SerialMIDISettings);
#endif


// WiFi OSC comunication

WiFiUDP                 oscUDP;                  // A UDP instance to let us send and receive packets over UDP
IPAddress               oscRemoteIp;             // remote IP of an external OSC device or broadcast address
const unsigned int      oscRemotePort = 9000;    // remote port of an external OSC device
const unsigned int      oscLocalPort = 8000;     // local port to listen for OSC packets (actually not used for sending)
OSCMessage              oscMsg;
OSCErrorCode            oscError;



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

void handleNoteOn(byte channel, byte note, byte velocity)
{
  AppleMIDI.noteOn(note, velocity, channel);
  OSCSendNoteOn(note, velocity, channel);
}

void handleNoteOff(byte channel, byte note, byte velocity)
{
  AppleMIDI.noteOff(note, velocity, channel);
  OSCSendNoteOff(note, velocity, channel);
}

void handleAfterTouchPoly(byte channel, byte note, byte pressure)
{
  AppleMIDI.polyPressure(note, pressure, channel);
  OSCSendAfterTouchPoly(note, pressure, channel);
}

void handleControlChange(byte channel, byte number, byte value)
{
  AppleMIDI.controlChange(number, value, channel);
  OSCSendControlChange(number, value, channel);
}

void handleProgramChange(byte channel, byte number)
{
  AppleMIDI.programChange(number, channel);
  OSCSendProgramChange(number, channel);
}

void handleAfterTouchChannel(byte channel, byte pressure)
{
  AppleMIDI.afterTouch(pressure, channel);
  OSCSendAfterTouch(pressure, channel);
}

void handlePitchBend(byte channel, int bend)
{
  AppleMIDI.pitchBend(bend, channel);
  OSCSendPitchBend(bend, channel);
}

void handleSystemExclusive(byte* array, unsigned size)
{
  AppleMIDI.sysEx(array, size);
  OSCSendSystemExclusive(array, size);
}

void handleTimeCodeQuarterFrame(byte data)
{
  AppleMIDI.timeCodeQuarterFrame(data);
  OSCSendTimeCodeQuarterFrame(data);
}

void handleSongPosition(unsigned int beats)
{
  AppleMIDI.songPosition(beats);
  OSCSendSongPosition(beats);
}

void handleSongSelect(byte songnumber)
{
  AppleMIDI.songSelect(songnumber);
  OSCSendSongSelect(songnumber);
}

void handleTuneRequest(void)
{
  AppleMIDI.tuneRequest();
  OSCSendTuneRequest();
}

void handleClock(void)
{
  AppleMIDI.clock();
  OSCSendClock();
}

void handleStart(void)
{
  AppleMIDI.start();
  OSCSendStart();
}

void handleContinue(void)
{
  AppleMIDI._continue();
  OSCSendContinue();
}

void handleStop(void)
{
  AppleMIDI.stop();
  OSCSendStop();
}

void handleActiveSensing(void)
{
  AppleMIDI.activeSensing();
  OSCSendActiveSensing();
}

void handleSystemReset(void)
{
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
  OSCSendNoteOn(note, velocity, channel);
}

void OnAppleMidiNoteOff(byte channel, byte note, byte velocity)
{
  MIDI.sendNoteOff(note, velocity, channel);
  OSCSendNoteOff(note, velocity, channel);
}

void OnAppleMidiReceiveAfterTouchPoly(byte channel, byte note, byte pressure)
{
  MIDI.sendPolyPressure(note, pressure, channel);
  OSCSendAfterTouchPoly(note, pressure, channel);
}

void OnAppleMidiReceiveControlChange(byte channel, byte number, byte value)
{
  MIDI.sendControlChange(number, value, channel);
  OSCSendControlChange(number, value, channel);
}

void OnAppleMidiReceiveProgramChange(byte channel, byte number)
{
  MIDI.sendProgramChange(number, channel);
  OSCSendProgramChange(number, channel);
}

void OnAppleMidiReceiveAfterTouchChannel(byte channel, byte pressure)
{
  MIDI.sendAfterTouch(pressure, channel);
  OSCSendAfterTouch(pressure, channel);
}

void OnAppleMidiReceivePitchBend(byte channel, int bend)
{
  MIDI.sendPitchBend(bend, channel);
  OSCSendPitchBend(bend, channel);
}

void OnAppleMidiReceiveSysEx(const byte * data, uint16_t size)
{
  MIDI.sendSysEx(size, data);
  OSCSendSystemExclusive(data, size);
}

void OnAppleMidiReceiveTimeCodeQuarterFrame(byte data)
{
  MIDI.sendTimeCodeQuarterFrame(data);
  OSCSendTimeCodeQuarterFrame(data);
}

void OnAppleMidiReceiveSongPosition(unsigned short beats)
{
  MIDI.sendSongPosition(beats);
  OSCSendSongPosition(beats);
}

void OnAppleMidiReceiveSongSelect(byte songnumber)
{
  MIDI.sendSongSelect(songnumber);
  OSCSendSongSelect(songnumber);
}

void OnAppleMidiReceiveTuneRequest(void)
{
  MIDI.sendTuneRequest();
  OSCSendTuneRequest();
}

void OnAppleMidiReceiveClock(void)
{
  MIDI.sendRealTime(midi::Clock);
  OSCSendClock();
}

void OnAppleMidiReceiveStart(void)
{
  MIDI.sendRealTime(midi::Start);
  OSCSendStart();
}

void OnAppleMidiReceiveContinue(void)
{
  MIDI.sendRealTime(midi::Continue);
  OSCSendContinue();
}

void OnAppleMidiReceiveStop(void)
{
  MIDI.sendRealTime(midi::Stop);
  OSCSendStop();
}

void OnAppleMidiReceiveActiveSensing(void)
{
  MIDI.sendRealTime(midi::ActiveSensing);
  OSCSendActiveSensing();
}

void OnAppleMidiReceiveReset(void)
{
  MIDI.sendRealTime(midi::SystemReset);
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

void ap_mode_start()
{
  BUILTIN_LED_OFF();

  WiFi.mode(WIFI_AP);
  boolean result = WiFi.softAP("Pedalino");
}

void ap_mode_stop()
{
  if (WiFi.getMode() == WIFI_AP) {
    WiFi.softAPdisconnect();
    BUILTIN_LED_OFF();
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

#ifdef PEDALINO_SERIAL_DEBUG
  SERIALDEBUG.println("");
  SERIALDEBUG.print("SmartConfig started");
#endif

  for (int i = 0; i < SMART_CONFIG_TIMEOUT && !WiFi.smartConfigDone(); i++) {
    status_blink();
    delay(950);
#ifdef PEDALINO_SERIAL_DEBUG
    SERIALDEBUG.print(".");
#endif
  }

#ifdef PEDALINO_SERIAL_DEBUG
  if (WiFi.smartConfigDone())
  {
    SERIALDEBUG.println("[SUCCESS]");
    SERIALDEBUG.printf("SSID        : %s\n", WiFi.SSID().c_str());
    SERIALDEBUG.printf("Password    : %s\n", WiFi.psk().c_str());
  }
  else
    SERIALDEBUG.println("[TIMEOUT]");
#endif

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

#ifdef PEDALINO_SERIAL_DEBUG
  SERIALDEBUG.println("");
  SERIALDEBUG.printf("Last used AP\n");
  SERIALDEBUG.printf("SSID        : %s\n", WiFi.SSID().c_str());
  SERIALDEBUG.printf("Password    : %s\n", WiFi.psk().c_str());
  SERIALDEBUG.printf("Connecting");
#endif

  WiFi.mode(WIFI_STA);
  for (byte i = 0; i < WIFI_CONNECT_TIMEOUT * 2 && WiFi.status() != WL_CONNECTED; i++) {
    status_blink();
    delay(100);
    status_blink();
    delay(300);
#ifdef PEDALINO_SERIAL_DEBUG
    SERIALDEBUG.print(".");
#endif
  }

  WiFi.status() == WL_CONNECTED ? BUILTIN_LED_ON() : BUILTIN_LED_OFF();

#ifdef PEDALINO_SERIAL_DEBUG
  if (WiFi.status() == WL_CONNECTED)
    SERIALDEBUG.println("[SUCCESS]");
  else
    SERIALDEBUG.println("[TIMEOUT]");
#endif

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


#ifdef PEDALINO_SERIAL_DEBUG
    SERIALDEBUG.println("");
    WiFi.printDiag(SERIALDEBUG);
    SERIALDEBUG.println("");

    uint8_t macAddr[6];
    WiFi.macAddress(macAddr);
    SERIALDEBUG.printf("BSSID       : %s\n", WiFi.BSSIDstr().c_str());
    SERIALDEBUG.printf("RSSI        : %d dBm\n", WiFi.RSSI());
#ifdef ARDUINO_ARCH_ESP8266
    SERIALDEBUG.printf("Hostname    : %s\n", WiFi.hostname().c_str());
#endif
#ifdef ARDUINO_ARCH_ESP32
    SERIALDEBUG.printf("Hostname    : %s\n", WiFi.getHostname());
#endif
    SERIALDEBUG.printf("STA         : %02X:%02X:%02X:%02X:%02X:%02X\n", macAddr[0], macAddr[1], macAddr[2], macAddr[3], macAddr[4], macAddr[5]);
    SERIALDEBUG.printf("IP address  : %s\n", WiFi.localIP().toString().c_str());
    SERIALDEBUG.printf("Subnet mask : %s\n", WiFi.subnetMask().toString().c_str());
    SERIALDEBUG.printf("Gataway IP  : %s\n", WiFi.gatewayIP().toString().c_str());
    SERIALDEBUG.printf("DNS 1       : %s\n", WiFi.dnsIP(0).toString().c_str());
    SERIALDEBUG.printf("DNS 2       : %s\n", WiFi.dnsIP(1).toString().c_str());
    SERIALDEBUG.println("");
#endif
  }

#ifdef ARDUINO_ARCH_ESP8266
  // Start LLMNR (Link-Local Multicast Name Resolution) responder
  LLMNR.begin(host);
#ifdef PEDALINO_SERIAL_DEBUG
  SERIALDEBUG.println("LLMNR responder started");
#endif
#endif

  // Start mDNS (Multicast DNS) responder (ping pedalino.local)
  if (MDNS.begin(host)) {
#ifdef PEDALINO_SERIAL_DEBUG
    SERIALDEBUG.println("mDNS responder started");
#endif
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
#ifdef PEDALINO_SERIAL_DEBUG
  SERIALDEBUG.println("HTTP server started");
  SERIALDEBUG.println("Connect to http://pedalino.local/update for firmware update");
#endif

  // Calculate the broadcast address of local WiFi to broadcast OSC messages
  oscRemoteIp = WiFi.localIP();
  IPAddress localMask = WiFi.subnetMask();
  for (int i = 0; i < 4; i++)
    oscRemoteIp[i] |= (localMask[i] ^ B11111111);

  // Set incoming OSC messages port
  oscUDP.begin(oscLocalPort);
#ifdef PEDALINO_SERIAL_DEBUG
  SERIALDEBUG.println("OSC server started");
#ifdef ARDUINO_ARCH_ESP8266
  SERIALDEBUG.print("Local port: ");
  SERIALDEBUG.println(oscUDP.localPort());
#endif
#endif
}

void midi_connect()
{
  // Connect the handle function called upon reception of a MIDI message from serial MIDI interface
  MIDI.setHandleNoteOn(handleNoteOn);
  MIDI.setHandleNoteOff(handleNoteOff);
  MIDI.setHandleAfterTouchPoly(handleAfterTouchPoly);
  MIDI.setHandleControlChange(handleControlChange);
  MIDI.setHandleProgramChange(handleProgramChange);
  MIDI.setHandleAfterTouchChannel(handleAfterTouchChannel);
  MIDI.setHandlePitchBend(handlePitchBend);
  MIDI.setHandleSystemExclusive(handleSystemExclusive);
  MIDI.setHandleTimeCodeQuarterFrame(handleTimeCodeQuarterFrame);
  MIDI.setHandleSongPosition(handleSongPosition);
  MIDI.setHandleSongSelect(handleSongSelect);
  MIDI.setHandleTuneRequest(handleTuneRequest);
  MIDI.setHandleClock(handleClock);
  MIDI.setHandleStart(handleStart);
  MIDI.setHandleContinue(handleContinue);
  MIDI.setHandleStop(handleStop);
  MIDI.setHandleActiveSensing(handleActiveSensing);
  MIDI.setHandleSystemReset(handleSystemReset);

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

void status_blink()
{
  BUILTIN_LED_ON();
  delay(50);
  BUILTIN_LED_OFF();
}

void setup()
{
  pinMode(BUILTIN_LED, OUTPUT);

#ifdef PEDALINO_SERIAL_DEBUG
  SERIALDEBUG.begin(115200);
  SERIALDEBUG.setDebugOutput(true);
  SERIALDEBUG.println("");
  SERIALDEBUG.println("");
  SERIALDEBUG.println("*** Pedalino(TM) ***");
#endif

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
  if (appleMidiConnected) {
    // led fast blinking (5 times per second)
    if (millis() - lastOn > 200) {
      BUILTIN_LED_ON();
      lastOn = millis();
    }
    else if (millis() - lastOn > 100) {
      BUILTIN_LED_OFF();
    }
  }
  else
    // led always on if connected to an AP or one or more client connected the the internal AP
    switch (WiFi.getMode()) {
      case WIFI_STA:
        WiFi.isConnected() ? BUILTIN_LED_ON() : BUILTIN_LED_OFF();
        break;
      case WIFI_AP:
        WiFi.softAPgetStationNum() > 0 ? BUILTIN_LED_ON() : BUILTIN_LED_OFF();
        break;
      default:
        BUILTIN_LED_OFF();
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


