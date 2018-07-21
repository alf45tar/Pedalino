//
//  Autosensing setup
//
void autosensing_setup()
{
  int tip;    // tip connected to an input digital pin 23, 25, ... 53 with internal pull-up resistor
  int ring;   // ring connected to an input analog pin A0, A1, ... A15
  /*        */// sleeve connected to GND
  int ring_min;
  int ring_max;
  Bounce debouncer;

  return;

  DPRINTLN("Pedal autosensing...");

  for (byte p = 0; p < PEDALS; p++) {
    pinMode(PIN_D(p), INPUT_PULLUP);
    if (pedals[p].autoSensing) {
      debouncer.attach(PIN_D(p));
      debouncer.interval(DEBOUNCE_INTERVAL);
      debouncer.update();
      tip = debouncer.read();

      DPRINT("Pedal ");
      if (p < 9) DPRINT(" ");
      DPRINT(p + 1);
      DPRINT("   Tip Pin ");
      DPRINT(PIN_D(p));
      DPRINT(" ");
      switch (tip) {
        case LOW:
          DPRINT("LOW ");
          break;
        case HIGH:
          DPRINT("HIGH");
          break;
      }
      DPRINT("    Ring Pin A");
      DPRINT(p);
      if (p < 10) DPRINT(" ");
      DPRINT(" ");

      ring_min = ADC_RESOLUTION;
      ring_max = 0;
      for (int i = 0; i < 10; i++) {
        ring = analogRead(PIN_A(p));
        ring_min = min(ring, ring_min);
        ring_max = max(ring, ring_max);

        DPRINT(ring);
        DPRINT(" ");

      }
      if ((ring_max - ring_min) > 1) {
        if (tip == LOW) {
          // tip connected to GND
          // switch between tip and ring normally closed
          pedals[p].mode = PED_MOMENTARY1;
          pedals[p].invertPolarity = true;
          DPRINTLN(" MOMENTARY POLARITY-");
        }
        else {
          // not connected
          pedals[p].mode = PED_MOMENTARY1;
          pedals[p].invertPolarity = false;
          DPRINTLN(" FLOATING PIN - NOT CONNECTED ");
        }
      }
      else if (ring <= 1) {
        // ring connected to sleeve (GND)
        // switch between tip and ring
        pedals[p].mode = PED_MOMENTARY1;
        if (tip == LOW) pedals[p].invertPolarity = true; // switch normally closed
        DPRINT(" MOMENTARY");
        if (pedals[p].invertPolarity) DPRINT(" POLARITY-");
        DPRINTLN("");
      }
      else if (ring > 0) {
        // analog
        pedals[p].mode = PED_ANALOG;
        pedals[p].invertPolarity = true;
        // inititalize continuos calibration
        pedals[p].expZero = ADC_RESOLUTION - 1;
        pedals[p].expMax = 0;
        DPRINTLN(" ANALOG POLARITY-");
      }
    }
    else {
      DPRINT("Pedal ");
      if (p < 9) DPRINT(" ");
      DPRINT(p + 1);
      DPRINTLN("   autosensing disabled");
    }
  }
  DPRINTLN("");
}

byte map_digital(byte p, byte value)
{
  p = constrain(p, 0, PEDALS - 1);
  return value;
}

unsigned int map_analog(byte p, unsigned int value)
{
  p = constrain(p, 0, PEDALS - 1);
  value = constrain(value, pedals[p].expZero, pedals[p].expMax);                  // make sure that the analog value is between the minimum and maximum value
  value = map(value, pedals[p].expZero, pedals[p].expMax, 0, ADC_RESOLUTION - 1); // map the value from [minimumValue, maximumValue] to [0, 1023]
  switch (pedals[p].mapFunction) {
    case PED_LINEAR:
      break;
    case PED_LOG:
      value = round(log(value + 1) * 147.61);             // y=log(x+1)/log(1023)*1023
      break;
    case PED_ANTILOG:
      value = round((exp(value / 511.5) - 1) * 160.12);   // y=[e^(2*x/1023)-1]/[e^2-1]*1023
      break;
  }
  return value;
}

//
//  Create new MIDI controllers setup
//
void controller_setup()
{
  // Delete previous setup
  for (byte i = 0; i < PEDALS; i++) {
    delete pedals[i].debouncer[0];
    delete pedals[i].debouncer[1];
    delete pedals[i].footSwitch[0];
    delete pedals[i].footSwitch[1];
    delete pedals[i].analogPedal;
  }

  lastUsedSwitch = 0xFF;
  lastUsedPedal  = 0xFF;

  DPRINT("MIDI Interface ");
  switch (currentInterface) {
    case PED_USBMIDI:
      DPRINTLN("USB");
      break;
    case PED_LEGACYMIDI:
      DPRINTLN("Legacy MIDI");
      break;
    case PED_APPLEMIDI:
      DPRINTLN("AppleMIDI");
      break;
    case PED_BLUETOOTHMIDI:
      DPRINTLN("Bluetooth");
      break;
  }
  DPRINT("Bank ");
  DPRINTLN(currentBank + 1);

  // Build new MIDI controllers setup
  for (byte i = 0; i < PEDALS; i++) {
    DPRINT("Pedal ");
    if (i < 9) DPRINT(" ");
    DPRINT(i + 1);
    DPRINT("     ");
    switch (pedals[i].function) {
      case PED_MIDI:        DPRINT("MIDI      "); break;
      case PED_BANK_PLUS:   DPRINT("BANK_PLUS "); break;
      case PED_BANK_MINUS:  DPRINT("BANK_MINUS"); break;
      case PED_START:       DPRINT("START     "); break;
      case PED_STOP:        DPRINT("STOP      "); break;
      case PED_TAP:         DPRINT("TAP       "); break;
      case PED_MENU:        DPRINT("MENU      "); break;
      case PED_CONFIRM:     DPRINT("CONFIRM   "); break;
      case PED_ESCAPE:      DPRINT("ESCAPE    "); break;
      case PED_NEXT:        DPRINT("NEXT      "); break;
      case PED_PREVIOUS:    DPRINT("PREVIOUS  "); break;
    }
    DPRINT("   ");
    switch (pedals[i].mode) {
      case PED_MOMENTARY1:  DPRINT("MOMENTARY1"); break;
      case PED_MOMENTARY2:  DPRINT("MOMENTARY2"); break;
      case PED_MOMENTARY3:  DPRINT("MOMENTARY3"); break;
      case PED_LATCH1:      DPRINT("LATCH1    "); break;
      case PED_LATCH2:      DPRINT("LATCH2    "); break;
      case PED_ANALOG:      DPRINT("ANALOG    "); break;
      case PED_JOG_WHEEL:   DPRINT("JOG_WHEEL "); break;
    }
    DPRINT("   ");
    switch (pedals[i].pressMode) {
      case PED_PRESS_1:     DPRINT("PRESS_1    "); break;
      case PED_PRESS_2:     DPRINT("PRESS_2    "); break;
      case PED_PRESS_L:     DPRINT("PRESS_L    "); break;
      case PED_PRESS_1_2:   DPRINT("PRESS_1_2  "); break;
      case PED_PRESS_1_L:   DPRINT("PRESS_1_L  "); break;
      case PED_PRESS_1_2_L: DPRINT("PRESS_1_2_L"); break;
      case PED_PRESS_2_L:   DPRINT("PRESS_2_L  "); break;
    }
    DPRINT("   ");
    switch (pedals[i].invertPolarity) {
      case false:           DPRINT("POLARITY+"); break;
      case true:            DPRINT("POLARITY-"); break;
    }
    DPRINT("   ");
    switch (banks[currentBank][i].midiMessage) {
      case PED_PROGRAM_CHANGE:
        DPRINT("PROGRAM_CHANGE ");
        DPRINT(banks[currentBank][i].midiCode);
        break;
      case PED_CONTROL_CHANGE:
        DPRINT("CONTROL_CHANGE ");
        DPRINT(banks[currentBank][i].midiCode);
        break;
      case PED_NOTE_ON_OFF:
        DPRINT("NOTE_ON_OFF    ");
        DPRINT(banks[currentBank][i].midiCode);
        break;
      case PED_PITCH_BEND:
        DPRINT("PITCH_BEND     ");
        break;
    }
    DPRINT("   Channel ");
    DPRINT(banks[currentBank][i].midiChannel);

    switch (pedals[i].mode) {

      case PED_MOMENTARY1:
      case PED_MOMENTARY2:
      case PED_MOMENTARY3:
      case PED_LATCH1:
      case PED_LATCH2:

        unsigned int input;
        unsigned int value;
        for (byte p = 0; p < 2; p++) {
          if (pedals[i].mode == PED_MOMENTARY1 && p == 1) continue;
          if (pedals[i].mode == PED_LATCH1     && p == 1) continue;

          pedals[i].debouncer[p] = new Bounce();
          switch (p) {
            case 0:
              // Setup the button with an internal pull-up
              pinMode(PIN_D(i), INPUT_PULLUP);

              // After setting up the button, setup the Bounce instance
              pedals[i].debouncer[0]->attach(PIN_D(i));
              DPRINT("   Pin D");
              DPRINT(PIN_D(i));
              break;
            case 1:
              // Setup the button with an internal pull-up
              pinMode(PIN_A(i), INPUT_PULLUP);

              // After setting up the button, setup the Bounce instance
              pedals[i].debouncer[1]->attach(PIN_A(i));
              DPRINT(" A");
              DPRINT(i);
              break;
          }
          pedals[i].debouncer[p]->interval(DEBOUNCE_INTERVAL);
          pedals[i].debouncer[p]->update();
          input = pedals[i].debouncer[p]->read();                                 // reads the updated pin state
          if (pedals[i].invertPolarity) input = (input == LOW) ? HIGH : LOW;      // invert the value
          value = map_digital(i, input);                                          // apply the digital map function to the value
          pedals[i].pedalValue[p] = value;
          pedals[i].lastUpdate[p] = millis();

          if (pedals[i].mode == PED_MOMENTARY1 || pedals[i].mode == PED_MOMENTARY2 || pedals[i].mode == PED_MOMENTARY3)
          {
            switch (p) {
              case 0:
                pedals[i].footSwitch[0] = new MD_UISwitch_Digital(PIN_D(i), pedals[i].invertPolarity ? HIGH : LOW);
                break;
              case 1:
                pedals[i].footSwitch[1] = new MD_UISwitch_Digital(PIN_A(i), pedals[i].invertPolarity ? HIGH : LOW);
                break;
            }
            pedals[i].footSwitch[p]->begin();
            pedals[i].footSwitch[p]->setDebounceTime(50);
            if (pedals[i].function == PED_MIDI) {
              switch (pedals[i].pressMode) {
                case PED_PRESS_1:
                  pedals[i].footSwitch[p]->enableDoublePress(false);
                  pedals[i].footSwitch[p]->enableLongPress(false);
                  break;
                case PED_PRESS_2:
                case PED_PRESS_1_2:
                  pedals[i].footSwitch[p]->enableDoublePress(true);
                  pedals[i].footSwitch[p]->enableLongPress(false);
                  break;
                case PED_PRESS_L:
                case PED_PRESS_1_L:
                  pedals[i].footSwitch[p]->enableDoublePress(false);
                  pedals[i].footSwitch[p]->enableLongPress(true);
                  break;
                case PED_PRESS_1_2_L:
                case PED_PRESS_2_L:
                  pedals[i].footSwitch[p]->enableDoublePress(true);
                  pedals[i].footSwitch[p]->enableLongPress(true);
                  break;
                  pedals[i].footSwitch[p]->enableDoublePress(true);
                  pedals[i].footSwitch[p]->enableLongPress(true);
                  break;
              }
              pedals[i].footSwitch[p]->setDoublePressTime(300);
              pedals[i].footSwitch[p]->setLongPressTime(500);
              pedals[i].footSwitch[p]->enableRepeat(false);
            }
            else
            {
              pedals[i].footSwitch[p]->setDoublePressTime(300);
              pedals[i].footSwitch[p]->setLongPressTime(500);
              pedals[i].footSwitch[p]->setRepeatTime(500);
              pedals[i].footSwitch[p]->enableRepeatResult(true);
            }
          }
        }
        break;

      case PED_ANALOG:
        pinMode(PIN_D(i), OUTPUT);
        digitalWrite(PIN_D(i), HIGH);
        if (pedals[i].function == PED_MIDI) {
          pedals[i].analogPedal = new ResponsiveAnalogRead(PIN_A(i), true);
          pedals[i].analogPedal->setActivityThreshold(6.0);
          pedals[i].analogPedal->setAnalogResolution(MIDI_RESOLUTION);        // 7-bit MIDI resolution
          pedals[i].analogPedal->enableEdgeSnap();                            // ensures that values at the edges of the spectrum can be easily reached when sleep is enabled
          if (lastUsedPedal == 0xFF) lastUsedPedal = i;
        }
        else
          pedals[i].footSwitch[0] = new MD_UISwitch_Analog(PIN_A(i), kt, ARRAY_SIZE(kt));
        break;

      case PED_JOG_WHEEL:
        break;
    }
    DPRINTLN("");
  }
}


void midi_send(byte message, byte code, byte value, byte channel, bool on_off = true )
{
  switch (message) {

    case PED_NOTE_ON_OFF:

      if (on_off && value > 0) {
#ifdef DEBUG_PEDALINO
        DPRINT("     NOTE ON     Note ");
        DPRINT(code);
        DPRINT("     Velocity ");
        DPRINT(value);
        DPRINT("     Channel ");
        DPRINT(channel);
#else
        if (interfaces[PED_USBMIDI].midiOut)    USB_MIDI.sendNoteOn(code, value, channel);
#endif
        if (interfaces[PED_LEGACYMIDI].midiOut) DIN_MIDI.sendNoteOn(code, value, channel);
        if (interfaces[PED_APPLEMIDI].midiOut)  RTP_MIDI.sendNoteOn(code, value, channel);
      }
      else {
#ifdef DEBUG_PEDALINO
        DPRINT("     NOTE OFF    Note ");
        DPRINT(code);
        DPRINT("     Velocity ");
        DPRINT(value);
        DPRINT("     Channel ");
        DPRINT(channel);
#else
        if (interfaces[PED_USBMIDI].midiOut)    USB_MIDI.sendNoteOff(code, value, channel);
#endif
        if (interfaces[PED_LEGACYMIDI].midiOut) DIN_MIDI.sendNoteOff(code, value, channel);
        if (interfaces[PED_APPLEMIDI].midiOut)  RTP_MIDI.sendNoteOff(code, value, channel);
      }
      break;

    case PED_CONTROL_CHANGE:

      if (on_off) {
#ifdef DEBUG_PEDALINO
        DPRINT("     CONTROL CHANGE     Code ");
        DPRINT(code);
        DPRINT("     Value ");
        DPRINT(value);
        DPRINT("     Channel ");
        DPRINT(channel);
#else
        if (interfaces[PED_USBMIDI].midiOut)    USB_MIDI.sendControlChange(code, value, channel);
#endif
        if (interfaces[PED_LEGACYMIDI].midiOut) DIN_MIDI.sendControlChange(code, value, channel);
        if (interfaces[PED_APPLEMIDI].midiOut)  RTP_MIDI.sendControlChange(code, value, channel);
      }
      break;

    case PED_PROGRAM_CHANGE:

      if (on_off) {
#ifdef DEBUG_PEDALINO
        DPRINT("     PROGRAM CHANGE     Program ");
        DPRINT(code);
        DPRINT("     Channel ");
        DPRINT(channel);
#else
        if (interfaces[PED_USBMIDI].midiOut)    USB_MIDI.sendProgramChange(code, channel);
#endif
        if (interfaces[PED_LEGACYMIDI].midiOut) DIN_MIDI.sendProgramChange(code, channel);
        if (interfaces[PED_APPLEMIDI].midiOut)  RTP_MIDI.sendProgramChange(code, channel);
      }
      break;

    case PED_PITCH_BEND:

      if (on_off) {
        int bend = map(value, 0, 127, - 8192, 8191);
#ifdef DEBUG_PEDALINO
        DPRINT("     PITCH BEND     Value ");
        DPRINT(bend);
        DPRINT("     Channel ");
        DPRINT(channel);
#else
        if (interfaces[PED_USBMIDI].midiOut)    USB_MIDI.sendPitchBend(bend, channel);
#endif
        if (interfaces[PED_LEGACYMIDI].midiOut) DIN_MIDI.sendPitchBend(bend, channel);
        if (interfaces[PED_APPLEMIDI].midiOut)  RTP_MIDI.sendPitchBend(bend, channel);
      }
      break;
  }
}

//
//  MIDI messages refresh
//
void midi_refresh()
{
  MD_UISwitch::keyResult_t  k, k1, k2;
  bool                      state1, state2;
  unsigned int              input;
  unsigned int              value;
  byte                      b;

  for (byte i = 0; i < PEDALS; i++) {
    if (pedals[i].function == PED_MIDI) {
      switch (pedals[i].mode) {

        case PED_MOMENTARY1:
        case PED_MOMENTARY2:
        case PED_MOMENTARY3:
        case PED_LATCH1:
        case PED_LATCH2:

          switch (pedals[i].pressMode) {

            case PED_PRESS_1:
              state1 = false;
              state2 = false;
              if (pedals[i].debouncer[0] != nullptr) state1 = pedals[i].debouncer[0]->update();
              if (pedals[i].debouncer[1] != nullptr) state2 = pedals[i].debouncer[1]->update();
              if (state1 && state2) {                                                     // pin state changed
                input = pedals[i].debouncer[0]->read();                                   // reads the updated pin state
                if (pedals[i].invertPolarity) input = (input == LOW) ? HIGH : LOW;        // invert the value
                value = map_digital(i, input);                                            // apply the digital map function to the value

                DPRINTLN("");
                DPRINT("Pedal ");
                if (i < 9) DPRINT(" ");
                DPRINT(i + 1);
                DPRINT("   input ");
                DPRINT(input);
                DPRINT(" output ");
                DPRINT(value);

                b = (currentBank + 2) % BANKS;
                if (value == LOW)                                                         // LOW = pressed, HIGH = released
                  midi_send(banks[b][i].midiMessage,
                            banks[b][i].midiCode,
                            banks[b][i].midiValue1,
                            banks[b][i].midiChannel);
                else
                  midi_send(banks[b][i].midiMessage,
                            banks[b][i].midiCode,
                            banks[b][i].midiValue2,
                            banks[b][i].midiChannel,
                            pedals[i].mode == PED_LATCH1 || pedals[i].mode == PED_LATCH2);
                pedals[i].pedalValue[0] = value;
                pedals[i].lastUpdate[0] = millis();
                pedals[i].pedalValue[1] = pedals[i].pedalValue[0];
                pedals[i].lastUpdate[1] = pedals[i].lastUpdate[0];
                lastUsedSwitch = i;
              }
              else {
                if (state1) {                                                             // pin state changed
                  input = pedals[i].debouncer[0]->read();                                 // reads the updated pin state
                  if (pedals[i].invertPolarity) input = (input == LOW) ? HIGH : LOW;      // invert the value
                  value = map_digital(i, input);                                          // apply the digital map function to the value

                  DPRINTLN("");
                  DPRINT("Pedal ");
                  if (i < 9) DPRINT(" ");
                  DPRINT(i + 1);
                  DPRINT("   input ");
                  DPRINT(input);
                  DPRINT(" output ");
                  DPRINT(value);

                  b = currentBank;
                  if (value == LOW)                                                         // LOW = pressed, HIGH = released
                    midi_send(banks[b][i].midiMessage,
                              banks[b][i].midiCode,
                              banks[b][i].midiValue1,
                              banks[b][i].midiChannel);
                  else
                    midi_send(banks[b][i].midiMessage,
                              banks[b][i].midiCode,
                              banks[b][i].midiValue2,
                              banks[b][i].midiChannel,
                              pedals[i].mode == PED_LATCH1 || pedals[i].mode == PED_LATCH2);
                  pedals[i].pedalValue[0] = value;
                  pedals[i].lastUpdate[0] = millis();
                  lastUsedSwitch = i;
                }
                if (state2) {                                                             // pin state changed
                  input = pedals[i].debouncer[1]->read();                                 // reads the updated pin state
                  if (pedals[i].invertPolarity) input = (input == LOW) ? HIGH : LOW;      // invert the value
                  value = map_digital(i, input);                                          // apply the digital map function to the value

                  DPRINTLN("");
                  DPRINT("Pedal ");
                  if (i < 9) DPRINT(" ");
                  DPRINT(i + 1);
                  DPRINT("   input ");
                  DPRINT(input);
                  DPRINT(" output ");
                  DPRINT(value);

                  b = (currentBank + 1) % BANKS;
                  if (value == LOW)                                                         // LOW = pressed, HIGH = released
                    midi_send(banks[b][i].midiMessage,
                              banks[b][i].midiCode,
                              banks[b][i].midiValue1,
                              banks[b][i].midiChannel);
                  else
                    midi_send(banks[b][i].midiMessage,
                              banks[b][i].midiCode,
                              banks[b][i].midiValue2,
                              banks[b][i].midiChannel,
                              pedals[i].mode == PED_LATCH1 || pedals[i].mode == PED_LATCH2);
                  pedals[i].pedalValue[1] = value;
                  pedals[i].lastUpdate[1] = millis();
                  lastUsedSwitch = i;
                }
              }
              break;

            case PED_PRESS_1_2:
            case PED_PRESS_1_L:
            case PED_PRESS_1_2_L:
            case PED_PRESS_2:
            case PED_PRESS_2_L:
            case PED_PRESS_L:

              if (pedals[i].mode == PED_LATCH1 || pedals[i].mode == PED_LATCH2) break;

              pedals[i].pedalValue[0] = digitalRead(PIN_D(i));
              //pedals[i].lastUpdate[0] = millis();
              pedals[i].pedalValue[1] = digitalRead(PIN_A(i));
              //pedals[i].lastUpdate[1] = millis();

              k1 = MD_UISwitch::KEY_NULL;
              k2 = MD_UISwitch::KEY_NULL;
              if (pedals[i].footSwitch[0] != nullptr) k1 = pedals[i].footSwitch[0]->read();
              if (pedals[i].footSwitch[1] != nullptr) k2 = pedals[i].footSwitch[1]->read();

              int j = 2;
              while ( j >= 0) {
                b = (currentBank + j) % BANKS;
                switch (j) {
                  case 0: k = k1; break;
                  case 1: k = k2; break;
                  case 2: k = (k1 == k2) ? k1 : MD_UISwitch::KEY_NULL; break;
                }
                switch (k) {

                  case MD_UISwitch::KEY_PRESS:

                    DPRINTLN("");
                    DPRINT("Pedal ");
                    if (i < 9) DPRINT(" ");
                    DPRINT(i + 1);
                    DPRINT("   SINGLE PRESS ");

                    midi_send(banks[b][i].midiMessage, banks[b][i].midiCode, banks[b][i].midiValue1, banks[b][i].midiChannel);
                    midi_send(banks[b][i].midiMessage, banks[b][i].midiCode, banks[b][i].midiValue1, banks[b][i].midiChannel, false);
                    lastUsedSwitch = i;
                    break;

                  case MD_UISwitch::KEY_DPRESS:

                    DPRINTLN("");
                    DPRINT("Pedal ");
                    if (i < 9) DPRINT(" ");
                    DPRINT(i + 1);
                    DPRINT("   DOUBLE PRESS ");

                    midi_send(banks[b][i].midiMessage, banks[b][i].midiCode, banks[b][i].midiValue2, banks[b][i].midiChannel);
                    midi_send(banks[b][i].midiMessage, banks[b][i].midiCode, banks[b][i].midiValue2, banks[b][i].midiChannel, false);
                    lastUsedSwitch = i;
                    break;

                  case MD_UISwitch::KEY_LONGPRESS:

                    DPRINTLN("");
                    DPRINT("Pedal ");
                    if (i < 9) DPRINT(" ");
                    DPRINT(i + 1);
                    DPRINT("   LONG   PRESS ");

                    midi_send(banks[b][i].midiMessage, banks[b][i].midiCode, banks[b][i].midiValue3, banks[b][i].midiChannel);
                    midi_send(banks[b][i].midiMessage, banks[b][i].midiCode, banks[b][i].midiValue3, banks[b][i].midiChannel, false);
                    lastUsedSwitch = i;
                    break;
                }
                if (k1 == k2 && k1 != MD_UISwitch::KEY_NULL) j = -1;
                else j--;
              }
              break;
          }
          break;

        case PED_ANALOG:

          if (pedals[i].analogPedal == nullptr) continue;           // sanity check

          input = analogRead(PIN_A(i));                             // read the raw analog input value
          if (pedals[i].autoSensing) {                              // continuos calibration

            if (pedals[i].expZero > round(1.1 * input)) {
              DPRINT("Pedal ");
              if (i < 9) DPRINT(" ");
              DPRINT(i + 1);
              DPRINT(" calibration min ");
              DPRINT(round(1.1 * input));
              DPRINTLN("");
            }
            if (pedals[i].expMax < round(0.9 * input)) {
              DPRINT("Pedal ");
              if (i < 9) DPRINT(" ");
              DPRINT(i + 1);
              DPRINT(" calibration max ");
              DPRINT(round(0.9 * input));
              DPRINTLN("");
            }

            pedals[i].expZero = min(pedals[i].expZero, round(1.1 * input));
            pedals[i].expMax  = max(pedals[i].expMax,  round(0.9 * input));
          }
          value = map_analog(i, input);                             // apply the digital map function to the value
          if (pedals[i].invertPolarity) input = ADC_RESOLUTION - 1 - input;   // invert the scale
          value = value >> 3;                                       // map from 10-bit value [0, 1023] to the 7-bit MIDI value [0, 127]
          pedals[i].analogPedal->update(value);                     // update the responsive analog average
          if (pedals[i].analogPedal->hasChanged())                  // if the value changed since last time
          {
            value = pedals[i].analogPedal->getValue();              // get the responsive analog average value
            double velocity = ((double)value - pedals[i].pedalValue[0]) / (millis() - pedals[i].lastUpdate[0]);

            DPRINTLN("");
            DPRINT("Pedal ");
            if (i < 9) DPRINT(" ");
            DPRINT(i + 1);
            DPRINT("   input ");
            DPRINT(input);
            DPRINT(" output ");
            DPRINT(value);
            DPRINT(" velocity ");
            DPRINT(velocity);

            midi_send(banks[currentBank][i].midiMessage, banks[currentBank][i].midiCode, value, banks[currentBank][i].midiChannel);
            midi_send(banks[currentBank][i].midiMessage, banks[currentBank][i].midiCode, value, banks[currentBank][i].midiChannel, false);
            pedals[i].pedalValue[0] = value;
            pedals[i].lastUpdate[0] = millis();
            lastUsedPedal = i;
          }
          break;
      }
    }
  }
}

//
// Calibration for analog controllers
//
void calibrate()
{
  unsigned long start = millis();

  // Clear display
  lcd.clear();
  lcd.setCursor(0, 0);

  // Display countdown bar
  for (int i = 1; i <= LCD_COLS; i++)
    lcd.print(char(B10100101));

  // Move expression pedal from min to max during CALIBRATION_DURATION
  pedals[currentPedal].expZero = ADC_RESOLUTION - 1;
  pedals[currentPedal].expMax = 0;

  while (millis() - start < CALIBRATION_DURATION) {

    // Read the current value and update min and max
    int ax = analogRead(PIN_A(currentPedal));
    pedals[currentPedal].expZero = min( pedals[currentPedal].expZero, ax + 20);
    pedals[currentPedal].expMax  = max( pedals[currentPedal].expMax, ax - 20);

    // Update countdown bar (1st row)
    lcd.setCursor(LCD_COLS - map(millis() - start, 0, CALIBRATION_DURATION, 0, LCD_COLS), 0);
    lcd.print(" ");

    // Update value bar (2nd row)
    lcd.setCursor(0, 1);
    lcd.print(pedals[currentPedal].expZero);
    for (int i = 1; i < LCD_COLS - floor(log10(pedals[currentPedal].expZero + 1)) - floor(log10(pedals[currentPedal].expMax + 1)) - 1; i++)
      lcd.print(" ");
    lcd.print(pedals[currentPedal].expMax);
  }
}

