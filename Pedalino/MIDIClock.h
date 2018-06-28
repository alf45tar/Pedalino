// The callback function wich will be called by Clock each Pulse of 96PPQN clock resolution.
void clockOut96PPQN(uint32_t * tick)
{
    if (interfaces[PED_USBMIDI].midiClock)    USB_MIDI.sendRealTime(midi::Clock);
    if (interfaces[PED_LEGACYMIDI].midiClock) DIN_MIDI.sendRealTime(midi::Clock);
    if (interfaces[PED_APPLEMIDI].midiClock)  RTP_MIDI.sendRealTime(midi::Clock);
}

// The callback function wich will be called when clock starts by using Clock.start() method.
void onClockStart()
{
  if (interfaces[PED_USBMIDI].midiClock)    USB_MIDI.sendRealTime(midi::Start);
  if (interfaces[PED_LEGACYMIDI].midiClock) DIN_MIDI.sendRealTime(midi::Start);
  if (interfaces[PED_APPLEMIDI].midiClock)  RTP_MIDI.sendRealTime(midi::Start);
}

// The callback function wich will be called when clock stops by using Clock.stop() method.
void onClockStop()
{
  if (interfaces[PED_USBMIDI].midiClock)    USB_MIDI.sendRealTime(midi::Stop);
  if (interfaces[PED_LEGACYMIDI].midiClock) DIN_MIDI.sendRealTime(midi::Stop);
  if (interfaces[PED_APPLEMIDI].midiClock)  RTP_MIDI.sendRealTime(midi::Stop);
}


void midi_clock_setup()
{
  return;
  
  // Inits the clock
  uClock.init();
  // Set the callback function for the clock output to send MIDI Sync message.
  uClock.setClock96PPQNOutput(clockOut96PPQN);
  // Set the callback function for MIDI Start and Stop messages.
  uClock.setOnClockStartOutput(onClockStart);
  uClock.setOnClockStopOutput(onClockStop);
  // Set the clock BPM
  uClock.setTempo(100);
  //uClock.start();
}
