**Status:** Working but under active development

# Pedalino
High customizable MIDI controller for guitarists and more.

## Main features
- Up to 16 controllers. Support for digital foot switches (momentary or latch), analog expression pedals and jog wheels (rotary encoders).
- MIDI output via USB or classic MIDI OUT connector
- 10 banks
- MIDI channel, MIDI note, MIDI control code, MIDI program change can be configured by each pedal.
- IR Remote control
- Remote control via Bluetooth
- Each parameters can be configured by menu. Configuration is saved into EEPROM. Non need to re-compile the program. 

## Requirements
- Arduino Mega 2560 R3 or equivalent (a lite version for Arduino Uno R3 is on the way)
- 16x2 LiquidCrystal displays (LCDs) based on the Hitachi HD44780 (or a compatible) chipset
- IR Receiver module (like KY-022 or equivalent)
- [New Liquid Crystal](https://bitbucket.org/fmalpartida/new-liquidcrystal/wiki/Home) library (may works also with standard LiquidCrystal library)
- Customized [MIDI_Controller](https://github.com/tttapa/MIDI_controller) library
- [MD_Menu](https://github.com/MajicDesigns/MD_Menu) library
- [IRRemore](https://github.com/z3t0/Arduino-IRremote) library) library


## Prototype

Potenziometers and switches can be replaced with any commercial footswitch or expression pedal for musical instruments.

![Fritzing](https://github.com/alf45tar/Pedalino/blob/master/Pedalino_bb.svg)
