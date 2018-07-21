// Forward messages received from one MIDI interface to the others

void midi_routing()
{
  if (interfaces[PED_USBMIDI].midiIn) {
    if (USB_MIDI.read())
    {
      // Thru on A has already pushed the input message to out A.
      // Forward the message to out B as well.
      
      DPRINT(" MIDI IN USB -> STATUS ");
      DPRINT(USB_MIDI.getType());
      DPRINT(" DATA1 ");
      DPRINT(USB_MIDI.getData1());
      DPRINT(" DATA2 ");
      DPRINT(USB_MIDI.getData2());
      DPRINT(" CHANNEL ");
      DPRINTLN(USB_MIDI.getChannel());
      
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
#ifdef DEBUG_PEDALINO
      {
        DPRINT(" MIDI IN DIN -> STATUS ");
        DPRINT(DIN_MIDI.getType());
        DPRINT(" DATA1 ");
        DPRINT(DIN_MIDI.getData1());
        DPRINT(" DATA2 ");
        DPRINT(DIN_MIDI.getData2());
        DPRINT(" CHANNEL ");
        DPRINTLN(DIN_MIDI.getChannel());
      }
#else
        USB_MIDI.send(DIN_MIDI.getType(),
                      DIN_MIDI.getData1(),
                      DIN_MIDI.getData2(),
                      DIN_MIDI.getChannel());
#endif
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
#ifdef DEBUG_PEDALINO
      {
        DPRINT(" MIDI IN RTP -> STATUS ");
        DPRINT(RTP_MIDI.getType());
        DPRINT(" DATA1 ");
        DPRINT(RTP_MIDI.getData1());
        DPRINT(" DATA2 ");
        DPRINT(RTP_MIDI.getData2());
        DPRINT(" CHANNEL ");
        DPRINTLN(RTP_MIDI.getChannel());
      }
#else
        USB_MIDI.send(RTP_MIDI.getType(),
                      RTP_MIDI.getData1(),
                      RTP_MIDI.getData2(),
                      RTP_MIDI.getChannel());
#endif
      if (interfaces[PED_LEGACYMIDI].midiRouting)
        DIN_MIDI.send(RTP_MIDI.getType(),
                      RTP_MIDI.getData1(),
                      RTP_MIDI.getData2(),
                      RTP_MIDI.getChannel());
    }
  }
}



