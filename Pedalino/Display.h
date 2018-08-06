#define LCD_LINE1_PERSISTENCE   1500;

byte m1, m2, m3, m4;
unsigned long endMillis2;


void screen_info(byte b1, byte b2, byte b3, byte b4)
{
  m1 = b1;
  m2 = b2;
  m3 = b3;
  m4 = b4;
  endMillis2 = millis() + LCD_LINE1_PERSISTENCE;
}


char foot_char (byte footswitch)
{
  footswitch = constrain(footswitch, 0, PEDALS - 1);
  if (pedals[footswitch].function != PED_MIDI) return ' ';
  if ((footswitch == lastUsedPedal) ||

      (pedals[footswitch].mode == PED_MOMENTARY1 || pedals[footswitch].mode == PED_LATCH1)
      && pedals[footswitch].pedalValue[0] == LOW ||

      (pedals[footswitch].mode == PED_MOMENTARY2 || pedals[footswitch].mode == PED_MOMENTARY3 || pedals[footswitch].mode == PED_LATCH2)
      && (pedals[footswitch].pedalValue[0] == LOW || pedals[footswitch].pedalValue[1] == LOW)) return bar1[footswitch % 10];
  return ' ';
}


void screen_update(bool force = false) {

  static char screen1[LCD_COLS + 1];
  static char screen2[LCD_COLS + 1];

  if (!powersaver) {
    char buf[LCD_COLS + 1];
    // Line 1
    if (millis() < endMillis2) {
      memset(buf, 0, sizeof(buf));
      switch (m1) {
        case midi::NoteOn:
        case midi::NoteOff:
          sprintf(&buf[strlen(buf)], "Note%3d/%3d Ch%2d", m2, m3, m4);
          break;
        case midi::ControlChange:
          sprintf(&buf[strlen(buf)], "CC %3d/%3d Ch %2d", m2, m3, m4);
          break;
        case midi::ProgramChange:
          sprintf(&buf[strlen(buf)], "PC %3d     Ch %2d", m2, m4);
          break;
        case midi::PitchBend:
          sprintf(&buf[strlen(buf)], "PiBend %3d Ch %2d", m2, m4);
          break;
      }
      strncpy(screen1, buf, LCD_COLS);
      lcd.noCursor();
      lcd.setCursor(0, 0);
      lcd.print(buf);
    }
    else {
      memset(buf, 0, sizeof(buf));
      if ( MidiTimeCode::getMode() == MidiTimeCode::SynchroClockMaster || MidiTimeCode::getMode() == MidiTimeCode::SynchroClockSlave) {
        sprintf(&buf[strlen(buf)], "%3dBPM", bpm);
        for (byte i = 0; i < 10; i++)
          if (MTC.isPlaying())
            buf[6 + i] = (MTC.getBeat() == i) ? '>' : ' ';
          else
            buf[6 + i] = (MTC.getBeat() == i) ? '.' : ' ';
      }
      else if ( MidiTimeCode::getMode() == MidiTimeCode::SynchroMTCMaster || MidiTimeCode::getMode() == MidiTimeCode::SynchroMTCSlave) {
        sprintf(&buf[strlen(buf)], "MTC  %02d:%02d:%02d:%02d", MTC.getHours(), MTC.getMinutes(), MTC.getSeconds(), MTC.getFrames());
      }
      /*
        if (playing) {
        for (byte i = 0; i < 10; i++)
          buf[6 + i] = ((beat % 4) == i) && ((blinkCount % 24) < 12) ? '>' : ' ';
        }
        else
        for (byte i = 6; i < LCD_COLS; i++)
          buf[i] = ' ';
      */
      /*
        for (byte i = 0; i < PEDALS; i++) {
        //buf[i] = foot_char(i);
        buf[i] = ' ';
        }
      */
      if (force || strcmp(screen1, buf) != 0) {
        memset(screen1, 0, LCD_COLS + 1);
        strncpy(screen1, buf, LCD_COLS);
        lcd.setCursor(0, 0);
        lcd.print(buf);
      }
    }
    // Line 2
    memset(buf, 0, sizeof(buf));
    //if (playing)
    //  sprintf(&buf[strlen(buf)], "BPM%3d", bpm / 10);
    //else
    sprintf(&buf[strlen(buf)], "Bank%2d", currentBank + 1);
    //sprintf(&buf[strlen(buf)], "%2x%2x%2x", currentBank + 1, banks[currentBank][lastUsedSwitch].midiChannel, banks[currentBank][lastUsedSwitch].midiCode);
    if (lastUsedPedal >= 0 && lastUsedPedal < PEDALS) {
      strncpy(&buf[strlen(buf)], &bar2[0], map(pedals[lastUsedPedal].pedalValue[0], 0, MIDI_RESOLUTION - 1, 0, 10));
      strncpy(&buf[strlen(buf)], "          ", 10 - map(pedals[lastUsedPedal].pedalValue[0], 0, MIDI_RESOLUTION - 1, 0, 10));
    }
    if (force || strcmp(screen2, buf) != 0) {     // do not update if not changed
      memset(screen2, 0, LCD_COLS + 1);
      strncpy(screen2, buf, LCD_COLS);
      lcd.setCursor(0, 1);
      lcd.print(buf);
    }
    if (selectBank) {
      lcd.setCursor(5, 1);
      lcd.cursor();
    }
    else
      lcd.noCursor();
  }
}

