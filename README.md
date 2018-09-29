_I know. You are thinking: "yet another MIDI controller with Arduino"._<br>
_Pedalino™ is something new from any previous DIY projects and even better of commercial alternatives at a fraction of the cost._

[![Build Status](https://travis-ci.org/alf45tar/Pedalino.svg?branch=master)](https://travis-ci.org/alf45tar/Pedalino)
[![Codacy Badge](https://api.codacy.com/project/badge/Grade/44375d0162e149469a30ee6549b9edb8)](https://app.codacy.com/app/alf45tar/Pedalino?utm_source=github.com&utm_medium=referral&utm_content=alf45tar/Pedalino&utm_campaign=Badge_Grade_Dashboard)

# Pedalino™

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
- Modular assembly of easy to find pieces of hardware and re-use of open source software libraries

## Applications

- Change preset to your favourite guitar rig
- Transform legacy MIDI equipment to USB MIDI class-compliant device
- Transform wired MIDI equipment into wireless MIDI equipment
- Transform legacy MIDI equipment to OSC control surface
- Connect Windows PC to macOS and iOS devices via AppleMIDI or Bluetooth LE
- Hands-free way to control your audio parameters during live performance
- Dramatically expanded audio system parameter control via WiFi, Bluetooth, USB, MIDI or OSC.
- Set your music free with wireless MIDI connectivity
- Bluetooth wireless MIDI adaptor for connecting instruments with MIDI IN/OUT terminals to your iOS devices (iPhone/iPad/iPod Touch) or Mac
- Send MIDI messages using an IR remote control

## Features

- Support for digital foot switches (momentary or latch), analog expression pedals and jog wheels (rotary encoders)
- 10 banks of 16 controllers each (up to 8 controllers for lite version)
- 3 user configuration profiles
- Each port can connect 1 expression pedal or up to 3 foot switches for a maximum of 48 foot switches.
- MIDI output via USB MIDI, Bluetooth, classic MIDI OUT connector, AppleMIDI (also known as RTP-MIDI) or ipMIDI via Wi-Fi
- Send the following MIDI events: Program Change, Control Code, Note On/Off or Pitch Bend
- MIDI channel, MIDI note, MIDI control code, MIDI program change can be configured by each pedal and by each bank
- Switch debouncing and analog noise suppression without decreasing responsiveness
- Invert polarity via software
- Individual automatic calibration of expression pedals. Manual fine tuning is not usually requested.
- Transform a linear expression pedal into log expression pedal and vice versa
- Configuration via IR remote control
- Change bank via IR remote control
- Simulate footswitch push via IR remote control
- Use any spare IR remote control
- Smart Config technology to help users connect to a Wi-Fi network through simple app on a smartphone.
- Firmware update via HTTP (http://pedalino.local/update)

## iOS and Android App

The app made with [Blynk](https://www.blynk.cc) is on the way. Here some images.

<img src="https://github.com/alf45tar/Pedalino/blob/master/images/ios-live.png" width="280"/> <img src="https://github.com/alf45tar/Pedalino/blob/master/images/ios-bank.png" width="280"/> <img src="https://github.com/alf45tar/Pedalino/blob/master/images/ios-pedal.png" width="280"/> <img src="https://github.com/alf45tar/Pedalino/blob/master/images/ios-interface.png" width="280"/>

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

- WIFI only (OPTION 1)
  - Any ESP8266/ESP12 board supported by [Arduino core for ESP8266 WiFi chip](https://github.com/esp8266/Arduino) with a serial interface available. We suggest a development board due to USB connection and 5V compatibility.
  - Tested on [ESP8266 ESP-01S](https://en.wikipedia.org/wiki/ESP8266) 1M WiFi module and YL-46 AMS1117 3.3V Power Supply Module (Arduino 3.3V pin cannot provide enough current for the ESP-01S stable operation)
  - [Arduino core for ESP8266 WiFi chip](https://github.com/esp8266/Arduino)
  
- WIFI and Bluetooth (OPTION 2)
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

### OPTION 1 - WiFi only

![Fritzing](https://github.com/alf45tar/Pedalino/blob/master/images/PedalinoESP8266_bb.png)

Have a look the video of a working prototype on [YouTube](https://youtu.be/9d6LPA7-HyU).

### OPTION 2 - WiFi and Bluetooth

![Fritzing](https://github.com/alf45tar/Pedalino/blob/master/images/PedalinoESP32-LCDKeypadShield_bb.png)

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

Most of them are plug-and-play because Pedalino will recognize via auto-sensing feature. 

Auto-sensing will also enable automatic calibration. After each power on cycle move the expression pedal to its full range and Pedalino will calibrate it. During the first full movement of the pedal MIDI events could be not precise because Pedalino is still learning the full range of the pedal.

## USB MIDI

A MIDI firware for Arduino Uno/Mega is required for using USB MIDI. Pedalino is tested with mocoLUFA because it supports dual mode boot (USB-MIDI or Arduino-Serial) and high-speed mode (1 Mbps). HIDUINO can works with minimal changes.

More information can be obtained in the following links:

- [MIDI_Controller](https://github.com/tttapa/MIDI_controller)
- [MocoLUFA](https://github.com/kuwatay/mocolufa)
- [HIDUINO](https://github.com/ddiakopoulos/hiduino)

### mocoLUFA dual boot

mocoLUFA firmware boots in USB MIDI mode by default.

Arduino-Serial mode is required to upload a new sketch into main Arduino ATmega2560 microcontroller.

To enable Arduino-Serial, add a jumper to PIN 4 (MOSI PB2) and PIN6 (GND) on ICSP connector for ATmega16U2. Reset is required to swicth the firmware mode.

![ArduinoSerialJumper](https://github.com/alf45tar/Pedalino/blob/master/images/mocoLUFA-ArduinoSerial-Jumper.svg)

### mocoLUFA high speed mode

mocoLUFA default speed is 31.250 bps but an undocumented high speed mode is in the code. A jumper betwen PIN 1 (MISO) and PIN 3 (SCK) on ICSP connector for ATmega16U2 enable the 1 Mbps speed. Pedalino USB MIDI works at 1 Mbps (1.000.000 bps).

![HighSpeedJumper](https://github.com/alf45tar/Pedalino/blob/master/images/mocoLUFA-HighSpeed-Jumper.svg)


## How to connect Pedalino to a WiFi network

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

You can reset the last know access point via menu.

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

## Firmware update

Pedalino is using 2 boards and 3 microcontrollers. All of them need to be flashed with the right firmware before using Pedalino.

Board|Microcontroller|Firmware|Flashing software|Flashing hardware
-----|-----|-----|-----|-----
Arduino Mega 2560|ATmega2560<hr>ATmega16U2|[Pedalino](https://github.com/alf45tar/Pedalino/tree/master/src/avr)<hr>[MocoLUFA](https://github.com/kuwatay/mocolufa)|[Arduino IDE](https://www.arduino.cc/en/Main/Software)/[PlatformIO IDE](https://platformio.org/platformio-ide)<hr>[Atmel's flip programmer](http://www.microchip.com/developmenttools/productdetails.aspx?partno=flip)|None
ESP-01S|ESP8266|[PedalinoESP](https://github.com/alf45tar/Pedalino/tree/master/src/esp)|[Arduino IDE](https://www.arduino.cc/en/Main/Software)/[PlatformIO IDE](https://platformio.org/platformio-ide)|Arduino Mega
NodeMCU DEVKIT V1.0|ESP8266|[PedalinoESP](https://github.com/alf45tar/Pedalino/tree/master/src/esp)|[Arduino IDE](https://www.arduino.cc/en/Main/Software)/[PlatformIO IDE](https://platformio.org/platformio-ide)|None
DOIT ESP32 DevKit V1|ESP32|[PedalinoESP](https://github.com/alf45tar/Pedalino/tree/master/src/esp)|[Arduino IDE](https://www.arduino.cc/en/Main/Software)/[PlatformIO IDE](https://platformio.org/platformio-ide)|None

Arduino and ESP8266/ESP32 board flash update is very easy and straithforward. It allows you to upload code without using any additional hardware inside the [Arduino IDE](https://www.arduino.cc/en/Guide/Environment) or [PlatformIO IDE](https://platformio.org/platformio-ide). All the requested libraries can be installed using the [Library Manager](https://www.arduino.cc/en/Guide/Libraries) inside the [Arduino IDE](https://www.arduino.cc/en/Guide/Environment) too. [PlatformIO IDE](https://platformio.org/platformio-ide) will install requested libraries on-the-fly during build process. 

Instructions for installing an USB class-compliant MIDI firmware for the ATmega16U2 can be found [here](https://www.arduino.cc/en/Hacking/DFUProgramming8U2) and [here](https://github.com/tttapa/MIDI_controller).

ESP-01S module is usually shipped with AT-Command firmware and flashing a new firmware requires an USB to Serial adapter as outlined [here](https://arduino-esp8266.readthedocs.io/en/latest/boards.html#minimal-hardware-setup-for-bootloading-and-usage). The easy way is to use an Arduino Uno/Mega as USB to Serial adapter. Connect a jumper from RESET to GND and Arduino is now a TTL-serial-to-USB converter.  RX0, TX0, and GND are the connections. Connect Arduino RX0 to ESP-01S RX (RX to RX is not an error), TX0 to TX and GND to GND. ESP8266 is 5V tolerant on RX and TX pins. Remember that Arduino 3.3V pin cannot provide enough current for the ESP-01S stable operation. 3.3V can be obtained from 5V pin using a power supply module or with an external power source.
An USB to Serial adapter is required just for the first firmware update. Once connected to WiFi, as outlined above, the ESP-01S module can be updated connecting your browser to http://pedalino.local/update.

Many different procedures can be used and many tutorials are available on internet including YouTube video.

## Build and upload software

### PlatformIO (recommended)

[PlatformIO](https://platformio.org) is the recommended IDE for Pedalino build and upload.

1. Install [PlatformIO IDE for VSCode](https://platformio.org/install/ide?install=vscode)
2. Run the following commands from View->Command Palette (Ctrl+Shift+P)
    - Git: Clone  
    - You will be asked for the URL of the remote repository (https://github.com/alf45tar/Pedalino) and the parent directory under which to put the local repository.
    - PlaftormIO: Build
    - PlatformIO: Upload

That's all folks.

### Arduino IDE

[Arduino IDE](https://www.arduino.cc/en/Guide/Environment) can be used for Pedalino build and upload but it requires more manual steps for libraries installation.

## Commercial alternatives

- [ControllerHub 8](https://ameliascompass.com/product/controllerhub-8/)
- [Audifront MIDI Expression](https://www.audiofront.net/MIDIExpression.php)
- [BomeBox](https://www.bome.com/products/bomebox)
- [iRig BlueBoard](http://www.ikmultimedia.com/products/irigblueboard)
- [Yamaha MD-BT01](https://usa.yamaha.com/products/music_production/accessories/md-bt01/index.html)

## ToDo

- [ ] Lite version for Arduino Uno R3
- [ ] Add rotary encoders
- [ ] User guide

# Copyright

Copyright 2017-2018 alf45tar
