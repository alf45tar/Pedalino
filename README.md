**Status:** Working but under active development

# Pedalino™
High customizable MIDI controller for guitarists and more.

## Features
- Support for digital foot switches (momentary or latch), analog expression pedals and jog wheels (rotary encoders)
- 10 banks of 16 controllers each (up to 8 controllers for lite version)
- MIDI output via USB MIDI, classic MIDI OUT connector or AppleMIDI (also known as RTP-MIDI) via WiFi
- Send the following MIDI events: Program Change, Control Code, Note On/Off or Pitch Bend
- MIDI channel, MIDI note, MIDI control code, MIDI program change can be configured by each pedal and by each bank
- Switch debouncing and analog noise suppression without decreasing responsiveness
- Invert polarity via software
- Individual automatic calibration of expression pedals. Manual fine tuning is not usually requested.
- Tranform a linear expression pedal into log expression pedal and vice versa
- Configuration via IR/Bluetooth remote control

## Requirements
- Arduino Mega 2560 R3 or equivalent (a lite version for Arduino Uno R3 is on the way)
- 16x2 LiquidCrystal displays (LCDs) based on the Hitachi HD44780 (or a compatible) chipset
- ESP8266 ESP-01 WiFi module
- Any IR Receiver module (like KY-022 or equivalent) supported by [IRremote](https://github.com/z3t0/Arduino-IRremote) library
- ZS-040 breakout board based on HC-08 Bluetooth UART Communication Module (HC-05 or HC-06 may works)
- [New Liquid Crystal](https://bitbucket.org/fmalpartida/new-liquidcrystal/wiki/Home) library (may works also with standard LiquidCrystal library)
- [MIDI_Controller](https://github.com/alf45tar/MIDI_controller) library
- [Arduino MIDI Library](https://github.com/FortySevenEffects/arduino_midi_library)
- [MD_Menu](https://github.com/MajicDesigns/MD_Menu) library
- [MD_UISwitch](https://github.com/MajicDesigns/MD_UISwitch) library
- [ResponsiveAnalogRead](https://github.com/dxinteractive/ResponsiveAnalogRead) library
- [IRremore](https://github.com/z3t0/Arduino-IRremote) library
- [Arduino core for ESP8266 WiFi chip](https://github.com/esp8266/Arduino)
- [AppleMIDI for Arduino](https://github.com/lathoub/Arduino-AppleMIDI-Library) library

## 

A MIDI firware for Arduino Uno/Mega is required. HIDUINO or mocoLUFA can be used.
I suggest a dual mode firmware like mocoLUFA because is supporting both USB-MIDI and Arduino-Serial.

More information can be obtained in the following links:
- [MIDI_Controller](https://github.com/tttapa/MIDI_controller)
- [HIDUINO](https://github.com/ddiakopoulos/hiduino)
- [MocoLUFA](https://github.com/kuwatay/mocolufa)

## Prototype

Potenziometers and switches can be replaced with any commercial footswitch or expression pedal for musical instruments.
Connect up to 16 pots and switches from pin A0 (pedal 1) to pin A15 (pedal 16).

![Fritzing](https://github.com/alf45tar/Pedalino/blob/master/Pedalino_bb.png)

## Pedalino™ Shield
![Fritzing](https://github.com/alf45tar/Pedalino/blob/master/PedalinoShield_bb.png)
![Fritzing](https://github.com/alf45tar/Pedalino/blob/master/PedalinoShield_pcb.png)

## How to connect Pedalino to a WiFi network for enable AppleMIDI

On power on Pedalino will try to connect to the last know access point for 30 seconds. If it cannot connect to the last used access point it enter into SmartConfig mode for 60 seconds. If it doesn't receive any SSID and password it switch to AP mode. In AP mode Pedalino create a WiFi network called 'Pedalino' waiting connection fro clients.

## ToDo

- [ ] Test lite version for Arduino Uno R3
- [ ] Test rotary encoders
- [ ] Bluetooth MIDI

# Copyright
Copyright 2017-2018 alfa45star
