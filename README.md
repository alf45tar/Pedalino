**Status:** Working but under active development

# Pedalino
High customizable MIDI controller for guitarists and more.

## Features
- Support for digital foot switches (momentary or latch), analog expression pedals and jog wheels (rotary encoders)
- Up to 16 controllers in any quantity or order (up to 8 for lite version)
- MIDI output via USB MIDI and classic MIDI OUT connector (only one in lite version)
- Send Program Change, Control Code and Note On/Off MIDI events
- 10 banks
- MIDI channel, MIDI note, MIDI control code, MIDI program change can be configured by each pedal and by each bank.
- Invert polarity via software
- Individual calibration of expression pedals
- Configuration via IR/Bluetooth remote control

## Requirements
- Arduino Mega 2560 R3 or equivalent (a lite version for Arduino Uno R3 is on the way)
- 16x2 LiquidCrystal displays (LCDs) based on the Hitachi HD44780 (or a compatible) chipset
- IR Receiver module (like KY-022 or equivalent)
- [New Liquid Crystal](https://bitbucket.org/fmalpartida/new-liquidcrystal/wiki/Home) library (may works also with standard LiquidCrystal library)
- Customized [MIDI_Controller](https://github.com/tttapa/MIDI_controller) library
- [MD_Menu](https://github.com/MajicDesigns/MD_Menu) library
- [IRRemore](https://github.com/z3t0/Arduino-IRremote) library) library

## 

A MIDI firware for Arduino Uno/Mega is required. HIDUINO or mocoLUFA can be used.
I suggest a dual mode firmware like mocoLUFA because is supporting both USB-MIDI and Arduino-Serial.

More information can be obtained in the following links:
- [MIDI_Controller](https://github.com/tttapa/MIDI_controller)
- [HIDUINO](https://github.com/ddiakopoulos/hiduino)
- [MocoLUFA](https://github.com/kuwatay/mocolufa)

## Prototype

Potenziometers and switches can be replaced with any commercial footswitch or expression pedal for musical instruments.

![Fritzing](https://github.com/alf45tar/Pedalino/blob/master/Pedalino_bb.svg)

## ToDo

- Bluetooth MIDI
