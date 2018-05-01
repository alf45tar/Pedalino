// ESP8266 MIDI Gateway between Serial MIDI <-> WiFi AppleMIDI

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <WiFiUdp.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ESP8266LLMNR.h>
#include <ESP8266HTTPUpdateServer.h>
#include <MIDI.h>
#include <AppleMidi.h>
#include <OSCMessage.h>
#include <OSCBundle.h>
#include <OSCData.h>

#define PEDALINO_SERIAL_DEBUG
#define PEDALINO_TELNET_DEBUG

#ifdef PEDALINO_TELNET_DEBUG
#include "RemoteDebug.h"          // Remote debug over telnet - not recommended for production, only for development    
RemoteDebug Debug;
#endif

#define WIFI_CONNECT_TIMEOUT    10
#define SMART_CONFIG_TIMEOUT    30

#ifdef PEDALINO_SERIAL_DEBUG
#define BUILTIN_LED       0  // onboard LED on GPIO2 is shared with Serial1 TX
#else
#define BUILTIN_LED       2  // onboard LED, used as status indicator
#endif
#define BUILTIN_LED_OFF() digitalWrite(BUILTIN_LED, HIGH)
#define BUILTIN_LED_ON()  digitalWrite(BUILTIN_LED, LOW)

const char host[]     = "pedalino";

WiFiEventHandler        stationModeConnectedHandler;
WiFiEventHandler        stationModeDisconnectedHandler;
WiFiEventHandler        stationConnectedHandler;
WiFiEventHandler        stationDisconnectedHandler;

ESP8266WebServer        httpServer(80);
ESP8266HTTPUpdateServer httpUpdater;

// WiFi MIDI interface to comunicate with AppleMIDI/RTP-MDI devices

APPLEMIDI_CREATE_INSTANCE(WiFiUDP, AppleMIDI); // see definition in AppleMidi_Defs.h

bool          appleMidiConnected = false;
unsigned long lastOn             = 0;

// Serial MIDI interface to comunicate with Arduino

struct SerialMIDISettings : public midi::DefaultSettings
{
  static const long BaudRate = 115200;
};

MIDI_CREATE_CUSTOM_INSTANCE(HardwareSerial, Serial, MIDI, SerialMIDISettings);
//MIDI_CREATE_INSTANCE(HardwareSerial, Serial, MIDI);

WiFiUDP                 oscUDP;                  // A UDP instance to let us send and receive packets over UDP
IPAddress               oscRemoteIp;             // remote IP of an external OSC device
const unsigned int      oscRemotePort = 9000;    // remote port of an external OSC device
const unsigned int      oscLocalPort = 8000;     // local port to listen for OSC packets (actually not used for sending)
OSCMessage              oscMsg;
OSCErrorCode            oscError;

// Forward messages received from serial MIDI interface to WiFI MIDI interface

void handleNoteOn(byte channel, byte note, byte velocity)
{
  AppleMIDI.noteOn(note, velocity, channel);

  OSCMessage msg("/noteOn");
  oscUDP.beginPacket(oscRemoteIp, oscRemotePort);
  msg.add((int32_t)channel).add((int32_t)note).add((int32_t)velocity).send(oscUDP).empty();
  oscUDP.endPacket();
}

void handleNoteOff(byte channel, byte note, byte velocity)
{
  AppleMIDI.noteOff(note, velocity, channel);

  OSCMessage msg("/noteOff");
  oscUDP.beginPacket(oscRemoteIp, oscRemotePort);
  msg.add((int32_t)channel).add((int32_t)note).add((int32_t)velocity).send(oscUDP).empty();
  oscUDP.endPacket();
}

void handleAfterTouchPoly(byte channel, byte note, byte pressure)
{
  AppleMIDI.polyPressure(note, pressure, channel);
}

void handleControlChange(byte channel, byte number, byte value)
{
  AppleMIDI.controlChange(number, value, channel);

  OSCMessage msg("/controlChange");
  oscUDP.beginPacket(oscRemoteIp, oscRemotePort);
  msg.add((int32_t)channel).add((int32_t)number).add((int32_t)value).send(oscUDP).empty();
  oscUDP.endPacket();
}

void handleProgramChange(byte channel, byte number)
{
  AppleMIDI.programChange(number, channel);
}

void handleAfterTouchChannel(byte channel, byte pressure)
{
  AppleMIDI.afterTouch(pressure, channel);
}

void handlePitchBend(byte channel, int bend)
{
  AppleMIDI.pitchBend(bend, channel);
}

void handleSystemExclusive(byte* array, unsigned size)
{
  AppleMIDI.sysEx(array, size);
  /*
    // SysEx starts with 0xF0 and ends with 0xF7
    if (array[2] == 'S') {
    // Reset currently used SSID / password stored in flash
    WiFi.persistent(true);
    switch (WiFi.getMode()) {
      case WIFI_STA:
        WiFi.disconnect();
        break;
      case WIFI_AP:
        WiFi.softAPdisconnect();
        break;
    }
    //ESP.restart();
    WiFi.persistent(false);
    wifi_connect();
    }
    if (array[2] == 'A') {
    ap_mode_start();
    }
  */
}

void handleTimeCodeQuarterFrame(byte data)
{
  AppleMIDI.timeCodeQuarterFrame(data);
}

void handleSongPosition(unsigned int beats)
{
  AppleMIDI.songPosition(beats);
}

void handleSongSelect(byte songnumber)
{
  AppleMIDI.songSelect(songnumber);
}

void handleTuneRequest(void)
{
  AppleMIDI.tuneRequest();
}

void handleClock(void)
{
  AppleMIDI.clock();
}

void handleStart(void)
{
  AppleMIDI.start();
}

void handleContinue(void)
{
  AppleMIDI._continue();
}

void handleStop(void)
{
  AppleMIDI.stop();
}

void handleActiveSensing(void)
{
  AppleMIDI.activeSensing();
}

void handleSystemReset(void)
{
  AppleMIDI.reset();
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
}

void OnAppleMidiNoteOff(byte channel, byte note, byte velocity)
{
  MIDI.sendNoteOff(note, velocity, channel);
}

void OnAppleMidiReceiveAfterTouchPoly(byte channel, byte note, byte pressure)
{
  MIDI.sendPolyPressure(note, pressure, channel);
}

void OnAppleMidiReceiveControlChange(byte channel, byte number, byte value)
{
  MIDI.sendControlChange(number, value, channel);
}

void OnAppleMidiReceiveProgramChange(byte channel, byte number)
{
  MIDI.sendProgramChange(number, channel);
}
void OnAppleMidiReceiveAfterTouchChannel(byte channel, byte pressure)
{
  MIDI.sendAfterTouch(pressure, channel);
}

void OnAppleMidiReceivePitchBend(byte channel, int bend)
{
  MIDI.sendPitchBend(bend, channel);
}

void OnAppleMidiReceiveSysEx(const byte * data, uint16_t size)
{
  MIDI.sendSysEx(size, data);
}

void OnAppleMidiReceiveTimeCodeQuarterFrame(byte data)
{
  MIDI.sendTimeCodeQuarterFrame(data);
}

void OnAppleMidiReceiveSongPosition(unsigned short beats)
{
  MIDI.sendSongPosition(beats);
}

void OnAppleMidiReceiveSongSelect(byte songnumber)
{
  MIDI.sendSongSelect(songnumber);
}

void OnAppleMidiReceiveTuneRequest(void)
{
  MIDI.sendTuneRequest();
}

void OnAppleMidiReceiveClock(void)
{
  MIDI.sendRealTime(midi::Clock);
}

void OnAppleMidiReceiveStart(void)
{
  MIDI.sendRealTime(midi::Start);
}

void OnAppleMidiReceiveContinue(void)
{
  MIDI.sendRealTime(midi::Continue);
}

void OnAppleMidiReceiveStop(void)
{
  MIDI.sendRealTime(midi::Stop);
}

void OnAppleMidiReceiveActiveSensing(void)
{
  MIDI.sendRealTime(midi::ActiveSensing);
}

void OnAppleMidiReceiveReset(void)
{
  MIDI.sendRealTime(midi::SystemReset);
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



void onConnected(const WiFiEventStationModeConnected& evt)
{
  BUILTIN_LED_ON();
}

void onDisconnected(const WiFiEventStationModeDisconnected& evt)
{
  BUILTIN_LED_OFF();
}

void onStationConnected(const WiFiEventSoftAPModeStationConnected& evt)
{
  WiFi.softAPgetStationNum() > 0 ? BUILTIN_LED_ON() : BUILTIN_LED_OFF();
}

void onStationDisconnected(const WiFiEventSoftAPModeStationDisconnected& evt)
{
  WiFi.softAPgetStationNum() > 0 ? BUILTIN_LED_ON() : BUILTIN_LED_OFF();
}

void ap_mode_start()
{
  BUILTIN_LED_OFF();

  WiFi.mode(WIFI_AP);
  boolean result = WiFi.softAP("Pedalino");

  // Call "onStationConnected" each time a station connects
  //stationConnectedHandler = WiFi.onSoftAPModeStationConnected(&onStationConnected);

  // Call "onStationDisconnected" each time a station disconnects
  //stationDisconnectedHandler = WiFi.onSoftAPModeStationDisconnected(&onStationDisconnected);
}

void ap_mode_stop()
{
  if (WiFi.getMode() == WIFI_AP) {
    stationConnectedHandler    = WiFiEventHandler();
    stationDisconnectedHandler = WiFiEventHandler();
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
  Serial1.println("");
  Serial1.print("SmartConfig started");
#endif

  for (int i = 0; i < SMART_CONFIG_TIMEOUT && !WiFi.smartConfigDone(); i++) {
    status_blink();
    delay(950);
#ifdef PEDALINO_SERIAL_DEBUG
    Serial1.print(".");
#endif
  }

#ifdef PEDALINO_SERIAL_DEBUG
  if (WiFi.smartConfigDone())
  {
    Serial1.println("[SUCCESS]");
    Serial1.printf("SSID        : %s\n", WiFi.SSID().c_str());
    Serial1.printf("Password    : %s\n", WiFi.psk().c_str());
  }
  else
    Serial1.println("[NO SUCCESS]");
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
  Serial1.println("");
  Serial1.printf("Connecting to %s ", WiFi.SSID().c_str());
#endif

  WiFi.mode(WIFI_STA);

  // Call "onConnected" when connected to an AP
  //stationModeConnectedHandler = WiFi.onStationModeConnected(&onConnected);

  // Call "onDisconnected" when disconnected from an AP
  //stationModeDisconnectedHandler = WiFi.onStationModeDisconnected(&onDisconnected);

  for (byte i = 0; i < WIFI_CONNECT_TIMEOUT * 2 && WiFi.status() != WL_CONNECTED; i++) {
    status_blink();
    delay(100);
    status_blink();
    delay(300);
#ifdef PEDALINO_SERIAL_DEBUG
    Serial1.print(".");
#endif
  }

  WiFi.status() == WL_CONNECTED ? BUILTIN_LED_ON() : BUILTIN_LED_OFF();

#ifdef PEDALINO_SERIAL_DEBUG
  if (WiFi.status() == WL_CONNECTED)
    Serial1.println("[SUCCESS]");
  else
    Serial1.println("[NO SUCCESS]");
#endif

  return WiFi.status() == WL_CONNECTED;
}

void wifi_connect()
{
  if (!auto_reconnect())       // WIFI_CONNECT_TIMEOUT seconds to reconnect to last used access point
    if (smart_config())        // SMART_CONFIG_TIMEOUT seconds to receive SmartConfig parameters
      auto_reconnect();        // WIFI_CONNECT_TIMEOUT seconds to connect to SmartConfig access point
  if (WiFi.status() != WL_CONNECTED) {
    //stationModeConnectedHandler    = WiFiEventHandler();  // disconnect station mode handler
    //stationModeDisconnectedHandler = WiFiEventHandler();  // disconnect station mode handler
    ap_mode_start();          // switch to AP mode until next reboot
  }
  else
  {
    // connected to an AP

    WiFi.hostname(host);

#ifdef PEDALINO_SERIAL_DEBUG
    Serial1.println("");
    WiFi.printDiag(Serial1);
    Serial1.println("");

    uint8_t macAddr[6];
    WiFi.macAddress(macAddr);
    Serial1.printf("BSSID       : %s\n", WiFi.BSSIDstr().c_str());
    Serial1.printf("RSSI        : %d dBm\n", WiFi.RSSI());
    Serial1.printf("Hostname    : %s\n", WiFi.hostname().c_str());
    Serial1.printf("STA         : %02X:%02X:%02X:%02X:%02X:%02X\n", macAddr[0], macAddr[1], macAddr[2], macAddr[3], macAddr[4], macAddr[5]);
    Serial1.printf("IP address  : %s\n", WiFi.localIP().toString().c_str());
    Serial1.printf("Subnet mask : %s\n", WiFi.subnetMask().toString().c_str());
    Serial1.printf("Gataway IP  : %s\n", WiFi.gatewayIP().toString().c_str());
    Serial1.printf("DNS 1       : %s\n", WiFi.dnsIP(0).toString().c_str());
    Serial1.printf("DNS 2       : %s\n", WiFi.dnsIP(1).toString().c_str());
    Serial1.println("");
#endif
  }

  // Start LLMNR (Link-Local Multicast Name Resolution) responder
  LLMNR.begin(host);
#ifdef PEDALINO_SERIAL_DEBUG
  Serial1.println("LLMNR responder started");
#endif

  // Start mDNS (Multicast DNS) responder
  if (MDNS.begin(host, WiFi.localIP())) {
#ifdef PEDALINO_SERIAL_DEBUG
    Serial1.println("mDNS responder started");
#endif
    MDNS.addService("apple-midi", "udp", 5004);
    MDNS.addService("osc",        "udp", 8000);
  }

  // Start firmawre update via HTTP (connect to http://pedalino/update)
  httpUpdater.setup(&httpServer);
  httpServer.begin();
  MDNS.addService("http", "tcp", 80);
#ifdef PEDALINO_TELNET_DEBUG
  MDNS.addService("telnet", "tcp", 23);
#endif
#ifdef PEDALINO_SERIAL_DEBUG
  Serial1.println("HTTP server started");
  Serial1.println("Connect to http://pedalino/update for firmware update");
#endif

  // Broadcast outcoming OSC messages to local WiFi network
  oscRemoteIp = WiFi.localIP();
  oscRemoteIp[3] = 255;

  // Start listen incoming OSC messages
  oscUDP.begin(oscLocalPort);
#ifdef PEDALINO_SERIAL_DEBUG
  Serial1.println("OSC server started");
  Serial1.print("Local port: ");
  Serial1.println(oscUDP.localPort());
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
  Serial1.begin(115200);
  Serial1.setDebugOutput(true);
  Serial1.println("");
  Serial1.println("");
  Serial1.println("*** Pedalino(TM) ***");
#endif

  // Write SSID/password to flash only if currently used values do not match what is already stored in flash
  WiFi.persistent(false);
  wifi_connect();

#ifdef PEDALINO_SERIAL_DEBUG
  Serial1.flush();
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
      oscMsg.dispatch("/noteOn",        OnOscNoteOn);
      oscMsg.dispatch("/noteOff",       OnOscNoteOff);
      oscMsg.dispatch("/controlChange", OnOscControlChange);
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


