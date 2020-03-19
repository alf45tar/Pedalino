# Pedalino™ has been superseed by [PedalinoMini™](https://github.com/alf45tar/PedalinoMini).


# Pedalino™ (discontinued)

_I know. You are thinking: "yet another MIDI controller with Arduino"._
_Pedalino™ is something new from any previous DIY projects and even better of commercial alternatives at a fraction of the cost._

>Right now the hardware is just a working well prototype. It will be boxed when the hardware will be frozen.

[![](https://github.com/alf45tar/Pedalino/blob/master/images/youtube-video.png)](https://youtu.be/pCNSvJ9QiDs)

<img src="logo/Pedalino_Transparent.png" width="180"/>

[![Build Status](https://travis-ci.org/alf45tar/Pedalino.svg?branch=master)](https://travis-ci.org/alf45tar/Pedalino)
[![Codacy Badge](https://api.codacy.com/project/badge/Grade/44375d0162e149469a30ee6549b9edb8)](https://app.codacy.com/app/alf45tar/Pedalino?utm_source=github.com&utm_medium=referral&utm_content=alf45tar/Pedalino&utm_campaign=Badge_Grade_Dashboard)
[![Source Line of Code](https://img.shields.io/badge/Lines%20Of%20Code-7.3k-brightgreen.svg)](https://github.com/alf45tar/Pedalino)

Smart wireless MIDI foot controller for guitarists and more.

- Plug-and-play with any MIDI-compatible app on iOS 8 and above as well as OS X Yosemite and above.
- High customizable using iOS or Android app
- 4 MIDI interface (USB, Bluetooth, WiFi, legacy DIN MIDI IN and MIDI OUT connectors)
- No extra drivers to connect Windows, macOS, iOS (iPad/iPhone) and Android
- USB MIDI class-compliant device
- Bluetooth LE MIDI (iOS and macOS compatible)
- Network MIDI (aka AppleMIDI or RTP-MIDI)
- ipMIDI
- Open Sound Control (OSC)
- IEEE 802.11 b/g/n Wi-Fi 2.4 GHZ with WPA/WPA2 authentication
- Bluetooth Low Energy 4.0
- MIDI routing from/to any interface (USB, Bluetooth, WiFi, legacy)
- MIDI clock master and slave
- MIDI Time Code (MTC) master and slave
- OSC to MIDI and vicecersa
- Any number of pedals of any type in any order
- Auto-sensing footswitches and expression pedals
- Modular assembly of easy to find hardware and re-use of open source software libraries

## Applications

- Change preset to your favourite guitar rig
- Hands-free way to control your audio parameters during live performance
- Dramatically expanded audio system parameter control via WiFi, Bluetooth, USB, MIDI or OSC.
- Set your music free with wireless MIDI connectivity
- Bluetooth wireless MIDI adaptor for connecting instruments with MIDI IN/OUT terminals to your iOS devices (iPhone/iPad/iPod Touch) or Mac
- Transform legacy MIDI equipment to USB MIDI class-compliant device
- Transform wired MIDI equipment into wireless MIDI equipment
- Transform legacy MIDI equipment to OSC control surface
- Connect Windows to macOS and iOS devices via AppleMIDI or Bluetooth LE
- Send MIDI messages using an IR remote control from your sofa

## Features

- Support for digital foot switches (momentary or latch), analog expression pedals and jog wheels (rotary encoders)
- 10 banks of 16 controllers each
- 3 user configuration profiles
- Each port can connect 1 expression pedal or up to 3 foot switches for a maximum of 48 foot switches.
- MIDI output via USB MIDI, Bluetooth, classic MIDI OUT connector, AppleMIDI (also known as RTP-MIDI) or ipMIDI via Wi-Fi
- Send the following MIDI events: Program Change, Control Code, Note On/Off or Pitch Bend
- MIDI channel, MIDI note, MIDI control code, MIDI program change can be configured by each pedal and by each bank
- Switch debouncing and analog noise suppression without decreasing responsiveness
- Invert polarity via software
- Individual automatic calibration of expression pedals. Manual fine tuning is not usually requested.
- Transform a linear expression pedal into log expression pedal and vice versa
- Responsive and mobile-first configuration web interface (http://pedalino.local)
- Configuration via IR remote control
- Change bank via IR remote control
- Simulate footswitch push via IR remote control
- Use any spare IR remote control
- Smart Config technology to help users connect to a Wi-Fi network through simple app on a smartphone.
- Firmware update via HTTP (<http://pedalino.local/update>)

## iOS and Android App

The app made with [Blynk](https://www.blynk.cc) is on the way. Final version will be released later this year. Here some sneak preview images.

<img src="https://github.com/alf45tar/Pedalino/blob/master/images/ios-live.png" width="280"/> <img src="https://github.com/alf45tar/Pedalino/blob/master/images/ios-bank.png" width="280"/> <img src="https://github.com/alf45tar/Pedalino/blob/master/images/ios-pedal.png" width="280"/>

## Bill of materials

- Arduino Mega 2560 R3 or equivalent
- [LCD Keypad Shield](https://www.dfrobot.com/wiki/index.php/Arduino_LCD_KeyPad_Shield_(SKU:_DFR0009)) very popular 16x2 LCD based on the Hitachi HD44780 (or a compatible) chipset
- [New Liquid Crystal](https://bitbucket.org/fmalpartida/new-liquidcrystal/wiki/Home) library
- [Bounce2](https://github.com/thomasfredericks/Bounce2) library
- [ResponsiveAnalogRead](https://github.com/dxinteractive/ResponsiveAnalogRead) library
- [MD_Menu](https://github.com/MajicDesigns/MD_Menu) library
- [MD_UISwitch](https://github.com/MajicDesigns/MD_UISwitch) library
- [Arduino MIDI](https://github.com/FortySevenEffects/arduino_midi_library) library

The rest is not mandatory but it depends of which features you want to support.

- USB MIDI class-compliant device
  - [MocoLUFA](https://github.com/kuwatay/mocolufa) firmware
  
- MIDI IN interface
  - 6N137 Single-Channel High Speed Optocoupler (6N138 may works too)

- WIFI only (Model A)
  - Any ESP8266/ESP12 board supported by [Arduino core for ESP8266 WiFi chip](https://github.com/esp8266/Arduino) with a serial interface available. A development board provide direct USB connection and 5V compatibility.
  - Tested on [ESP8266 ESP-01S](https://en.wikipedia.org/wiki/ESP8266) 1M WiFi module and YL-46 AMS1117 3.3V Power Supply Module (Arduino 3.3V pin cannot provide enough current for the ESP-01S stable operation)
  - [Arduino core for ESP8266 WiFi chip](https://github.com/esp8266/Arduino)
  
- WIFI and Bluetooth MIDI (Model B)
  - Any ESP32 board supported by [Arduino core for ESP32 WiFi chip](https://github.com/espressif/arduino-esp32) with a serial interface available (usually Serial2 on standard development board because Serial is connected to USB and Serial1 cannot be connected during reset)
  - Tested on [DOIT ESP32 DevKit V1](https://github.com/SmartArduino/SZDOITWiKi/wiki/ESP8266---ESP32) 4M dual-mode Wi-Fi and Bluetooth module
  - [Arduino core for ESP32 WiFi chip](https://github.com/espressif/arduino-esp32)
  
- AppleMIDI via WIFI
  - [AppleMIDI for Arduino](https://github.com/lathoub/Arduino-AppleMIDI-Library) library
  
- OSC via WIFI
  - [CNMAT OSC for Arduino](https://github.com/CNMAT/OSC) library

- Infrared Remote Control
  - Any IR Receiver module (like KY-022 or equivalent) supported by [IRremote](https://github.com/z3t0/Arduino-IRremote) library
  - [IRremore](https://github.com/z3t0/Arduino-IRremote) library
  
- Bluetooth Remote Control
  - HM-10 (or compatible like HC-08) Bluetooth UART Communication Module

## Pedalino™ Breadboard Prototype

### MODEL A - WiFi only
![Fritzing](https://github.com/alf45tar/Pedalino/blob/master/images/PedalinoESP8266-LCDKeypadShield_bb.png)

### MODEL B - WiFi and Bluetooth MIDI
![Fritzing](https://github.com/alf45tar/Pedalino/blob/master/images/PedalinoESP32-LCDKeypadShield_bb.png)

Model A and B use HM-10 Bluetooth LE module to connect the app.

### MODEL C - Arduino UNO R3 + ESP8266 - WiFi only
Due to memory limit of Arduino Uno R3 some of the features cannot be supported. We eliminated the superfluous ones and kept the most interesting ones.

- All the interfaces (USB, Bluetooth, WiFi, legacy DIN MIDI IN and MIDI OUT connectors) and protocols (NetworkMIDI, IPMIDI and OSC) are supported
- 5 banks of 8 controllers each
- 3 profiles
- Configuration via web interface only
- No LCD
- No app
- No IR remote control

![Fritzing](https://github.com/alf45tar/Pedalino/blob/master/images/PedalinoESP8266-Uno_bb.png)

### First prototype
![](https://github.com/alf45tar/Pedalino/blob/master/images/first-prototype-model-a.jpg)

## Firmware update

Pedalino is using 2 boards and 3 microcontrollers. All of them need to be flashed with the right firmware before using Pedalino.

Model|Board|Microcontroller|Firmware|Flashing software|Flashing hardware|Instructions
-----|-----|-----|-----|-----|-----|-----
Both|Arduino Mega 2560|ATmega2560<hr>ATmega16U2|[Pedalino](https://github.com/alf45tar/Pedalino/tree/master/src/avr)<hr>[MocoLUFA](https://github.com/kuwatay/mocolufa)|[Arduino IDE](https://www.arduino.cc/en/Main/Software)/[PlatformIO IDE](https://platformio.org/platformio-ide)<hr>[Atmel's flip programmer](http://www.microchip.com/developmenttools/productdetails.aspx?partno=flip)|None|[Click here](https://github.com/alf45tar/Pedalino/wiki/Build-and-upload-software)<hr>[Click here](https://www.arduino.cc/en/Hacking/DFUProgramming8U2) and [here](https://github.com/tttapa/MIDI_controller)
A|ESP-01S 1M|ESP8266|[PedalinoESP](https://github.com/alf45tar/Pedalino/tree/master/src/esp)|[Arduino IDE](https://www.arduino.cc/en/Main/Software)/[PlatformIO IDE](https://platformio.org/platformio-ide)|Arduino Mega|[Click here](https://github.com/alf45tar/Pedalino/wiki/How-to-flash-ESP8266-ESP%E2%80%9001S-WiFi-module)
B|DOIT ESP32 DevKit V1|ESP32|[PedalinoESP](https://github.com/alf45tar/Pedalino/tree/master/src/esp)|[Arduino IDE](https://www.arduino.cc/en/Main/Software)/[PlatformIO IDE](https://platformio.org/platformio-ide)|None|[Click here](https://github.com/alf45tar/Pedalino/wiki/Build-and-upload-software)

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
1|Mono 1/4" TS<br>![TS](https://github.com/alf45tar/Pedalino/blob/master/images/ts.gif)|1 switch between T and S|[Boss FS-5U](https://www.boss.info/us/products/fs-5u_5l/)<br>[Boss FS-5L](https://www.boss.info/us/products/fs-5u_5l/)
2|Stereo 1/4" TRS<br>![TRS](https://github.com/alf45tar/Pedalino/blob/master/images/trs.gif)|1<sup>st</sup> switch between T and S <br>2<sup>nd</sup> switch between R and S|[Boss FS-6](https://www.boss.info/us/products/fs-6/)<br>[Boss FS-7](https://www.boss.info/us/products/fs-7/)
3|Stereo 1/4" TRS<br>![TRS](https://github.com/alf45tar/Pedalino/blob/master/images/trs.gif)|[3-Button Schematic](https://github.com/alf45tar/Pedalino/blob/master/images/trs3button.jpg)|[Digitech FS3X](https://digitech.com/en/products/fs3x-3-button-footswitch)
5|Stereo 1/4" TRS<br>![TRS](https://github.com/alf45tar/Pedalino/blob/master/images/trs.gif)|[Voltage Ladder](https://github.com/alf45tar/Pedalino/blob/master/images/lcd_switch_ladder.png)|[LCD Keypad Shield](https://www.dfrobot.com/wiki/index.php/Arduino_LCD_KeyPad_Shield_(SKU:_DFR0009))

Momentary and latch type switches are supported.

Normally open (NO) and normally closed (NC) is always supported and configurable by software if your foot switch do not have a polarity switch.

## Expression pedals

An expression pedal is more or less a pot with a 1/4" TRS jack plug.

Most potentiometers have three connectors; Clockwise, Counter-clockwise, and Wiper. Amazingly, there are multiple different ways these can be wired, all achieving largely the same result, which means yet more variations for expression pedals. The most common expression pedal wiring is to connect the pot to a 1/4″ stereo (TRS) instrument jack as follows:

Standard|Connector|Wiring|Example
-----|-----|-----|-----
Roland|Stereo 1/4" TRS<br>![TRS](https://github.com/alf45tar/Pedalino/blob/master/images/trs.gif)|CW —> Sleeve<br>Wiper —> Tip<br>CCW —> Ring|[Roland EV-5](https://www.roland.com/global/products/ev-5/)<br>[M-Audio EX-P](http://www.m-audio.com/products/view/ex-p) (switch in "M-Audio")
Yamaha|Stereo 1/4" TRS<br>![TRS](https://github.com/alf45tar/Pedalino/blob/master/images/trs.gif)|CW —> Sleeve<br>Wiper —> Ring<br>CCW —> Tip|[Yamaha FC7](https://usa.yamaha.com/products/music_production/accessories/fc7/index.html)<br>[M-Audio EX-P](http://www.m-audio.com/products/view/ex-p) (switch in "Other")<br>Technics SZ-E1/SZ-E2

Using a pedal with incompatible wiring can result in limited range, jumping or notch like response, or the pedal just won’t function at all, so make sure you check the requirements.

### Calibration

Pedalino like some of the more sophisticated effects and controllers incorporate a calibration utility that can mitigate some of the issues with pot rotation. There is a software option that allows the user to match the device to a specific expression pedal. This involves moving the pedal between it’s maximum and minimum settings and the device measuring the result. It then sets it’s internal parameters so that it recognizes where the maximum and minimum settings are of that particular pedal. This can often resolve problems of limited range. It’s important to calibrate all expression pedals. If the pedal is ever replaced, even with the same model, calibration should be run again.

## Auto Sensing

Most of the foot switches and expression pedals are plug-and-play because Pedalino will recognize via auto-sensing feature.

Auto-sensing will also enable automatic calibration. After each power on cycle move the expression pedal to its full range and Pedalino will calibrate it. During the first full movement of the pedal MIDI events could be not precise because Pedalino is still learning the full range of the pedal.

## USB MIDI

An USB class-compliant MIDI firmware for Arduino Uno/Mega (ATmega16U2) is required for using USB MIDI. Pedalino is tested with mocoLUFA because it supports dual mode boot (USB-MIDI or Arduino-Serial) and high-speed mode (1 Mbps). HIDUINO can works with minimal changes.

More information can be obtained in the following links:

- [MIDI_Controller](https://github.com/tttapa/MIDI_controller)
- [MocoLUFA](https://github.com/kuwatay/mocolufa)
- [HIDUINO](https://github.com/ddiakopoulos/hiduino)

### mocoLUFA dual boot

mocoLUFA firmware boots in USB MIDI mode by default.

Arduino-Serial mode is required to upload a new sketch into main Arduino ATmega2560 microcontroller.

To enable Arduino-Serial, add a jumper to PIN 4 (MOSI PB2) and PIN6 (GND) on ICSP connector for ATmega16U2. Power on cycle is required to switch the firmware mode.

![ArduinoSerialJumper](https://github.com/alf45tar/Pedalino/blob/master/images/mocoLUFA-ArduinoSerial-Jumper.svg)

### mocoLUFA high speed mode

mocoLUFA default speed is 31.250 bps but an undocumented high speed mode is in the code. A jumper between PIN 1 (MISO) and PIN 3 (SCK) on ICSP connector for ATmega16U2 enable the 1 Mbps speed. Pedalino USB MIDI works at 1 Mbps (1.000.000 bps).

![HighSpeedJumper](https://github.com/alf45tar/Pedalino/blob/master/images/mocoLUFA-HighSpeed-Jumper.svg)

## <a name="wifi"></a>How to connect Pedalino to a WiFi network

AppleMIDI, ipMIDI and Open Sound Control (OSC) protocol requires a network connection. Pedalino support IEEE 802.11 b/g/n WiFi with WPA/WPA2 authentication (only 2.4 GHz).

Pedalino implements Smart Config technology via [Espressif’s ESP-TOUCH protocol](https://www.espressif.com/en/products/software/esp-touch/overview) to help users connect embedded devices to a WiFi network through simple configuration on a smartphone.

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

You can reset the last know access point via _Options->WiFi Reset_ menu.

## MIDI Network - AppleMIDI - RTP-MIDI

RTP-MIDI (also known as AppleMIDI) is a protocol to transport MIDI messages within RTP (Real-time Protocol) packets over Ethernet and WiFi networks. It is completely open and free.

Pedalino is a session listener over WiFi. It does not support session initiation functionalities, which requires the use of an external session initiator on the network to open a RTP-MIDI session with the Pedalino. This session initiator can be a macOS computer (Audio MIDI Setup->MIDI Studio->MIDI Network Setup) or a Windows computer with the [RTP-MIDI driver](https://www.tobias-erichsen.de/software/rtpmidi.html), an app on your iOS device (for example [MIDI Network](http://audioapps.nl/app/MIDINetwork)) or an embedded RTP-MIDI device.

Pedalino is designed to be compatible with:

- macOS computer
- iOS devices
- [rtpMIDI](https://www.tobias-erichsen.de/software/rtpmidi.html) for Windows
- [mnet MIDIhub](https://www.humatic.de/htools/mnet/man.htm) for Windows

## ipMIDI

Pedalino can route MIDI over your Ethernet or WiFi network, using [ipMIDI](https://www.nerds.de/en/ipmidi.html) protocol to send and receive MIDI data between computers connected to your LAN.

Pedalino is designed to be compatible with:

- [ipMIDI](https://www.nerds.de/en/ipmidi.html) for Windows/macOS
- [mnet MIDIhub](https://www.humatic.de/htools/mnet/man.htm) for Windows/macOS
- [QmidiNet](https://qmidinet.sourceforge.io) for Linux
- [multimidicast](http://llg.cubic.org/tools/multimidicast) for Linux

I prefers [mnet MIDIhub](https://www.humatic.de/htools/mnet/man.htm) for both RTP-MIDI and ipMIDI protocols. It is free and it works very well under Windows.

## Open Sound Control (OSC)

[Open Sound Control (OSC)](http://opensoundcontrol.org/introduction-osc) is an open, transport-independent, message-based protocol developed for communication among computers, sound synthesizers, and other multimedia devices.
Although the OSC specification does not enforce any particular type of transport, OSC is nowadays mostly used over traditional
networks known as IP (Internet Protocol).

The OSC protocol uses the IP network to carry messages from a source to a destination. Sources of OSC messages are
usually called OSC Clients, and destinations OSC Servers.

Pedalino is both able to receive (as a server), and send to several destinations (as multiple clients).

UDP and TCP are network protocols used for communicating OSC messages between two devices. UDP is the most widely used at the moment.

Pedalino supports UDP protocol only for transmitting and receiving OSC messages because TCP has more latency than UDP.

Pedalino will listen for OSC messages on UDP port 8000 and broadcast OSC messages on UDP port 9000 on the same WiFi LAN segment it is connected.

### OSC namespace

OSC specification does not define any namespace. There is no de-facto standard too.

Pedalino OSC namespace for incoming and outcoming MIDI events is:

MIDI Event|OSC Address|OSC Arguments|Min|Max|Note
-----|-----|-----|-----|-----|-----
Note On|/pedalino/midi/note/#|float velocity<br>int channel|0<br>1|1<br>16|# is the MIDI note number 0..127
Note Off|/pedalino/midi/note/#|float velocity<br>int channel|0<br>1|0<br>16|# is the MIDI note number 0..127
Control Change|/pedalino/midi/cc/#|float value<br>int channel|0<br>1|1<br>16|# is the MIDI CC number 0..127
Program Change|/pedalino/midi/pc/#|int channel|1|16|# is the MIDI PC number 0..127
Pitch Bend|/pedalino/midi/pitchbend/#|float bend|0|1|# is the MIDI channel 1..16
After Touch Poly|/pedalino/midi/aftertouchpoly/#|float pressure|0|1|# is the MIDI note number 0..127
After Touch Chennel|/pedalino/midi/aftertouchchannel/#|float pressure|0|1|# is the MIDI channel 1..16
Song Position Pointer|/pedalino/midi/songposition/#|int beats|0|16383|# is song position in beats (1/16 note)
Song Select|/pedalino/midi/songselect/#|int number|0|127|# is song number 0..127
Tune Request|/pedalino/midi/tunerequest/||||
Start|/pedalino/midi/start/||||
Continue|/pedalino/midi/continue/||||
Stop|/pedalino/midi/stop/||||
Active Sensing|/pedalino/midi/activesensing/||||
Reset|/pedalino/midi/reset/||||

### OSC-to-MIDI and MIDI-to-OSC

Pedalino is able to converts incoming OSC messages to MIDI events and outgoing MIDI events to OSC messages.

The bottom line is you can connect MIDI devices (or software) that does not suport OSC natively with OSC enabled software (or device) without any hard to configure software bridge.

## Pedalino articles

- [These twenty projects won the Musical Instrument Challenge in the Hackaday Prize](https://hackaday.com/2018/10/16/these-twenty-projects-won-the-musical-instrument-challenge-in-the-hackaday-prize/) by Brian Benchoff on [hackaday.com](https://hackaday.com/2018/10/16/these-twenty-projects-won-the-musical-instrument-challenge-in-the-hackaday-prize/)
- [Finally, an open source MIDI foot controller](https://hackaday.com/2018/10/21/finally-an-open-source-midi-foot-controller/) by Brian Benchoff on [hackaday.com](https://hackaday.com/2018/10/21/finally-an-open-source-midi-foot-controller/)
- [Pedalino: A smart, open source wireless MIDI foot controller for guitarists and more](https://blog.adafruit.com/2018/10/30/pedalino-a-smart-open-source-wireless-midi-foot-controller-for-guitarists-and-more/) by Mike Barela on [adafruit.com](https://blog.adafruit.com/2018/10/30/pedalino-a-smart-open-source-wireless-midi-foot-controller-for-guitarists-and-more/)
- [Pedalino open source wireless MIDI foot controller with iOS and Android app](https://www.geeky-gadgets.com/pedalino-open-source-wireless-midi-foot-controller-31-10-2018/) by Julian Horsey on [geeky-gadgets.com](https://www.geeky-gadgets.com/pedalino-open-source-wireless-midi-foot-controller-31-10-2018/)
- [Pedalino: Open Source Wireless MIDI Foot Controller](https://www.open-electronics.org/pedalino-open-source-wireless-midi-foot-controller/) by Luca Ruggeri on [open-electronics.org](https://www.open-electronics.org/pedalino-open-source-wireless-midi-foot-controller/)

## Commercial alternatives

- [ControllerHub 8](https://ameliascompass.com/product/controllerhub-8/)
- [Audifront MIDI Expression](https://www.audiofront.net/MIDIExpression.php)
- [BomeBox](https://www.bome.com/products/bomebox)
- [iRig BlueBoard](http://www.ikmultimedia.com/products/irigblueboard)
- [Yamaha MD-BT01](https://usa.yamaha.com/products/music_production/accessories/md-bt01/index.html)

## ToDo

- [ ] Lite version for Arduino Uno R3
- [ ] Configuration web interface
- [ ] Build a plug&play network of interconnected Pedalino for MIDI routing
- [ ] Pedalino Mini (only USB and WiFi)
- [ ] Add rotary encoders
- [ ] User guide

## Copyright

Copyright 2017-2018 alf45tar
