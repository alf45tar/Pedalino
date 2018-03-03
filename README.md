**Status:** Working but under active development

[![Build Status](https://travis-ci.org/alf45tar/Pedalino.svg?branch=master)](https://travis-ci.org/alf45tar/Pedalino)


# Pedalino™
High customizable MIDI controller for guitarists and more.

- USB MIDI class-compliant device
- No extra drivers to connect Windows, macOS, iOS (iPad/iPhone) and Android. 
- AppleMIDI (aka RTP-MIDI) via WiFi
- IEEE 802.11 b/g/n Wi-Fi 2.4 GHZ with WPA/WPA2 authentication
- MIDI DIN connector configured as MIDI OUT or MIDI IN or MIDI THRU
- MIDI routing

## Applications
- Change preset to your favourite guitar rig
- Transform legacy MIDI equipment to USB MIDI class-compliant device
- Connect Windows PC to macOS and iOS devices via AppleMIDI

## Features
- Support for digital foot switches (momentary or latch), analog expression pedals and jog wheels (rotary encoders)
- 10 banks of 16 controllers each (up to 8 controllers for lite version)
- MIDI output via USB MIDI, classic MIDI OUT connector or AppleMIDI (also known as RTP-MIDI) via WiFi
- Send the following MIDI events: Program Change, Control Code, Note On/Off or Pitch Bend
- MIDI channel, MIDI note, MIDI control code, MIDI program change can be configured by each pedal and by each bank
- Switch debouncing and analog noise suppression without decreasing responsiveness
- Invert polarity via software
- Individual automatic calibration of expression pedals. Manual fine tuning is not usually requested.
- Transform a linear expression pedal into log expression pedal and vice versa
- Configuration via IR/Bluetooth remote control
- Smart Config technology to help users connect to a Wi-Fi network through simple app on a smartphone.
- Firmware update via HTTP (http://pedalino/update)

## Bill of materials
- Arduino Mega 2560 R3 or equivalent (a lite version for Arduino Uno R3 is on the way)
- 16x2 LiquidCrystal displays (LCDs) based on the Hitachi HD44780 (or a compatible) chipset
- ESP8266 ESP-01 1M WiFi module
- Any IR Receiver module (like KY-022 or equivalent) supported by [IRremote](https://github.com/z3t0/Arduino-IRremote) library
- ZS-040 breakout board based on HC-08 Bluetooth UART Communication Module (HC-05 or HC-06 may works)
- [Arduino MIDI](https://github.com/FortySevenEffects/arduino_midi_library) library
- [New Liquid Crystal](https://bitbucket.org/fmalpartida/new-liquidcrystal/wiki/Home) library (may works also with standard LiquidCrystal library with minimal changes)
- [Bounce2](https://github.com/thomasfredericks/Bounce2) library
- [ResponsiveAnalogRead](https://github.com/dxinteractive/ResponsiveAnalogRead) library
- [MD_Menu](https://github.com/MajicDesigns/MD_Menu) library
- [MD_UISwitch](https://github.com/MajicDesigns/MD_UISwitch) library
- [IRremore](https://github.com/z3t0/Arduino-IRremote) library
- [Arduino core for ESP8266 WiFi chip](https://github.com/esp8266/Arduino)
- [AppleMIDI for Arduino](https://github.com/lathoub/Arduino-AppleMIDI-Library) library

## Pedalino™ Shield

Connect up to 16 pots and switches from pin A0 (pedal 1) to pin A15 (pedal 16). Potenziometers and switches can be replaced with any commercial footswitch or expression pedal for musical instruments.

![Fritzing](https://github.com/alf45tar/Pedalino/blob/master/PedalinoShield_bb.png)
![Fritzing](https://github.com/alf45tar/Pedalino/blob/master/PedalinoShield_pcb.png)
![Fritzing](https://github.com/alf45tar/Pedalino/blob/master/PedalinoShieldOverview_bb.png)

## Prototype

This is just an example for pots and switch connection.

![Fritzing](https://github.com/alf45tar/Pedalino/blob/master/Pedalino_bb.png)

## USB MIDI

A MIDI firware for Arduino Uno/Mega is required for using USB MIDI. HIDUINO or mocoLUFA can be used.
I suggest a dual mode firmware like mocoLUFA because is supporting both USB-MIDI and Arduino-Serial.

More information can be obtained in the following links:
- [MIDI_Controller](https://github.com/tttapa/MIDI_controller)
- [HIDUINO](https://github.com/ddiakopoulos/hiduino)
- [MocoLUFA](https://github.com/kuwatay/mocolufa)

## How to connect Pedalino to a WiFi network

AppleMIDI requires a network connection. Pedalino support IEEE 802.11 b/g/n Wi-Fi with WPA/WPA2 authentication (only 2.4 GHz).
Pedalino is a session listener over WiFi. It does not support session initiation functionnalities, which requires the use of an external session initiator on the network to open a RTP-MIDI session with the Pedalino. This session initiator can be a macOS computer or a Windows computer with the [RTP-MIDI driver activated](https://www.tobias-erichsen.de/software/rtpmidi.html), or an embedded RTP-MIDI device.

Pedalino implements Smart Config technology via [Espressif’s ESP-TOUCH protocol](https://www.espressif.com/en/products/software/esp-touch/overview) to help users connect embedded devices to a Wi-Fi network through simple configuration on a smartphone.

Tested apps for configure SSID and password are:
- [ESP8266 SmartConfig](https://play.google.com/store/apps/details?id=com.cmmakerclub.iot.esptouch) for Android
- [SmartConfig](https://itunes.apple.com/us/app/smartconfig/id1233975749?platform=iphone&preserveScrollPosition=true#platform/iphone) for iOS

Boot procedure
- On power on Pedalino will try to connect to the last know access point (double blinking led)
- If it cannot connect to the last used access point within 15 seconds it enters into Smart Config mode (slow blinking led)
- Start one of the tested apps to configure SSID and password 
- If it doesn't receive any SSID and password during the next 30 seconds it switch to AP mode (led off)
- In AP mode Pedalino create a WiFi network called 'Pedalino' waiting connection from clients. No password required. Led is off until a client connect to AP.
- Led is on when Pedalino is connected to an AP or a client is connected to Pedalino AP.
- Led will start fast blinking when Pedalino is partecipating to an AppleMIDI session. 
- Reboot Pedalino to restart the procedure.

You can reset the last know access point via menu.

## Pre-compiled source into hex files

I know compile the source code requires a lot of dependancies. I decided to provide also the .hex file for your convenience.
Uploading .hex file to Arduino it is very easy and straithforward with [Xloader](http://xloader.russemotto.com/).


## ToDo

- [ ] Test lite version for Arduino Uno R3
- [ ] Add rotary encoders
- [ ] Bluetooth MIDI
- [ ] MIDI routing

# Copyright
Copyright 2017-2018 alfa45star
