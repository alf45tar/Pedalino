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

  if (interfaces[PED_LEGACYMIDI].midiIn)
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
    
  if (interfaces[PED_APPLEMIDI].midiIn)
    if (RTP_MIDI.read()) {
      DPRINTF(" MIDI IN RTP -> STATUS ");
      DPRINT(RTP_MIDI.getType());
      DPRINTF(" DATA1 ");
      DPRINT(RTP_MIDI.getData1());
      DPRINTF(" DATA2 ");
      DPRINT(RTP_MIDI.getData2());
      DPRINTF(" CHANNEL ");
      DPRINTLN(RTP_MIDI.getChannel());
    }
}

void OnUsbMidiNoteOn(byte channel, byte note, byte velocity)
{
  DIN_MIDI.sendNoteOn(note, velocity, channel);
  RTP_MIDI.sendNoteOn(note, velocity, channel);
}

void OnUsbMidiNoteOff(byte channel, byte note, byte velocity)
{
  DIN_MIDI.sendNoteOff(note, velocity, channel);
  RTP_MIDI.sendNoteOff(note, velocity, channel);
}

void OnUsbMidiAfterTouchPoly(byte channel, byte note, byte pressure)
{
  DIN_MIDI.sendAfterTouch(note, pressure, channel);
  RTP_MIDI.sendAfterTouch(note, pressure, channel);
}

void OnUsbMidiControlChange(byte channel, byte number, byte value)
{
  DIN_MIDI.sendControlChange(number, value, channel);
  RTP_MIDI.sendControlChange(number, value, channel);
}

void OnUsbMidiProgramChange(byte channel, byte number)
{
  DIN_MIDI.sendProgramChange(number, channel);
  RTP_MIDI.sendProgramChange(number, channel);
}

void OnUsbMidiAfterTouchChannel(byte channel, byte pressure)
{
  DIN_MIDI.sendAfterTouch(pressure, channel);
  RTP_MIDI.sendAfterTouch(pressure, channel);
}

void OnUsbMidiPitchBend(byte channel, int bend)
{
  DIN_MIDI.sendPitchBend(bend, channel);
  RTP_MIDI.sendPitchBend(bend, channel);
}

void OnUsbMidiSystemExclusive(byte * array, unsigned size)
{
  DIN_MIDI.sendSysEx(size, array);
  RTP_MIDI.sendSysEx(size, array);
  MTC.decodeMTCFullFrame(size, array);   
}

void OnUsbMidiTimeCodeQuarterFrame(byte data)
{
  DIN_MIDI.sendTimeCodeQuarterFrame(data);
  RTP_MIDI.sendTimeCodeQuarterFrame(data);
  MTC.decodMTCQuarterFrame(data);
}

void OnUsbMidiSongPosition(unsigned int beats)
{
  DIN_MIDI.sendSongPosition(beats);
  RTP_MIDI.sendSongPosition(beats);
}

void OnUsbMidiSongSelect(byte songnumber)
{
  DIN_MIDI.sendSongSelect(songnumber);
  RTP_MIDI.sendSongSelect(songnumber);
}

void OnUsbMidiTuneRequest(void)
{
  DIN_MIDI.sendTuneRequest();
  RTP_MIDI.sendTuneRequest();
}

void OnUsbMidiClock(void)
{
  DIN_MIDI.sendRealTime(midi::Clock);
  RTP_MIDI.sendRealTime(midi::Clock);
  if (MTC.getMode() == MidiTimeCode::SynchroClockSlave) bpm = MTC.tapTempo();
}

void OnUsbMidiStart(void)
{
  DIN_MIDI.sendRealTime(midi::Start);
  RTP_MIDI.sendRealTime(midi::Start);
  if (MTC.getMode() == MidiTimeCode::SynchroClockSlave) MTC.sendPlay();
}

void OnUsbMidiContinue(void)
{
  DIN_MIDI.sendRealTime(midi::Continue);
  RTP_MIDI.sendRealTime(midi::Continue);
  if (MTC.getMode() == MidiTimeCode::SynchroClockSlave) MTC.sendContinue();
}

void OnUsbMidiStop(void)
{
  DIN_MIDI.sendRealTime(midi::Stop);
  RTP_MIDI.sendRealTime(midi::Stop);
  if (MTC.getMode() == MidiTimeCode::SynchroClockSlave) MTC.sendStop();
}

void OnUsbMidiActiveSensing(void)
{
  DIN_MIDI.sendRealTime(midi::ActiveSensing);
  RTP_MIDI.sendRealTime(midi::ActiveSensing);
}

void OnUsbMidiSystemReset(void)
{
  DIN_MIDI.sendRealTime(midi::SystemReset);
  RTP_MIDI.sendRealTime(midi::SystemReset);
}


// Forward messages received from legacy MIDI interface

void OnDinMidiNoteOn(byte channel, byte note, byte velocity)
{
  USB_MIDI.sendNoteOn(note, velocity, channel);
  RTP_MIDI.sendNoteOn(note, velocity, channel);
}

void OnDinMidiNoteOff(byte channel, byte note, byte velocity)
{
  USB_MIDI.sendNoteOff(note, velocity, channel);
  RTP_MIDI.sendNoteOff(note, velocity, channel);
}

void OnDinMidiAfterTouchPoly(byte channel, byte note, byte pressure)
{
  USB_MIDI.sendAfterTouch(note, pressure, channel);
  RTP_MIDI.sendAfterTouch(note, pressure, channel);
}

void OnDinMidiControlChange(byte channel, byte number, byte value)
{
  USB_MIDI.sendControlChange(number, value, channel);
  RTP_MIDI.sendControlChange(number, value, channel);
}

void OnDinMidiProgramChange(byte channel, byte number)
{
  USB_MIDI.sendProgramChange(number, channel);
  RTP_MIDI.sendProgramChange(number, channel);
}

void OnDinMidiAfterTouchChannel(byte channel, byte pressure)
{
  USB_MIDI.sendAfterTouch(pressure, channel);
  RTP_MIDI.sendAfterTouch(pressure, channel);
}

void OnDinMidiPitchBend(byte channel, int bend)
{
  USB_MIDI.sendPitchBend(bend, channel);
  RTP_MIDI.sendPitchBend(bend, channel);
}

void OnDinMidiSystemExclusive(byte * array, unsigned size)
{
  USB_MIDI.sendSysEx(size, array);
  RTP_MIDI.sendSysEx(size, array);
  MTC.decodeMTCFullFrame(size, array);
}

void OnDinMidiTimeCodeQuarterFrame(byte data)
{
  USB_MIDI.sendTimeCodeQuarterFrame(data);
  RTP_MIDI.sendTimeCodeQuarterFrame(data);
  MTC.decodMTCQuarterFrame(data);
}

void OnDinMidiSongPosition(unsigned int beats)
{
  USB_MIDI.sendSongPosition(beats);
  RTP_MIDI.sendSongPosition(beats);
}

void OnDinMidiSongSelect(byte songnumber)
{
  USB_MIDI.sendSongSelect(songnumber);
  RTP_MIDI.sendSongSelect(songnumber);
}

void OnDinMidiTuneRequest(void)
{
  USB_MIDI.sendTuneRequest();
  RTP_MIDI.sendTuneRequest();
}

void OnDinMidiClock(void)
{
  USB_MIDI.sendRealTime(midi::Clock);
  RTP_MIDI.sendRealTime(midi::Clock);
  if (MTC.getMode() == MidiTimeCode::SynchroClockSlave) bpm = MTC.tapTempo();
}

void OnDinMidiStart(void)
{
  USB_MIDI.sendRealTime(midi::Start);
  RTP_MIDI.sendRealTime(midi::Start);
  if (MTC.getMode() == MidiTimeCode::SynchroClockSlave) MTC.sendPlay();
}

void OnDinMidiContinue(void)
{
  USB_MIDI.sendRealTime(midi::Continue);
  RTP_MIDI.sendRealTime(midi::Continue);
  if (MTC.getMode() == MidiTimeCode::SynchroClockSlave) MTC.sendContinue();
}

void OnDinMidiStop(void)
{
  USB_MIDI.sendRealTime(midi::Stop);
  RTP_MIDI.sendRealTime(midi::Stop);
  if (MTC.getMode() == MidiTimeCode::SynchroClockSlave) MTC.sendStop();
}

void OnDinMidiActiveSensing(void)
{
  USB_MIDI.sendRealTime(midi::ActiveSensing);
  RTP_MIDI.sendRealTime(midi::ActiveSensing);
}

void OnDinMidiSystemReset(void)
{
  USB_MIDI.sendRealTime(midi::SystemReset);
  RTP_MIDI.sendRealTime(midi::SystemReset);
}


// Forward messages received from WiFI MIDI interface

void OnAppleMidiNoteOn(byte channel, byte note, byte velocity)
{
  USB_MIDI.sendNoteOn(note, velocity, channel);
  DIN_MIDI.sendNoteOn(note, velocity, channel);
}

void OnAppleMidiNoteOff(byte channel, byte note, byte velocity)
{
  USB_MIDI.sendNoteOff(note, velocity, channel);
  DIN_MIDI.sendNoteOff(note, velocity, channel);
}

void OnAppleMidiReceiveAfterTouchPoly(byte channel, byte note, byte pressure)
{
  USB_MIDI.sendAfterTouch(note, pressure, channel);
  DIN_MIDI.sendAfterTouch(note, pressure, channel);
}

void OnAppleMidiReceiveControlChange(byte channel, byte number, byte value)
{
  USB_MIDI.sendControlChange(number, value, channel);
  DIN_MIDI.sendControlChange(number, value, channel);
}

void OnAppleMidiReceiveProgramChange(byte channel, byte number)
{
  USB_MIDI.sendProgramChange(number, channel);
  DIN_MIDI.sendProgramChange(number, channel);
}

void OnAppleMidiReceiveAfterTouchChannel(byte channel, byte pressure)
{
  USB_MIDI.sendAfterTouch(pressure, channel);
  DIN_MIDI.sendAfterTouch(pressure, channel);
}

void OnAppleMidiReceivePitchBend(byte channel, int bend)
{
  USB_MIDI.sendPitchBend(bend, channel);
  DIN_MIDI.sendPitchBend(bend, channel);
}

void OnAppleMidiReceiveSysEx(byte *data, unsigned int size)
{
  char json[size - 1];
  byte decodedArray[size];
  unsigned int decodedSize;

  // Extract JSON string
  memset(json, 0, size - 1);
  memcpy(json, &data[1], size - 2);
  DPRINT("JSON: ");
  DPRINTLN(json);

  // Memory pool for JSON object tree.
  StaticJsonBuffer<200> jsonBuffer;

  // Root of the object tree.
  JsonObject& root = jsonBuffer.parseObject(json);

  // Test if parsing succeeds.
  if (root.success()) {
    // Fetch values.
    //   
    if (root.containsKey("wifi.on")) {
      
    }
    else if (root.containsKey("wifi.connected")) {
       
    }
    else if (root.containsKey("wifi.disconnected")) {
       
    }
    else if (root.containsKey("ble.on")) {
      
    }
    else if (root.containsKey("ble.connected")) {
      
    }
    else if (root.containsKey("ble.disconnected")) {
      
    }
    else {
      USB_MIDI.sendSysEx(size, data);
      DIN_MIDI.sendSysEx(size, data);
      MTC.decodeMTCFullFrame(size, data);
    }
  }
}

void OnAppleMidiReceiveTimeCodeQuarterFrame(byte data)
{
  USB_MIDI.sendTimeCodeQuarterFrame(data);
  DIN_MIDI.sendTimeCodeQuarterFrame(data);
  MTC.decodMTCQuarterFrame(data);
}

void OnAppleMidiReceiveSongPosition(unsigned int beats)
{
  USB_MIDI.sendSongPosition(beats);
  DIN_MIDI.sendSongPosition(beats);
}

void OnAppleMidiReceiveSongSelect(byte songnumber)
{
  USB_MIDI.sendSongSelect(songnumber);
  DIN_MIDI.sendSongSelect(songnumber);
}

void OnAppleMidiReceiveTuneRequest(void)
{
  USB_MIDI.sendTuneRequest();
  DIN_MIDI.sendTuneRequest();
}

void OnAppleMidiReceiveClock(void)
{
  USB_MIDI.sendRealTime(midi::Clock);
  DIN_MIDI.sendRealTime(midi::Clock);
  if (MTC.getMode() == MidiTimeCode::SynchroClockSlave) bpm = MTC.tapTempo();
}

void OnAppleMidiReceiveStart(void)
{
  USB_MIDI.sendRealTime(midi::Start);
  DIN_MIDI.sendRealTime(midi::Start);
  if (MTC.getMode() == MidiTimeCode::SynchroClockSlave) MTC.sendPlay();
}

void OnAppleMidiReceiveContinue(void)
{
  USB_MIDI.sendRealTime(midi::Continue);
  DIN_MIDI.sendRealTime(midi::Continue);
  if (MTC.getMode() == MidiTimeCode::SynchroClockSlave) MTC.sendContinue();
}

void OnAppleMidiReceiveStop(void)
{
  USB_MIDI.sendRealTime(midi::Stop);
  DIN_MIDI.sendRealTime(midi::Stop);
  if (MTC.getMode() == MidiTimeCode::SynchroClockSlave) MTC.sendStop();
}

void OnAppleMidiReceiveActiveSensing(void)
{
  USB_MIDI.sendRealTime(midi::ActiveSensing);
  DIN_MIDI.sendRealTime(midi::ActiveSensing);
}

void OnAppleMidiReceiveReset(void)
{
  USB_MIDI.sendRealTime(midi::SystemReset);
  DIN_MIDI.sendRealTime(midi::SystemReset);
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

  RTP_MIDI.setHandleNoteOn(OnAppleMidiNoteOn);
  RTP_MIDI.setHandleNoteOff(OnAppleMidiNoteOff);
  RTP_MIDI.setHandleAfterTouchPoly(OnAppleMidiReceiveAfterTouchPoly);
  RTP_MIDI.setHandleControlChange(OnAppleMidiReceiveControlChange);
  RTP_MIDI.setHandleProgramChange(OnAppleMidiReceiveProgramChange);
  RTP_MIDI.setHandleAfterTouchChannel(OnAppleMidiReceiveAfterTouchChannel);
  RTP_MIDI.setHandlePitchBend(OnAppleMidiReceivePitchBend);
  RTP_MIDI.setHandleSystemExclusive(OnAppleMidiReceiveSysEx);
  RTP_MIDI.setHandleTimeCodeQuarterFrame(OnAppleMidiReceiveTimeCodeQuarterFrame);
  RTP_MIDI.setHandleSongPosition(OnAppleMidiReceiveSongPosition);
  RTP_MIDI.setHandleSongSelect(OnAppleMidiReceiveSongSelect);
  RTP_MIDI.setHandleTuneRequest(OnAppleMidiReceiveTuneRequest);
  RTP_MIDI.setHandleClock(OnAppleMidiReceiveClock);
  RTP_MIDI.setHandleStart(OnAppleMidiReceiveStart);
  RTP_MIDI.setHandleContinue(OnAppleMidiReceiveContinue);
  RTP_MIDI.setHandleStop(OnAppleMidiReceiveStop);
  RTP_MIDI.setHandleActiveSensing(OnAppleMidiReceiveActiveSensing);
  RTP_MIDI.setHandleSystemReset(OnAppleMidiReceiveReset);
}

