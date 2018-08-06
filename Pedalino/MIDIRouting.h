// Forward messages received from one MIDI interface to the others


void mtc_decode(byte b1, byte b2)
{
  static byte b[8] = {0, 0, 0, 0, 0, 0, 0, 0};
  byte i;

  if (b1 == midi::TimeCodeQuarterFrame) {
    DPRINT("MTC ");
    i = (b2 & 0xf0) >> 4;
    DPRINT(i);
    DPRINT(" ");
    if (i > 7) return;

    b[i] = b2 & 0x0f;

    if (i == 7)
    {
      byte frameType;

      frameType = b[7] & 0x06;

      byte h = (b[7] & 0x01) << 4 + b[6];
      byte m = b[5] << 4 + b[4];
      byte s = b[3] << 4 + b[2];
      byte f = b[1] << 4 + b[0];

      if (h > 23)  h = 23;
      if (m > 59)  m = 59;
      if (s > 59)  s = 59;
      if (f > 30)  f = 30;

      DPRINT(h);
      DPRINT(":");
      DPRINT(m);
      DPRINT(":");
      DPRINT(s);
      DPRINT(":");
      DPRINTLN(f);
      MTC.sendPosition(h, m, s, f);
      for (i = 0; i < 8; i++)
        b[i] = 0;
    }
  }
}


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
      mtc_decode(USB_MIDI.getType(), USB_MIDI.getData1());
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

