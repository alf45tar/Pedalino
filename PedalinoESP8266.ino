// ESP8266 MIDI Gateway between Serial MIDI <-> WiFi AppleMIDI

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <WiFiUdp.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ESP8266HTTPUpdateServer.h>
#include <MIDI.h>
#include <AppleMidi.h>

#define MAX_STRING  16

const char host[]     = "pedalino";
char ssid[MAX_STRING] = "MyWireless";
char pass[MAX_STRING] = "0000000000";

ESP8266WebServer        httpServer(80);
ESP8266HTTPUpdateServer httpUpdater;


// WiFi MIDI interface to comunicate with AppleMIDI/RTP-MDI devices

APPLEMIDI_CREATE_INSTANCE(WiFiUDP, AppleMIDI); // see definition in AppleMidi_Defs.h


// Serial MIDI interface to comunicate with Arduino

struct SerialMIDISettings : public midi::DefaultSettings
{
  static const long BaudRate = 115200;
};

MIDI_CREATE_CUSTOM_INSTANCE(HardwareSerial, Serial, MIDI, SerialMIDISettings);


// Forward messages received from serial MIDI interface to WiFI MIDI interface

void handleNoteOn(byte channel, byte note, byte velocity)
{
  AppleMIDI.noteOn(note, velocity, channel);
}

void handleNoteOff(byte channel, byte note, byte velocity)
{
  AppleMIDI.noteOff(note, velocity, channel);
}

void handleAfterTouchPoly(byte channel, byte note, byte pressure)
{
  AppleMIDI.polyPressure(note, pressure, channel);
}

void handleControlChange(byte channel, byte number, byte value)
{
  AppleMIDI.controlChange(number, value, channel);
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
  //isConnected  = true;
}

void OnAppleMidiDisconnected(uint32_t ssrc)
{
  //isConnected  = false;
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


void setup()
{
  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }

  MDNS.begin(host);
  httpUpdater.setup(&httpServer);
  httpServer.begin();
  MDNS.addService("http", "tcp", 80);

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


void loop()
{
  // Listen to incoming messages from Arduino
  MIDI.read();

  // Listen to incoming messages from WiFi
  AppleMIDI.run();

  // Run HTTP Updater
  httpServer.handleClient();
}


