/*  __________           .___      .__  .__                   ___ ________________    ___
 *  \______   \ ____   __| _/____  |  | |__| ____   ____     /  / \__    ___/     \   \  \   
 *   |     ___// __ \ / __ |\__  \ |  | |  |/    \ /  _ \   /  /    |    | /  \ /  \   \  \  
 *   |    |   \  ___// /_/ | / __ \|  |_|  |   |  (  <_> ) (  (     |    |/    Y    \   )  )
 *   |____|    \___  >____ |(____  /____/__|___|  /\____/   \  \    |____|\____|__  /  /  /
 *                 \/     \/     \/             \/           \__\                 \/  /__/
 *                                                                (c) 2018 alf45star
 *                                                        https://github.com/alf45tar/Pedalino
 */


#define SIGNATURE         "Pedalino(TM)"
#define EEPROM_VERSION    4               // Increment each time you change the eeprom structure

//
//  Load factory deafult value for banks, pedals and interfaces
//
void load_factory_default()
{
  for (byte b = 0; b < BANKS; b++)
    for (byte p = 0; p < PEDALS; p++)
      switch (b % 4)
      {
        case 0:
          banks[b][p] = { PED_PROGRAM_CHANGE,             // MIDI message
                          (byte)(b / 4 + 1),              // MIDI channel
                          (byte)(b / 4 * 16 + p),         // MIDI code
                          127,
                          0,
                          65
                      };
          break;

        case 1:
          banks[b][p] = { PED_CONTROL_CHANGE,             // MIDI message
                          (byte)(b / 4 + 1),              // MIDI channel
                          (byte)(b / 4 * 16 + p),          // MIDI code
                          127,
                          0,
                          65
                      };
          break;

        case 2:
          banks[b][p] = { PED_NOTE_ON_OFF,                // MIDI message
                          (byte)(b / 4 + 1),              // MIDI channel
                          (byte)(b / 4 * 16 + p + 24),    // MIDI code
                          127,
                          0,
                          65
                      };
          break;

        case 3:
          banks[b][p] = { PED_PITCH_BEND,                  // MIDI message
                          (byte)(b / 4 + 1),               // MIDI channel
                          (byte)(b / 4 * 16 + p),          // MIDI code
                          127,
                          0,
                          65
                      };
          break;
      }

  for (byte p = 0; p < PEDALS; p++)
    pedals[p] = {PED_MIDI,                  // function
                 1,                         // autosensing disabled
                 PED_MOMENTARY1,            // mode
                 PED_PRESS_1,               // press mode
                 0,                         // invert polarity disabled
                 0,                         // map function
                 50,                        // expression pedal zero
                 930,                       // expression pedal max
                 0,                         // last state of switch 1
                 0,                         // last state of switch 2
                 millis(),                  // last time switch 1 status changed
                 millis(),                  // last time switch 2 status changed
                 nullptr, nullptr, nullptr, nullptr, nullptr
                };

  pedals[7].function  = PED_START;
  pedals[7].mode      = PED_MOMENTARY1;
  pedals[8].function  = PED_STOP;
  pedals[8].mode      = PED_MOMENTARY1;
  pedals[9].function  = PED_CONTINUE;
  pedals[9].mode      = PED_MOMENTARY1;
  pedals[10].function = PED_TAP;
  pedals[10].mode     = PED_MOMENTARY1;
  pedals[11].function = PED_BANK_PLUS;
  pedals[11].mode     = PED_MOMENTARY2;
  pedals[12].function = PED_MENU;
  pedals[12].mode     = PED_MOMENTARY3;
  pedals[15].function = PED_MIDI;
  pedals[15].mode     = PED_ANALOG;

  for (byte i = 0; i < INTERFACES; i++)
    interfaces[i] = {PED_ENABLE,            // MIDI IN
                     PED_ENABLE,            // MIDI OUT
                     PED_DISABLE,           // MIDI THRU
                     PED_ENABLE,            // MIDI routing
                     PED_DISABLE            // MIDI clock
                    };

  for (byte c = 0; c < IR_CUSTOM_CODES; c++)
    ircustomcode[c] = 0xFFFFFE;
}

//
//  Write current configuration to EEPROM (changes only)
//
void update_eeprom() {

  int offset = 0;

  DPRINTLNF("Updating EEPROM ... ");

  EEPROM.put(offset, SIGNATURE);
  offset += sizeof(SIGNATURE);
  EEPROM.put(offset, EEPROM_VERSION);
  offset += sizeof(byte);

  for (byte b = 0; b < BANKS; b++)
    for (byte p = 0; p < PEDALS; p++) {
      EEPROM.put(offset, banks[b][p].midiMessage);
      offset += sizeof(byte);
      EEPROM.put(offset, banks[b][p].midiChannel);
      offset += sizeof(byte);
      EEPROM.put(offset, banks[b][p].midiCode);
      offset += sizeof(byte);
      EEPROM.put(offset, banks[b][p].midiValue1);
      offset += sizeof(byte);
      EEPROM.put(offset, banks[b][p].midiValue2);
      offset += sizeof(byte);
      EEPROM.put(offset, banks[b][p].midiValue3);
      offset += sizeof(byte);
    }

  for (byte p = 0; p < PEDALS; p++) {
    EEPROM.put(offset, pedals[p].function);
    offset += sizeof(byte);
    EEPROM.put(offset, pedals[p].autoSensing);
    offset += sizeof(byte);
    EEPROM.put(offset, pedals[p].mode);
    offset += sizeof(byte);
    EEPROM.put(offset, pedals[p].pressMode);
    offset += sizeof(byte);
    EEPROM.put(offset, pedals[p].invertPolarity);
    offset += sizeof(byte);
    EEPROM.put(offset, pedals[p].mapFunction);
    offset += sizeof(byte);
    EEPROM.put(offset, pedals[p].expZero);
    offset += sizeof(int);
    EEPROM.put(offset, pedals[p].expMax);
    offset += sizeof(int);
  }

  for (byte i = 0; i < INTERFACES; i++) {
    EEPROM.put(offset, interfaces[i].midiIn);
    offset += sizeof(byte);
    EEPROM.put(offset, interfaces[i].midiOut);
    offset += sizeof(byte);
    EEPROM.put(offset, interfaces[i].midiThru);
    offset += sizeof(byte);
    EEPROM.put(offset, interfaces[i].midiRouting);
    offset += sizeof(byte);
    EEPROM.put(offset, interfaces[i].midiClock);
    offset += sizeof(byte);
  }

  DPRINTF("[0x");
  DPRINT2(offset, HEX);
  DPRINTF("] ");
  EEPROM.put(offset, currentBank);
  offset += sizeof(byte);
  DPRINTF("Current bank:      0x");
  DPRINTLN2(currentBank, HEX);

  DPRINTF("[0x");
  DPRINT2(offset, HEX);
  DPRINTF("] ");
  EEPROM.put(offset, currentPedal);
  offset += sizeof(byte);
  DPRINTF("Current pedal:     0x");
  DPRINTLN2(currentPedal, HEX);

  DPRINTF("[0x");
  DPRINT2(offset, HEX);
  DPRINTF("] ");
  EEPROM.put(offset, currentInterface);
  offset += sizeof(byte);
  DPRINTF("Current interface: 0x");
  DPRINTLN2(currentInterface, HEX);

  DPRINTF("[0x");
  DPRINT2(offset, HEX);
  DPRINTF("] ");
  EEPROM.put(offset, currentMidiTimeCode);
  offset += sizeof(byte);
  DPRINTF("Current MTC:       0x");
  DPRINTLN2(currentMidiTimeCode, HEX);

  DPRINTF("[0x");
  DPRINT2(offset, HEX);
  DPRINTF("] ");
  EEPROM.put(offset, backlight);
  offset += sizeof(byte);
  DPRINTF("Backlight:         0x");
  DPRINTLN2(backlight, HEX);

  for (byte c = 0; c < IR_CUSTOM_CODES; c++) {
    DPRINTF("[0x");
    DPRINT2(offset, HEX);
    DPRINTF("] ");
    EEPROM.put(offset, ircustomcode[c]);
    offset += sizeof(unsigned long);
    DPRINTF("IR Code ");
    if (c < 10) DPRINTF(" ");
    DPRINT(c);
    DPRINTF(":        0x");
    DPRINTLN2(ircustomcode[c], HEX);
  }

  DPRINT(offset);
  DPRINTF(" bytes of ");
  DPRINT(EEPROM.length());
  DPRINTLNF(" updated.");

  blynk_refresh();
}


//
//  Read configuration from EEPROM
//
void read_eeprom() {

  int offset = 0;
  char signature[LCD_COLS + 1];
  byte saved_version;

  load_factory_default();

  EEPROM.get(offset, signature);
  offset += sizeof(SIGNATURE);
  EEPROM.get(offset, saved_version);
  offset += sizeof(byte);

  DPRINTF("EEPROM signature: ");
  DPRINTLN(signature);
  DPRINTF("EEPROM version  : ");
  DPRINTLN(saved_version);

  if ((strcmp(signature, SIGNATURE) == 0) && (saved_version == EEPROM_VERSION)) {

    DPRINTLNF("Reading EEPROM ... ");

    for (byte b = 0; b < BANKS; b++)
      for (byte p = 0; p < PEDALS; p++) {
        EEPROM.get(offset, banks[b][p].midiMessage);
        offset += sizeof(byte);
        EEPROM.get(offset, banks[b][p].midiChannel);
        offset += sizeof(byte);
        EEPROM.get(offset, banks[b][p].midiCode);
        offset += sizeof(byte);
        EEPROM.get(offset, banks[b][p].midiValue1);
        offset += sizeof(byte);
        EEPROM.get(offset, banks[b][p].midiValue2);
        offset += sizeof(byte);
        EEPROM.get(offset, banks[b][p].midiValue3);
        offset += sizeof(byte);
      }

    for (byte p = 0; p < PEDALS; p++) {
      EEPROM.get(offset, pedals[p].function);
      offset += sizeof(byte);
      EEPROM.get(offset, pedals[p].autoSensing);
      offset += sizeof(byte);
      EEPROM.get(offset, pedals[p].mode);
      offset += sizeof(byte);
      EEPROM.get(offset, pedals[p].pressMode);
      offset += sizeof(byte);
      EEPROM.get(offset, pedals[p].invertPolarity);
      offset += sizeof(byte);
      EEPROM.get(offset, pedals[p].mapFunction);
      offset += sizeof(byte);
      EEPROM.get(offset, pedals[p].expZero);
      offset += sizeof(int);
      EEPROM.get(offset, pedals[p].expMax);
      offset += sizeof(int);
    }

    for (byte i = 0; i < INTERFACES; i++) {
      EEPROM.get(offset, interfaces[i].midiIn);
      offset += sizeof(byte);
      EEPROM.get(offset, interfaces[i].midiOut);
      offset += sizeof(byte);
      EEPROM.get(offset, interfaces[i].midiThru);
      offset += sizeof(byte);
      EEPROM.get(offset, interfaces[i].midiRouting);
      offset += sizeof(byte);
      EEPROM.get(offset, interfaces[i].midiClock);
      offset += sizeof(byte);
    }

    DPRINTF("[0x");
    DPRINT2(offset, HEX);
    DPRINTF("] ");
    EEPROM.get(offset, currentBank);
    currentBank = constrain(currentBank, 0, BANKS - 1);
    offset += sizeof(byte);
    DPRINTF("Current bank:      0x");
    DPRINTLN2(currentBank, HEX);

    DPRINTF("[0x");
    DPRINT2(offset, HEX);
    DPRINTF("] ");
    EEPROM.get(offset, currentPedal);
    currentPedal = constrain(currentPedal, 0, PEDALS - 1);
    offset += sizeof(byte);
    DPRINTF("Current pedal:     0x");
    DPRINTLN2(currentPedal, HEX);

    DPRINTF("[0x");
    DPRINT2(offset, HEX);
    DPRINTF("] ");
    EEPROM.get(offset, currentInterface);
    currentInterface = constrain(currentInterface, 0, INTERFACES - 1);
    offset += sizeof(byte);
    DPRINTF("Current interface: 0x");
    DPRINTLN2(currentInterface, HEX);

    DPRINTF("[0x");
    DPRINT2(offset, HEX);
    DPRINTF("] ");
    EEPROM.get(offset, currentMidiTimeCode);
    offset += sizeof(byte);
    DPRINTF("Current MTC:       0x");
    DPRINTLN2(currentMidiTimeCode, HEX);

    DPRINTF("[0x");
    DPRINT2(offset, HEX);
    DPRINTF("] ");
    EEPROM.get(offset, backlight);
    offset += sizeof(byte);
    DPRINTF("Backlight:         0x");
    DPRINTLN2(backlight, HEX);

    for (byte c = 0; c < IR_CUSTOM_CODES; c++) {
      DPRINTF("[0x");
      DPRINT2(offset, HEX);
      DPRINTF("] ");
      EEPROM.get(offset, ircustomcode[c]);
      offset += sizeof(unsigned long);
      DPRINTF("IR Code ");
      if (c < 10) DPRINTF(" ");
      DPRINT(c);
      DPRINTF(":        0x");
      DPRINTLN2(ircustomcode[c], HEX);
    }

    DPRINT(offset);
    DPRINTF(" bytes of ");
    DPRINT(EEPROM.length());
    DPRINTLNF(" read.");

    blynk_refresh();
  }
}

