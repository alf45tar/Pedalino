/*  __________           .___      .__  .__                   ___ ________________    ___
 *  \______   \ ____   __| _/____  |  | |__| ____   ____     /  / \__    ___/     \   \  \   
 *   |     ___// __ \ / __ |\__  \ |  | |  |/    \ /  _ \   /  /    |    | /  \ /  \   \  \  
 *   |    |   \  ___// /_/ | / __ \|  |_|  |   |  (  <_> ) (  (     |    |/    Y    \   )  )
 *   |____|    \___  >____ |(____  /____/__|___|  /\____/   \  \    |____|\____|__  /  /  /
 *                 \/     \/     \/             \/           \__\                 \/  /__/
 *                                                                (c) 2018 alf45star
 *                                                        https://github.com/alf45tar/Pedalino
 */

//
// Forward messages received from one MIDI interface to the others


void midi_routing()
{
  if (interfaces[PED_USBMIDI].midiIn)
    if (USB_MIDI.read()) {
      DPRINTF(" MIDI IN USB -> STATUS ");
      DPRINT(USB_MIDI.getType());
      DPRINTF(" DATA1 ");
      DPRINT(USB_MIDI.getData1());
      DPRINTF(" DATA2 ");
      DPRINT(USB_MIDI.getData2());
      DPRINTF(" CHANNEL ");
      DPRINTLN(USB_MIDI.getChannel());
    }

  if (interfaces[PED_DINMIDI].midiIn)
    if (DIN_MIDI.read()) {
      DPRINTF(" MIDI IN DIN -> STATUS ");
      DPRINT(DIN_MIDI.getType());
      DPRINTF(" DATA1 ");
      DPRINT(DIN_MIDI.getData1());
      DPRINTF(" DATA2 ");
      DPRINT(DIN_MIDI.getData2());
      DPRINTF(" CHANNEL ");
      DPRINTLN(DIN_MIDI.getChannel());
    }
    
  if (interfaces[PED_RTPMIDI].midiIn)
    if (ESP_MIDI.read()) {
      if (ESP_MIDI.check()) {
        if (ESP_MIDI.isChannelMessage(ESP_MIDI.getType())) {
          DPRINTF(" MIDI IN RTP -> STATUS ");
          DPRINT(ESP_MIDI.getType());
          DPRINTF(" DATA1 ");
          DPRINT(ESP_MIDI.getData1());
          DPRINTF(" DATA2 ");
          DPRINT(ESP_MIDI.getData2());
          DPRINTF(" CHANNEL ");
          DPRINTLN(ESP_MIDI.getChannel());
        }
        //else if (ESP_MIDI.getType() == midi::SystemExclusive)
        //  DPRINTLNF(" MIDI IN RTP -> SYSEXE ");
      }
    }
}

bool EspMidiRouting()
{
  return (interfaces[PED_RTPMIDI].midiRouting || interfaces[PED_IPMIDI].midiRouting || interfaces[PED_BLEMIDI].midiRouting || interfaces[PED_OSC].midiRouting);
}

void OnUsbMidiNoteOn(byte channel, byte note, byte velocity)
{
  if (interfaces[PED_DINMIDI].midiRouting) DIN_MIDI.sendNoteOn(note, velocity, channel);
  if (EspMidiRouting())                    ESP_MIDI.sendNoteOn(note, velocity, channel);
}

void OnUsbMidiNoteOff(byte channel, byte note, byte velocity)
{
  if (interfaces[PED_DINMIDI].midiRouting) DIN_MIDI.sendNoteOff(note, velocity, channel);
  if (EspMidiRouting())                    ESP_MIDI.sendNoteOff(note, velocity, channel);
}

void OnUsbMidiAfterTouchPoly(byte channel, byte note, byte pressure)
{
  if (interfaces[PED_DINMIDI].midiRouting) DIN_MIDI.sendAfterTouch(note, pressure, channel);
  if (EspMidiRouting())                    ESP_MIDI.sendAfterTouch(note, pressure, channel);
}

void OnUsbMidiControlChange(byte channel, byte number, byte value)
{
  if (interfaces[PED_DINMIDI].midiRouting) DIN_MIDI.sendControlChange(number, value, channel);
  if (EspMidiRouting())                    ESP_MIDI.sendControlChange(number, value, channel);
}

void OnUsbMidiProgramChange(byte channel, byte number)
{
  if (interfaces[PED_DINMIDI].midiRouting) DIN_MIDI.sendProgramChange(number, channel);
  if (EspMidiRouting())                    ESP_MIDI.sendProgramChange(number, channel);
}

void OnUsbMidiAfterTouchChannel(byte channel, byte pressure)
{
  if (interfaces[PED_DINMIDI].midiRouting) DIN_MIDI.sendAfterTouch(pressure, channel);
  if (EspMidiRouting())                    ESP_MIDI.sendAfterTouch(pressure, channel);
}

void OnUsbMidiPitchBend(byte channel, int bend)
{
  if (interfaces[PED_DINMIDI].midiRouting) DIN_MIDI.sendPitchBend(bend, channel);
  if (EspMidiRouting())                    ESP_MIDI.sendPitchBend(bend, channel);
}

void OnUsbMidiSystemExclusive(byte * array, unsigned size)
{
  if (interfaces[PED_DINMIDI].midiRouting) DIN_MIDI.sendSysEx(size, array);
  if (EspMidiRouting())                    ESP_MIDI.sendSysEx(size, array);
  MTC.decodeMTCFullFrame(size, array);   
}

void OnUsbMidiTimeCodeQuarterFrame(byte data)
{
  if (interfaces[PED_DINMIDI].midiRouting) DIN_MIDI.sendTimeCodeQuarterFrame(data);
  if (EspMidiRouting())                    ESP_MIDI.sendTimeCodeQuarterFrame(data);
  MTC.decodMTCQuarterFrame(data);
}

void OnUsbMidiSongPosition(unsigned int beats)
{
  if (interfaces[PED_DINMIDI].midiRouting) DIN_MIDI.sendSongPosition(beats);
  if (EspMidiRouting())                    ESP_MIDI.sendSongPosition(beats);
}

void OnUsbMidiSongSelect(byte songnumber)
{
  if (interfaces[PED_DINMIDI].midiRouting) DIN_MIDI.sendSongSelect(songnumber);
  if (EspMidiRouting())                    ESP_MIDI.sendSongSelect(songnumber);
}

void OnUsbMidiTuneRequest(void)
{
  if (interfaces[PED_DINMIDI].midiRouting) DIN_MIDI.sendTuneRequest();
  if (EspMidiRouting())                    ESP_MIDI.sendTuneRequest();
}

void OnUsbMidiClock(void)
{
  if (interfaces[PED_DINMIDI].midiRouting) DIN_MIDI.sendRealTime(midi::Clock);
  if (EspMidiRouting())                    ESP_MIDI.sendRealTime(midi::Clock);
  if (MTC.getMode() == MidiTimeCode::SynchroClockSlave) bpm = MTC.tapTempo();
}

void OnUsbMidiStart(void)
{
  if (interfaces[PED_DINMIDI].midiRouting) DIN_MIDI.sendRealTime(midi::Start);
  if (EspMidiRouting())                    ESP_MIDI.sendRealTime(midi::Start);
  if (MTC.getMode() == MidiTimeCode::SynchroClockSlave) MTC.sendPlay();
}

void OnUsbMidiContinue(void)
{
  if (interfaces[PED_DINMIDI].midiRouting) DIN_MIDI.sendRealTime(midi::Continue);
  if (EspMidiRouting())                    ESP_MIDI.sendRealTime(midi::Continue);
  if (MTC.getMode() == MidiTimeCode::SynchroClockSlave) MTC.sendContinue();
}

void OnUsbMidiStop(void)
{
  if (interfaces[PED_DINMIDI].midiRouting) DIN_MIDI.sendRealTime(midi::Stop);
  if (EspMidiRouting())                    ESP_MIDI.sendRealTime(midi::Stop);
  if (MTC.getMode() == MidiTimeCode::SynchroClockSlave) MTC.sendStop();
}

void OnUsbMidiActiveSensing(void)
{
  if (interfaces[PED_DINMIDI].midiRouting) DIN_MIDI.sendRealTime(midi::ActiveSensing);
  if (EspMidiRouting())                    ESP_MIDI.sendRealTime(midi::ActiveSensing);
}

void OnUsbMidiSystemReset(void)
{
  if (interfaces[PED_DINMIDI].midiRouting) DIN_MIDI.sendRealTime(midi::SystemReset);
  if (EspMidiRouting())                    ESP_MIDI.sendRealTime(midi::SystemReset);
}


// Forward messages received from legacy MIDI interface

void OnDinMidiNoteOn(byte channel, byte note, byte velocity)
{
  if (interfaces[PED_USBMIDI].midiRouting) USB_MIDI.sendNoteOn(note, velocity, channel);
  if (EspMidiRouting())                    ESP_MIDI.sendNoteOn(note, velocity, channel);
}

void OnDinMidiNoteOff(byte channel, byte note, byte velocity)
{
  if (interfaces[PED_USBMIDI].midiRouting) USB_MIDI.sendNoteOff(note, velocity, channel);
  if (EspMidiRouting())                    ESP_MIDI.sendNoteOff(note, velocity, channel);
}

void OnDinMidiAfterTouchPoly(byte channel, byte note, byte pressure)
{
  if (interfaces[PED_USBMIDI].midiRouting) USB_MIDI.sendAfterTouch(note, pressure, channel);
  if (EspMidiRouting())                    ESP_MIDI.sendAfterTouch(note, pressure, channel);
}

void OnDinMidiControlChange(byte channel, byte number, byte value)
{
  if (interfaces[PED_USBMIDI].midiRouting) USB_MIDI.sendControlChange(number, value, channel);
  if (EspMidiRouting())                    ESP_MIDI.sendControlChange(number, value, channel);
}

void OnDinMidiProgramChange(byte channel, byte number)
{
  if (interfaces[PED_USBMIDI].midiRouting) USB_MIDI.sendProgramChange(number, channel);
  if (EspMidiRouting())                    ESP_MIDI.sendProgramChange(number, channel);
}

void OnDinMidiAfterTouchChannel(byte channel, byte pressure)
{
  if (interfaces[PED_USBMIDI].midiRouting) USB_MIDI.sendAfterTouch(pressure, channel);
  if (EspMidiRouting())                    ESP_MIDI.sendAfterTouch(pressure, channel);
}

void OnDinMidiPitchBend(byte channel, int bend)
{
  if (interfaces[PED_USBMIDI].midiRouting) USB_MIDI.sendPitchBend(bend, channel);
  if (EspMidiRouting())                    ESP_MIDI.sendPitchBend(bend, channel);
}

void OnDinMidiSystemExclusive(byte * array, unsigned size)
{
  if (interfaces[PED_USBMIDI].midiRouting) USB_MIDI.sendSysEx(size, array);
  if (EspMidiRouting())                    ESP_MIDI.sendSysEx(size, array);
  MTC.decodeMTCFullFrame(size, array);
}

void OnDinMidiTimeCodeQuarterFrame(byte data)
{
  if (interfaces[PED_USBMIDI].midiRouting) USB_MIDI.sendTimeCodeQuarterFrame(data);
  if (EspMidiRouting())                    ESP_MIDI.sendTimeCodeQuarterFrame(data);
  MTC.decodMTCQuarterFrame(data);
}

void OnDinMidiSongPosition(unsigned int beats)
{
  if (interfaces[PED_USBMIDI].midiRouting) USB_MIDI.sendSongPosition(beats);
  if (EspMidiRouting())                    ESP_MIDI.sendSongPosition(beats);
}

void OnDinMidiSongSelect(byte songnumber)
{
  if (interfaces[PED_USBMIDI].midiRouting) USB_MIDI.sendSongSelect(songnumber);
  if (EspMidiRouting())                    ESP_MIDI.sendSongSelect(songnumber);
}

void OnDinMidiTuneRequest(void)
{
  if (interfaces[PED_USBMIDI].midiRouting) USB_MIDI.sendTuneRequest();
  if (EspMidiRouting())                    ESP_MIDI.sendTuneRequest();
}

void OnDinMidiClock(void)
{
  if (interfaces[PED_USBMIDI].midiRouting) USB_MIDI.sendRealTime(midi::Clock);
  if (EspMidiRouting())                    ESP_MIDI.sendRealTime(midi::Clock);
  if (MTC.getMode() == MidiTimeCode::SynchroClockSlave) bpm = MTC.tapTempo();
}

void OnDinMidiStart(void)
{
  if (interfaces[PED_USBMIDI].midiRouting) USB_MIDI.sendRealTime(midi::Start);
  if (EspMidiRouting())                    ESP_MIDI.sendRealTime(midi::Start);
  if (MTC.getMode() == MidiTimeCode::SynchroClockSlave) MTC.sendPlay();
}

void OnDinMidiContinue(void)
{
  if (interfaces[PED_USBMIDI].midiRouting) USB_MIDI.sendRealTime(midi::Continue);
  if (EspMidiRouting())                    ESP_MIDI.sendRealTime(midi::Continue);
  if (MTC.getMode() == MidiTimeCode::SynchroClockSlave) MTC.sendContinue();
}

void OnDinMidiStop(void)
{
  if (interfaces[PED_USBMIDI].midiRouting) USB_MIDI.sendRealTime(midi::Stop);
  if (EspMidiRouting())                    ESP_MIDI.sendRealTime(midi::Stop);
  if (MTC.getMode() == MidiTimeCode::SynchroClockSlave) MTC.sendStop();
}

void OnDinMidiActiveSensing(void)
{
  if (interfaces[PED_USBMIDI].midiRouting) USB_MIDI.sendRealTime(midi::ActiveSensing);
  if (EspMidiRouting())                    ESP_MIDI.sendRealTime(midi::ActiveSensing);
}

void OnDinMidiSystemReset(void)
{
  if (interfaces[PED_USBMIDI].midiRouting) USB_MIDI.sendRealTime(midi::SystemReset);
  if (EspMidiRouting())                    ESP_MIDI.sendRealTime(midi::SystemReset);
}


// Forward messages received from WiFI MIDI interface

void OnEspMidiNoteOn(byte channel, byte note, byte velocity)
{
  if (interfaces[PED_USBMIDI].midiRouting) USB_MIDI.sendNoteOn(note, velocity, channel);
  if (interfaces[PED_DINMIDI].midiRouting) DIN_MIDI.sendNoteOn(note, velocity, channel);
}

void OnEspMidiNoteOff(byte channel, byte note, byte velocity)
{
  if (interfaces[PED_USBMIDI].midiRouting) USB_MIDI.sendNoteOff(note, velocity, channel);
  if (interfaces[PED_DINMIDI].midiRouting) DIN_MIDI.sendNoteOff(note, velocity, channel);
}

void OnEspMidiReceiveAfterTouchPoly(byte channel, byte note, byte pressure)
{
  if (interfaces[PED_USBMIDI].midiRouting) USB_MIDI.sendAfterTouch(note, pressure, channel);
  if (interfaces[PED_DINMIDI].midiRouting) DIN_MIDI.sendAfterTouch(note, pressure, channel);
}

void OnEspMidiReceiveControlChange(byte channel, byte number, byte value)
{
  if (interfaces[PED_USBMIDI].midiRouting) USB_MIDI.sendControlChange(number, value, channel);
  if (interfaces[PED_DINMIDI].midiRouting) DIN_MIDI.sendControlChange(number, value, channel);
}

void OnEspMidiReceiveProgramChange(byte channel, byte number)
{
  if (interfaces[PED_USBMIDI].midiRouting) USB_MIDI.sendProgramChange(number, channel);
  if (interfaces[PED_DINMIDI].midiRouting) DIN_MIDI.sendProgramChange(number, channel);
}

void OnEspMidiReceiveAfterTouchChannel(byte channel, byte pressure)
{
  if (interfaces[PED_USBMIDI].midiRouting) USB_MIDI.sendAfterTouch(pressure, channel);
  if (interfaces[PED_DINMIDI].midiRouting) DIN_MIDI.sendAfterTouch(pressure, channel);
}

void OnEspMidiReceivePitchBend(byte channel, int bend)
{
  if (interfaces[PED_USBMIDI].midiRouting) USB_MIDI.sendPitchBend(bend, channel);
  if (interfaces[PED_DINMIDI].midiRouting) DIN_MIDI.sendPitchBend(bend, channel);
}

void OnEspMidiReceiveTimeCodeQuarterFrame(byte data)
{
  if (interfaces[PED_USBMIDI].midiRouting) USB_MIDI.sendTimeCodeQuarterFrame(data);
  if (interfaces[PED_DINMIDI].midiRouting) DIN_MIDI.sendTimeCodeQuarterFrame(data);
  MTC.decodMTCQuarterFrame(data);
}

void OnEspMidiReceiveSongPosition(unsigned int beats)
{
  if (interfaces[PED_USBMIDI].midiRouting) USB_MIDI.sendSongPosition(beats);
  if (interfaces[PED_DINMIDI].midiRouting) DIN_MIDI.sendSongPosition(beats);
}

void OnEspMidiReceiveSongSelect(byte songnumber)
{
  if (interfaces[PED_USBMIDI].midiRouting) USB_MIDI.sendSongSelect(songnumber);
  if (interfaces[PED_DINMIDI].midiRouting) DIN_MIDI.sendSongSelect(songnumber);
}

void OnEspMidiReceiveTuneRequest(void)
{
  if (interfaces[PED_USBMIDI].midiRouting) USB_MIDI.sendTuneRequest();
  if (interfaces[PED_DINMIDI].midiRouting) DIN_MIDI.sendTuneRequest();
}

void OnEspMidiReceiveClock(void)
{
  if (interfaces[PED_USBMIDI].midiRouting) USB_MIDI.sendRealTime(midi::Clock);
  if (interfaces[PED_DINMIDI].midiRouting) DIN_MIDI.sendRealTime(midi::Clock);
  if (MTC.getMode() == MidiTimeCode::SynchroClockSlave) bpm = MTC.tapTempo();
}

void OnEspMidiReceiveStart(void)
{
  if (interfaces[PED_USBMIDI].midiRouting) USB_MIDI.sendRealTime(midi::Start);
  if (interfaces[PED_DINMIDI].midiRouting) DIN_MIDI.sendRealTime(midi::Start);
  if (MTC.getMode() == MidiTimeCode::SynchroClockSlave) MTC.sendPlay();
}

void OnEspMidiReceiveContinue(void)
{
  if (interfaces[PED_USBMIDI].midiRouting) USB_MIDI.sendRealTime(midi::Continue);
  if (interfaces[PED_DINMIDI].midiRouting) DIN_MIDI.sendRealTime(midi::Continue);
  if (MTC.getMode() == MidiTimeCode::SynchroClockSlave) MTC.sendContinue();
}

void OnEspMidiReceiveStop(void)
{
  if (interfaces[PED_USBMIDI].midiRouting) USB_MIDI.sendRealTime(midi::Stop);
  if (interfaces[PED_DINMIDI].midiRouting) DIN_MIDI.sendRealTime(midi::Stop);
  if (MTC.getMode() == MidiTimeCode::SynchroClockSlave) MTC.sendStop();
}

void OnEspMidiReceiveActiveSensing(void)
{
  if (interfaces[PED_USBMIDI].midiRouting) USB_MIDI.sendRealTime(midi::ActiveSensing);
  if (interfaces[PED_DINMIDI].midiRouting) DIN_MIDI.sendRealTime(midi::ActiveSensing);
}

void OnEspMidiReceiveReset(void)
{
  if (interfaces[PED_USBMIDI].midiRouting) USB_MIDI.sendRealTime(midi::SystemReset);
  if (interfaces[PED_DINMIDI].midiRouting) DIN_MIDI.sendRealTime(midi::SystemReset);
}

void OnEspMidiReceiveSysEx(byte *data, unsigned int size)
{
  char json[size - 1];

  // Extract JSON string
  memset(json, 0, size - 1);
  memcpy(json, &data[1], size - 2);   // discard first and last byte
  DPRINTLN(json);

  // Memory pool for JSON object tree.
  StaticJsonBuffer<200> jsonBuffer;

  // Root of the object tree.
  JsonObject& root = jsonBuffer.parseObject(json);

  // Test if parsing succeeds.
  if (root.success()) {
    // Fetch values.
    //   
    if (root.containsKey("on")) {

    }
    else if (root.containsKey("wifi.on")) {
      
    }
    else if (root.containsKey("wifi.connected")) {
       wifiConnected = root["wifi.connected"];
    }
    else if (root.containsKey("ble.on")) {

    }
    else if (root.containsKey("ble.connected")) {
      bleConnected = root["ble.connected"];
    }
    else {
      if (interfaces[PED_USBMIDI].midiRouting) USB_MIDI.sendSysEx(size, data);
      if (interfaces[PED_DINMIDI].midiRouting) DIN_MIDI.sendSysEx(size, data);
      MTC.decodeMTCFullFrame(size, data);
    }
  }
}


void midi_routing_start()
{
  // Connect the handle function called upon reception of a MIDI message from USB MIDI interface

  USB_MIDI.setHandleNoteOn(OnUsbMidiNoteOn);
  USB_MIDI.setHandleNoteOff(OnUsbMidiNoteOff);
  USB_MIDI.setHandleAfterTouchPoly(OnUsbMidiAfterTouchPoly);
  USB_MIDI.setHandleControlChange(OnUsbMidiControlChange);
  USB_MIDI.setHandleProgramChange(OnUsbMidiProgramChange);
  USB_MIDI.setHandleAfterTouchChannel(OnUsbMidiAfterTouchChannel);
  USB_MIDI.setHandlePitchBend(OnUsbMidiPitchBend);
  USB_MIDI.setHandleSystemExclusive(OnUsbMidiSystemExclusive);
  USB_MIDI.setHandleTimeCodeQuarterFrame(OnUsbMidiTimeCodeQuarterFrame);
  USB_MIDI.setHandleSongPosition(OnUsbMidiSongPosition);
  USB_MIDI.setHandleSongSelect(OnUsbMidiSongSelect);
  USB_MIDI.setHandleTuneRequest(OnUsbMidiTuneRequest);
  USB_MIDI.setHandleClock(OnUsbMidiClock);
  USB_MIDI.setHandleStart(OnUsbMidiStart);
  USB_MIDI.setHandleContinue(OnUsbMidiContinue);
  USB_MIDI.setHandleStop(OnUsbMidiStop);
  USB_MIDI.setHandleActiveSensing(OnUsbMidiActiveSensing);
  USB_MIDI.setHandleSystemReset(OnUsbMidiSystemReset);

  // Connect the handle function called upon reception of a MIDI message from legacy MIDI interface

  DIN_MIDI.setHandleNoteOn(OnDinMidiNoteOn);
  DIN_MIDI.setHandleNoteOff(OnDinMidiNoteOff);
  DIN_MIDI.setHandleAfterTouchPoly(OnDinMidiAfterTouchPoly);
  DIN_MIDI.setHandleControlChange(OnDinMidiControlChange);
  DIN_MIDI.setHandleProgramChange(OnDinMidiProgramChange);
  DIN_MIDI.setHandleAfterTouchChannel(OnDinMidiAfterTouchChannel);
  DIN_MIDI.setHandlePitchBend(OnDinMidiPitchBend);
  DIN_MIDI.setHandleSystemExclusive(OnDinMidiSystemExclusive);
  DIN_MIDI.setHandleTimeCodeQuarterFrame(OnDinMidiTimeCodeQuarterFrame);
  DIN_MIDI.setHandleSongPosition(OnDinMidiSongPosition);
  DIN_MIDI.setHandleSongSelect(OnDinMidiSongSelect);
  DIN_MIDI.setHandleTuneRequest(OnDinMidiTuneRequest);
  DIN_MIDI.setHandleClock(OnDinMidiClock);
  DIN_MIDI.setHandleStart(OnDinMidiStart);
  DIN_MIDI.setHandleContinue(OnDinMidiContinue);
  DIN_MIDI.setHandleStop(OnDinMidiStop);
  DIN_MIDI.setHandleActiveSensing(OnDinMidiActiveSensing);
  DIN_MIDI.setHandleSystemReset(OnDinMidiSystemReset);

  // Connect the handle function called upon reception of a MIDI message from WiFi MIDI interface

  ESP_MIDI.setHandleNoteOn(OnEspMidiNoteOn);
  ESP_MIDI.setHandleNoteOff(OnEspMidiNoteOff);
  ESP_MIDI.setHandleAfterTouchPoly(OnEspMidiReceiveAfterTouchPoly);
  ESP_MIDI.setHandleControlChange(OnEspMidiReceiveControlChange);
  ESP_MIDI.setHandleProgramChange(OnEspMidiReceiveProgramChange);
  ESP_MIDI.setHandleAfterTouchChannel(OnEspMidiReceiveAfterTouchChannel);
  ESP_MIDI.setHandlePitchBend(OnEspMidiReceivePitchBend);
  ESP_MIDI.setHandleSystemExclusive(OnEspMidiReceiveSysEx);
  ESP_MIDI.setHandleTimeCodeQuarterFrame(OnEspMidiReceiveTimeCodeQuarterFrame);
  ESP_MIDI.setHandleSongPosition(OnEspMidiReceiveSongPosition);
  ESP_MIDI.setHandleSongSelect(OnEspMidiReceiveSongSelect);
  ESP_MIDI.setHandleTuneRequest(OnEspMidiReceiveTuneRequest);
  ESP_MIDI.setHandleClock(OnEspMidiReceiveClock);
  ESP_MIDI.setHandleStart(OnEspMidiReceiveStart);
  ESP_MIDI.setHandleContinue(OnEspMidiReceiveContinue);
  ESP_MIDI.setHandleStop(OnEspMidiReceiveStop);
  ESP_MIDI.setHandleActiveSensing(OnEspMidiReceiveActiveSensing);
  ESP_MIDI.setHandleSystemReset(OnEspMidiReceiveReset);
}

