// Forward messages received from USB MIDI interface

void OnUsbMidiNoteOn(byte channel, byte note, byte velocity)
{
  if (currentLegacyMIDIPort == PED_LEGACY_MIDI_OUT) DIN_MIDI.sendNoteOn(note, velocity, channel);
  RTP_MIDI.sendNoteOn(note, velocity, channel);
}

void OnUsbMidiNoteOff(byte channel, byte note, byte velocity)
{
  if (currentLegacyMIDIPort == PED_LEGACY_MIDI_OUT) DIN_MIDI.sendNoteOff(note, velocity, channel);
  RTP_MIDI.sendNoteOff(note, velocity, channel);
}

void OnUsbMidiAfterTouchPoly(byte channel, byte note, byte pressure)
{
  if (currentLegacyMIDIPort == PED_LEGACY_MIDI_OUT) DIN_MIDI.sendPolyPressure(note, pressure, channel);
  RTP_MIDI.sendPolyPressure(note, pressure, channel);
}

void OnUsbMidiControlChange(byte channel, byte number, byte value)
{
  if (currentLegacyMIDIPort == PED_LEGACY_MIDI_OUT) DIN_MIDI.sendControlChange(number, value, channel);
  RTP_MIDI.sendControlChange(number, value, channel);
}

void OnUsbMidiProgramChange(byte channel, byte number)
{
  if (currentLegacyMIDIPort == PED_LEGACY_MIDI_OUT) DIN_MIDI.sendProgramChange(number, channel);
  RTP_MIDI.sendProgramChange(number, channel);
}

void OnUsbMidiAfterTouchChannel(byte channel, byte pressure)
{
  if (currentLegacyMIDIPort == PED_LEGACY_MIDI_OUT) DIN_MIDI.sendAfterTouch(pressure, channel);
  RTP_MIDI.sendAfterTouch(pressure, channel);
}

void OnUsbMidiPitchBend(byte channel, int bend)
{
  if (currentLegacyMIDIPort == PED_LEGACY_MIDI_OUT) DIN_MIDI.sendPitchBend(bend, channel);
  RTP_MIDI.sendPitchBend(bend, channel);
}

void OnUsbMidiSystemExclusive(byte* array, unsigned size)
{
  if (currentLegacyMIDIPort == PED_LEGACY_MIDI_OUT) DIN_MIDI.sendSysEx(size, array);
  RTP_MIDI.sendSysEx(size, array);
}

void OnUsbMidiTimeCodeQuarterFrame(byte data)
{
  if (currentLegacyMIDIPort == PED_LEGACY_MIDI_OUT) DIN_MIDI.sendTimeCodeQuarterFrame(data);
  RTP_MIDI.sendTimeCodeQuarterFrame(data);
}

void OnUsbMidiSongPosition(unsigned int beats)
{
  if (currentLegacyMIDIPort == PED_LEGACY_MIDI_OUT) DIN_MIDI.sendSongPosition(beats);
  RTP_MIDI.sendSongPosition(beats);
}

void OnUsbMidiSongSelect(byte songnumber)
{
  if (currentLegacyMIDIPort == PED_LEGACY_MIDI_OUT) DIN_MIDI.sendSongSelect(songnumber);
  RTP_MIDI.sendSongSelect(songnumber);
}

void OnUsbMidiTuneRequest(void)
{
  if (currentLegacyMIDIPort == PED_LEGACY_MIDI_OUT) DIN_MIDI.sendTuneRequest();
  RTP_MIDI.sendTuneRequest();
}

void OnUsbMidiClock(void)
{
  if (currentLegacyMIDIPort == PED_LEGACY_MIDI_OUT) DIN_MIDI.sendRealTime(midi::Clock);
  RTP_MIDI.sendRealTime(midi::Clock);
}

void OnUsbMidiStart(void)
{
  if (currentLegacyMIDIPort == PED_LEGACY_MIDI_OUT) DIN_MIDI.sendRealTime(midi::Start);
  RTP_MIDI.sendRealTime(midi::Start);
}

void OnUsbMidiContinue(void)
{
  if (currentLegacyMIDIPort == PED_LEGACY_MIDI_OUT) DIN_MIDI.sendRealTime(midi::Continue);
  RTP_MIDI.sendRealTime(midi::Continue);
}

void OnUsbMidiStop(void)
{
  if (currentLegacyMIDIPort == PED_LEGACY_MIDI_OUT) DIN_MIDI.sendRealTime(midi::Stop);
  RTP_MIDI.sendRealTime(midi::Stop);
}

void OnUsbMidiActiveSensing(void)
{
  if (currentLegacyMIDIPort == PED_LEGACY_MIDI_OUT) DIN_MIDI.sendRealTime(midi::ActiveSensing);
  RTP_MIDI.sendRealTime(midi::ActiveSensing);
}

void OnUsbMidiSystemReset(void)
{
  if (currentLegacyMIDIPort == PED_LEGACY_MIDI_OUT) DIN_MIDI.sendRealTime(midi::SystemReset);
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
  USB_MIDI.sendPolyPressure(note, pressure, channel);
  RTP_MIDI.sendPolyPressure(note, pressure, channel);
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

void OnDinMidiSystemExclusive(byte* array, unsigned size)
{
  USB_MIDI.sendSysEx(size, array);
  RTP_MIDI.sendSysEx(size, array);
}

void OnDinMidiTimeCodeQuarterFrame(byte data)
{
  USB_MIDI.sendTimeCodeQuarterFrame(data);
  RTP_MIDI.sendTimeCodeQuarterFrame(data);
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
}

void OnDinMidiStart(void)
{
  USB_MIDI.sendRealTime(midi::Start);
  RTP_MIDI.sendRealTime(midi::Start);
}

void OnDinMidiContinue(void)
{
  USB_MIDI.sendRealTime(midi::Continue);
  RTP_MIDI.sendRealTime(midi::Continue);
}

void OnDinMidiStop(void)
{
  USB_MIDI.sendRealTime(midi::Stop);
  RTP_MIDI.sendRealTime(midi::Stop);
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
  if (currentLegacyMIDIPort == PED_LEGACY_MIDI_OUT) DIN_MIDI.sendNoteOn(note, velocity, channel);
}

void OnAppleMidiNoteOff(byte channel, byte note, byte velocity)
{
  USB_MIDI.sendNoteOff(note, velocity, channel);
  if (currentLegacyMIDIPort == PED_LEGACY_MIDI_OUT) DIN_MIDI.sendNoteOff(note, velocity, channel);
}

void OnAppleMidiReceiveAfterTouchPoly(byte channel, byte note, byte pressure)
{
  USB_MIDI.sendPolyPressure(note, pressure, channel);
  if (currentLegacyMIDIPort == PED_LEGACY_MIDI_OUT) DIN_MIDI.sendPolyPressure(note, pressure, channel);
}

void OnAppleMidiReceiveControlChange(byte channel, byte number, byte value)
{
  USB_MIDI.sendControlChange(number, value, channel);
  if (currentLegacyMIDIPort == PED_LEGACY_MIDI_OUT) DIN_MIDI.sendControlChange(number, value, channel);
}

void OnAppleMidiReceiveProgramChange(byte channel, byte number)
{
  USB_MIDI.sendProgramChange(number, channel);
  if (currentLegacyMIDIPort == PED_LEGACY_MIDI_OUT) DIN_MIDI.sendProgramChange(number, channel);
}

void OnAppleMidiReceiveAfterTouchChannel(byte channel, byte pressure)
{
  USB_MIDI.sendAfterTouch(pressure, channel);
  if (currentLegacyMIDIPort == PED_LEGACY_MIDI_OUT) DIN_MIDI.sendAfterTouch(pressure, channel);
}

void OnAppleMidiReceivePitchBend(byte channel, int bend)
{
  USB_MIDI.sendPitchBend(bend, channel);
  if (currentLegacyMIDIPort == PED_LEGACY_MIDI_OUT) DIN_MIDI.sendPitchBend(bend, channel);
}

void OnAppleMidiReceiveSysEx(const byte * data, uint16_t size)
{
  USB_MIDI.sendSysEx(size, data);
  if (currentLegacyMIDIPort == PED_LEGACY_MIDI_OUT) DIN_MIDI.sendSysEx(size, data);
}

void OnAppleMidiReceiveTimeCodeQuarterFrame(byte data)
{
  USB_MIDI.sendTimeCodeQuarterFrame(data);
  if (currentLegacyMIDIPort == PED_LEGACY_MIDI_OUT) DIN_MIDI.sendTimeCodeQuarterFrame(data);
}

void OnAppleMidiReceiveSongPosition(unsigned short beats)
{
  USB_MIDI.sendSongPosition(beats);
  if (currentLegacyMIDIPort == PED_LEGACY_MIDI_OUT) DIN_MIDI.sendSongPosition(beats);
}

void OnAppleMidiReceiveSongSelect(byte songnumber)
{
  USB_MIDI.sendSongSelect(songnumber);
  if (currentLegacyMIDIPort == PED_LEGACY_MIDI_OUT) DIN_MIDI.sendSongSelect(songnumber);
}

void OnAppleMidiReceiveTuneRequest(void)
{
  USB_MIDI.sendTuneRequest();
  if (currentLegacyMIDIPort == PED_LEGACY_MIDI_OUT) DIN_MIDI.sendTuneRequest();
}

void OnAppleMidiReceiveClock(void)
{
  USB_MIDI.sendRealTime(midi::Clock);
  if (currentLegacyMIDIPort == PED_LEGACY_MIDI_OUT) DIN_MIDI.sendRealTime(midi::Clock);
}

void OnAppleMidiReceiveStart(void)
{
  USB_MIDI.sendRealTime(midi::Start);
  if (currentLegacyMIDIPort == PED_LEGACY_MIDI_OUT) DIN_MIDI.sendRealTime(midi::Start);
}

void OnAppleMidiReceiveContinue(void)
{
  USB_MIDI.sendRealTime(midi::Continue);
  if (currentLegacyMIDIPort == PED_LEGACY_MIDI_OUT) DIN_MIDI.sendRealTime(midi::Continue);
}

void OnAppleMidiReceiveStop(void)
{
  USB_MIDI.sendRealTime(midi::Stop);
  if (currentLegacyMIDIPort == PED_LEGACY_MIDI_OUT) DIN_MIDI.sendRealTime(midi::Stop);
}

void OnAppleMidiReceiveActiveSensing(void)
{
  USB_MIDI.sendRealTime(midi::ActiveSensing);
  if (currentLegacyMIDIPort == PED_LEGACY_MIDI_OUT) DIN_MIDI.sendRealTime(midi::ActiveSensing);
}

void OnAppleMidiReceiveReset(void)
{
  USB_MIDI.sendRealTime(midi::SystemReset);
  if (currentLegacyMIDIPort == PED_LEGACY_MIDI_OUT) DIN_MIDI.sendRealTime(midi::SystemReset);
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

void midi_routing()
{
  if (interfaces[PED_USBMIDI].midiIn) {
    if (USB_MIDI.read())
    {
      // Thru on A has already pushed the input message to out A.
      // Forward the message to out B as well.
      if (interfaces[PED_LEGACYMIDI].midiRouting)
        DIN_MIDI.send(USB_MIDI.getType(),
                      USB_MIDI.getData1(),
                      USB_MIDI.getData2(),
                      USB_MIDI.getChannel());
      if (interfaces[PED_APPLEMIDI].midiRouting)
        RTP_MIDI.send(USB_MIDI.getType(),
                      USB_MIDI.getData1(),
                      USB_MIDI.getData2(),
                      USB_MIDI.getChannel());
    }
  }
  if (interfaces[PED_LEGACYMIDI].midiIn) {
    if (DIN_MIDI.read())
    {
      // Thru on A has already pushed the input message to out A.
      // Forward the message to out B as well.
      if (interfaces[PED_USBMIDI].midiRouting)
        USB_MIDI.send(DIN_MIDI.getType(),
                      DIN_MIDI.getData1(),
                      DIN_MIDI.getData2(),
                      DIN_MIDI.getChannel());
      if (interfaces[PED_APPLEMIDI].midiRouting)
        RTP_MIDI.send(DIN_MIDI.getType(),
                      DIN_MIDI.getData1(),
                      DIN_MIDI.getData2(),
                      DIN_MIDI.getChannel());
    }
  }
  if (interfaces[PED_APPLEMIDI].midiIn) {
    if (RTP_MIDI.read())
    {
      // Thru on B has already pushed the input message to out B.
      // Forward the message to out A as well.
      if (interfaces[PED_USBMIDI].midiRouting)
        USB_MIDI.send(RTP_MIDI.getType(),
                      RTP_MIDI.getData1(),
                      RTP_MIDI.getData2(),
                      RTP_MIDI.getChannel());
      if (interfaces[PED_LEGACYMIDI].midiRouting)
        DIN_MIDI.send(RTP_MIDI.getType(),
                      RTP_MIDI.getData1(),
                      RTP_MIDI.getData2(),
                      RTP_MIDI.getChannel());
    }
  }
}



