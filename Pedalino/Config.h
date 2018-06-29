#include "Pedalino.h"

#define EEPROM_VERSION    4     // Increment each time you change the eeprom structure

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
          banks[b][p] = { PED_PROGRAM_CHANGE,    // MIDI message
                          b / 4 + 1,             // MIDI channel
                          b / 4 * 16 + p         // MIDI code
                      };
          break;

        case 1:
          banks[b][p] = { PED_CONTROL_CHANGE,    // MIDI message
                          b / 4 + 1,             // MIDI channel
                          b / 4 * 16 + p         // MIDI code
                      };
          break;

        case 2:
          banks[b][p] = { PED_NOTE_ON_OFF,       // MIDI message
                          b / 4 + 1,             // MIDI channel
                          b / 4 * 16 + p + 24    // MIDI code
                      };
          break;

        case 3:
          banks[b][p] = { PED_PITCH_BEND,        // MIDI message
                          b / 4 + 1,             // MIDI channel
                          b / 4 * 16 + p         // MIDI code
                      };
          break;
      }

  for (byte p = 0; p < PEDALS; p++)
    pedals[p] = {PED_MIDI,                  // function
                 1,                         // autosensing disabled
                 PED_MOMENTARY1,            // mode
                 PED_PRESS_1,               // press mode
                 1,                         // singles press
                 127,                       // double press
                 65,                        // long press
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

  pedals[11].function = PED_BANK_PLUS;
  pedals[11].mode     = PED_MOMENTARY2;
  pedals[12].function = PED_MENU;
  pedals[12].mode     = PED_MOMENTARY3;

  for (byte i = 0; i < INTERFACES; i++)
    interfaces[i] = {PED_ENABLE,            // MIDI IN
                     PED_ENABLE,            // MIDI OUT
                     PED_DISABLE,           // MIDI THRU
                     PED_ENABLE,            // MIDI routing
                     PED_DISABLE
                    };          // MIDI clock
}


//
//  Write current configuration to EEPROM (changes only)
//
void update_eeprom() {

  int offset = 0;

#ifdef DEBUG_PEDALINO
  Serial.print("Updating EEPROM ... ");
#endif

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

  EEPROM.put(offset, currentBank);
  offset += sizeof(byte);
  EEPROM.put(offset, currentPedal);
  offset += sizeof(byte);
  EEPROM.put(offset, currentInterface);
  offset += sizeof(byte);
  EEPROM.put(offset, currentWiFiMode);
  offset += sizeof(byte);

#ifdef DEBUG_PEDALINO
  Serial.println("end.");
#endif

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

#ifdef DEBUG_PEDALINO
  Serial.print("EEPROM signature: ");
  Serial.println(signature);
  Serial.print("EEPROM version: ");
  Serial.println(saved_version);
#endif

  if ((strcmp(signature, SIGNATURE) == 0) && (saved_version == EEPROM_VERSION)) {

#ifdef DEBUG_PEDALINO
    Serial.print("Reading EEPROM ... ");
#endif

    for (byte b = 0; b < BANKS; b++)
      for (byte p = 0; p < PEDALS; p++) {
        EEPROM.get(offset, banks[b][p].midiMessage);
        offset += sizeof(byte);
        EEPROM.get(offset, banks[b][p].midiChannel);
        offset += sizeof(byte);
        EEPROM.get(offset, banks[b][p].midiCode);
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

    EEPROM.get(offset, currentBank);
    offset += sizeof(byte);
    EEPROM.get(offset, currentPedal);
    offset += sizeof(byte);
    EEPROM.get(offset, currentInterface);
    offset += sizeof(byte);
    EEPROM.get(offset, currentWiFiMode);
    offset += sizeof(byte);

#ifdef DEBUG_PEDALINO
    Serial.println("end.");
#endif

  }
}

