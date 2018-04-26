**Status:** Working but under active development

[![Build Status](https://travis-ci.org/alf45tar/Pedalino.svg?branch=master)](https://travis-ci.org/alf45tar/Pedalino)


# Pedalino™
Smart wireless MIDI foot controller for guitarists and more.

- High customizable
- Multi purpose
- Battery operated
- No cable required
- Any number of pedals of any type in any order
- USB MIDI class-compliant device
- No extra drivers to connect Windows, macOS, iOS (iPad/iPhone) and Android. 
- AppleMIDI (aka RTP-MIDI) via Wi-Fi
- IEEE 802.11 b/g/n Wi-Fi 2.4 GHZ with WPA/WPA2 authentication
- Legacy MIDI OUT and MIDI IN connector
- Every MIDI port can route MIDI messages received from other interface
- Auto-sensing footswitches and expression pedals
- Modular assembly of easy to find pieces of hardware and re-use of open source software libraries

## Applications
- Change preset to your favourite guitar rig
- Transform legacy MIDI equipment to USB MIDI class-compliant device
- Transform wired MIDI equipment into wireless MIDI equipment
- Connect Windows PC to macOS and iOS devices via AppleMIDI
- Hands-free way to control your audio parameters during live performance
- Dramatically expanded audio system parameter control via WIFI, USB or MIDI.

## Features
- Support for digital foot switches (momentary or latch), analog expression pedals and jog wheels (rotary encoders)
- 10 banks of 16 controllers each (up to 8 controllers for lite version)
- Each port can connect 1 expression pedal or up to 3 foot switches for a maximum of 48 foot switches.
- MIDI output via USB MIDI, classic MIDI OUT connector or AppleMIDI (also known as RTP-MIDI) via Wi-Fi
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

- Arduino Mega 2560 R3 or equivalent
- 16x2 LiquidCrystal displays (LCDs) based on the Hitachi HD44780 (or a compatible) chipset
- [New Liquid Crystal](https://bitbucket.org/fmalpartida/new-liquidcrystal/wiki/Home) library
- [Bounce2](https://github.com/thomasfredericks/Bounce2) library
- [ResponsiveAnalogRead](https://github.com/dxinteractive/ResponsiveAnalogRead) library
- [MD_Menu](https://github.com/MajicDesigns/MD_Menu) library
- [MD_UISwitch](https://github.com/MajicDesigns/MD_UISwitch) library
- [Arduino MIDI](https://github.com/FortySevenEffects/arduino_midi_library) library

The rest is not mandatory but it depends of which features you want to support.

- USB MIDI class-compliant device
  - [MocoLUFA](https://github.com/kuwatay/mocolufa) firmware

- AppleMIDI via WIFI
  - ESP8266 ESP-01 1M WiFi module
  - [Arduino core for ESP8266 WiFi chip](https://github.com/esp8266/Arduino)
  - [AppleMIDI for Arduino](https://github.com/lathoub/Arduino-AppleMIDI-Library) library

- Infrared Remote Control
  - Any IR Receiver module (like KY-022 or equivalent) supported by [IRremote](https://github.com/z3t0/Arduino-IRremote) library
  - [IRremore](https://github.com/z3t0/Arduino-IRremote) library
  
- Bluetooth Remote Control
  - ZS-040 breakout board based on HC-08 Bluetooth UART Communication Module (HC-05 or HC-06 may works)

## Pedalino™ Shield

![Fritzing](https://github.com/alf45tar/Pedalino/blob/master/PedalinoShield_bb.png)
![Fritzing](https://github.com/alf45tar/Pedalino/blob/master/PedalinoShield_pcb.png)
![Fritzing](https://github.com/alf45tar/Pedalino/blob/master/PedalinoShieldOverview_bb.png)

## Pedal Wiring

Pedalino is designed to work with the majority of expression pedals on the market, but there are a few popular pedal types which are incompatible and need to use adapters in order to work with Pedalino.

There is no recognized standard for footswitch and expression pedal inputs. Effects and amp manufacturers use whatever variations are appropriate for their particular application. This can cause problems for the consumer needing to find a footswitch or an expression pedal that will work well with particular devices.

Pedals connector is usually a 1/4" TRS jack. Each stereo TRS socket should be connected as follow.

Pedal|TIP - Digital Pin|RING - Analog Pin|SLEEVE - Ground
-----|-----|-----|-----
1|23|A0|GND
2|25|A1|GND
3|27|A2|GND
4|29|A3|GND
5|31|A4|GND
6|33|A5|GND
7|35|A6|GND
8|37|A7|GND
9|39|A8|GND
10|41|A9|GND
11|43|A10|GND
12|45|A11|GND
13|47|A12|GND
14|49|A13|GND
15|52|A14|GND
16|53|A15|GND

## Foot switches

Pedalino supports the following wiring:

Switches per port|Connector|Wiring|Example
-----|-----|-----|-----
1|Mono 1/4" TS<br>![TS](https://github.com/alf45tar/Pedalino/blob/master/ts.gif)|1 switch between T and S|[Boss FS-5U](https://www.boss.info/us/products/fs-5u_5l/)<br>[Boss FS-5L](https://www.boss.info/us/products/fs-5u_5l/)
2|Stereo 1/4" TRS<br>![TRS](https://github.com/alf45tar/Pedalino/blob/master/trs.gif)|1<sup>st</sup> switch between T and S <br>2<sup>nd</sup> switch between R and S|[Boss FS-6](https://www.boss.info/us/products/fs-6/)<br>[Boss FS-7](https://www.boss.info/us/products/fs-7/)
3|Stereo 1/4" TRS<br>![TRS](https://github.com/alf45tar/Pedalino/blob/master/trs.gif)|[3-Button Schematic](https://github.com/alf45tar/Pedalino/blob/master/trs3button.jpg)|[Digitech FS3X](https://digitech.com/en/products/fs3x-3-button-footswitch)

Momentary and latch type switches are supported.

Normally open (NO) and normally closed (NC) is always supported and configurable by software if your foot switch do not have a polarity switch.

## Expression pedals

An expression pedal is more or less a pot with a 1/4" TRS jack plug.

Most potentiometers have three connectors; Clockwise, Counter-clockwise, and Wiper. Amazingly, there are multiple different ways these can be wired, all achieving largely the same result, which means yet more variations for expression pedals. The most common expression pedal wiring is to connect the pot to a 1/4″ stereo (TRS) instrument jack as follows:

Standard|Connector|Wiring|Example
-----|-----|-----|-----
Roland|Stereo 1/4" TRS<br>![TRS](https://github.com/alf45tar/Pedalino/blob/master/trs.gif)|CW —> Sleeve<br>Wiper —> Tip<br>CCW —> Ring|[Roland EV-5](https://www.roland.com/global/products/ev-5/)
Yamaha|Stereo 1/4" TRS<br>![TRS](https://github.com/alf45tar/Pedalino/blob/master/trs.gif)|CW —> Sleeve<br>Wiper —> Ring<br>CCW —> Tip|[Technics SZ-E1](https://www.roland.com/global/products/ev-5/)

Using a pedal with incompatible wiring can result in limited range, jumping or notch like response, or the pedal just won’t function at all, so make sure you check the requirements.

### Calibration

Pedalino like some of the more sophisticated effects and controllers incorporate a calibration utility that can mitigate some of the issues with pot rotation. There is a software option that allows the user to match the device to a specific expression pedal. This involves moving the pedal between it’s maximum and minimum settings and the device measuring the result. It then sets it’s internal parameters so that it recognizes where the maximum and minimum settings are of that particular pedal. This can often resolve problems of limited range. It’s important to calibrate all expression pedals. If the pedal is ever replaced, even with the same model, calibration should be run again.

## Auto Sensing

Most of them are plug-and-play because Pedalino will recognize via auto-senseing feature. 

Auto-sensing will also enable automatic calibration. After each power on cycle move the expression pedal to its full range and Pedalino will calibrate it. During the first full movement of the pedal MIDI events could be not precise because Pedalino is still learning the full range of the pedal.


## USB MIDI

A MIDI firware for Arduino Uno/Mega is required for using USB MIDI. HIDUINO or mocoLUFA can be used.
I suggest a dual mode firmware like mocoLUFA because is supporting both USB-MIDI and Arduino-Serial.

More information can be obtained in the following links:
- [MIDI_Controller](https://github.com/tttapa/MIDI_controller)
- [HIDUINO](https://github.com/ddiakopoulos/hiduino)
- [MocoLUFA](https://github.com/kuwatay/mocolufa)

## AppleMIDI

Pedalino is a session listener over Wi-Fi. It does not support session initiation functionalities, which requires the use of an external session initiator on the network to open a RTP-MIDI session with the Pedalino. This session initiator can be a macOS computer or a Windows computer with the [RTP-MIDI driver activated](https://www.tobias-erichsen.de/software/rtpmidi.html), or an embedded RTP-MIDI device.

## How to connect Pedalino to a WiFi network

AppleMIDI requires a network connection. Pedalino support IEEE 802.11 b/g/n Wi-Fi with WPA/WPA2 authentication (only 2.4 GHz).

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

## Commercial alternatives

- [ControllerHub 8](https://ameliascompass.com/product/controllerhub-8/)
- [Audifront MIDI Expression](https://www.audiofront.net/MIDIExpression.php)


## ToDo

- [ ] Lite version for Arduino Uno R3
- [ ] Add rotary encoders
- [ ] Bluetooth MIDI

# Copyright
Copyright 2017-2018 alfa45star
