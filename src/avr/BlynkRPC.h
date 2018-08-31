/*  __________           .___      .__  .__                   ___ ________________    ___
 *  \______   \ ____   __| _/____  |  | |__| ____   ____     /  / \__    ___/     \   \  \   
 *   |     ___// __ \ / __ |\__  \ |  | |  |/    \ /  _ \   /  /    |    | /  \ /  \   \  \  
 *   |    |   \  ___// /_/ | / __ \|  |_|  |   |  (  <_> ) (  (     |    |/    Y    \   )  )
 *   |____|    \___  >____ |(____  /____/__|___|  /\____/   \  \    |____|\____|__  /  /  /
 *                 \/     \/     \/             \/           \__\                 \/  /__/
 *                                                                (c) 2018 alf45star
 *                                                        https://github.com/alf45tar/Pedalino
 */

//
//  Use Serial BLE modules (HM-10, HC-08) to connect your project to Blynk.

#define BLYNK_USE_DIRECT_CONNECT
//#define BLYNK_NO_BUILTIN                // Disable built-in analog & digital pin operations
//#define BLYNK_NO_FLOAT                  // Disable float operations

#ifdef SERIALDEBUG
#define BLYNK_PRINT SERIALDEBUG
#endif

#include <BlynkSimpleSerialBLE.h>
//#include <ESP8266_Lib.h>
//#include <BlynkSimpleShieldEsp8266.h>

#define BLYNK_CLOCK_START           V11
#define BLYNK_CLOCK_STOP            V12
#define BLYNK_CLOCK_CONTINUE        V13
#define BLYNK_MIDI_TIME_CODE        V14
#define BLYNK_CLOCK_MASTER_SLAVE    V15
#define BLYNK_BPM                   V16
#define BLYNK_TAP_TEMPO             V17

#define BLYNK_BANK                  V20
#define BLYNK_MIDIMESSAGE           V21
#define BLYNK_MIDICHANNEL           V22
#define BLYNK_MIDICODE              V23
#define BLYNK_MIDIVALUE1            V24
#define BLYNK_MIDIVALUE2            V25
#define BLYNK_MIDIVALUE3            V26

#define BLYNK_PEDAL                 V30
#define BLYNK_PEDAL_MODE1           V31
#define BLYNK_PEDAL_MODE2           V32
#define BLYNK_PEDAL_FUNCTION        V33
#define BLYNK_PEDAL_AUTOSENSING     V34
#define BLYNK_PEDAL_SINGLEPRESS     V35
#define BLYNK_PEDAL_DOUBLEPRESS     V36
#define BLYNK_PEDAL_LONGPRESS       V37
#define BLYNK_PEDAL_POLARITY        V38
#define BLYNK_PEDAL_CALIBRATE       V39
#define BLYNK_PEDAL_ANALOGZERO      V51
#define BLYNK_PEDAL_ANALOGMAX       V52

#define BLYNK_INTERFACE             V40
#define BLYNK_INTERFACE_MIDIIN      V41
#define BLYNK_INTERFACE_MIDIOUT     V42
#define BLYNK_INTERFACE_MIDITHRU    V43
#define BLYNK_INTERFACE_MIDIROUTING V44
#define BLYNK_INTERFACE_MIDICLOCK   V45

#define PRINT_VIRTUAL_PIN(vPin)     { DPRINTF("WRITE VirtualPIN "); DPRINT(vPin); }

//ESP8266 wifi(&Serial1);

const char blynkAuthToken[] = "31795677450a4ac088805d6d914bc747";
WidgetLCD  blynkLCD(V0);
void screen_update(boolean);

void blynk_refresh()
{
  //if (Blynk.connected())
  {
    switch (currentMidiTimeCode) {

    case PED_MTC_NONE:
      Blynk.virtualWrite(BLYNK_MIDI_TIME_CODE,          1);
      Blynk.virtualWrite(BLYNK_CLOCK_MASTER_SLAVE,      1);
      break;

    case PED_MIDI_CLOCK_MASTER:
      Blynk.virtualWrite(BLYNK_MIDI_TIME_CODE,          2);
      Blynk.virtualWrite(BLYNK_CLOCK_MASTER_SLAVE,      1);
      break;

    case PED_MIDI_CLOCK_SLAVE:
      Blynk.virtualWrite(BLYNK_MIDI_TIME_CODE,          2);
      Blynk.virtualWrite(BLYNK_CLOCK_MASTER_SLAVE,      2);
      break;

    case PED_MTC_MASTER_24:
    case PED_MTC_MASTER_25:
    case PED_MTC_MASTER_30DF:
    case PED_MTC_MASTER_30:
      Blynk.virtualWrite(BLYNK_MIDI_TIME_CODE,          3);
      Blynk.virtualWrite(BLYNK_CLOCK_MASTER_SLAVE,      1);
      break;

    case PED_MTC_SLAVE:
      Blynk.virtualWrite(BLYNK_MIDI_TIME_CODE,          3);
      Blynk.virtualWrite(BLYNK_CLOCK_MASTER_SLAVE,      2);
      break;
    }
    Blynk.virtualWrite(BLYNK_BPM,                   bpm);

    Blynk.virtualWrite(BLYNK_BANK,                  currentBank + 1);
    Blynk.virtualWrite(BLYNK_PEDAL,                 currentPedal + 1);
    Blynk.virtualWrite(BLYNK_MIDIMESSAGE,           banks[currentBank][currentPedal].midiMessage + 1);
    Blynk.virtualWrite(BLYNK_MIDICHANNEL,           banks[currentBank][currentPedal].midiChannel);
    Blynk.virtualWrite(BLYNK_MIDICODE,              banks[currentBank][currentPedal].midiCode);
    Blynk.virtualWrite(BLYNK_MIDIVALUE1,            banks[currentBank][currentPedal].midiValue1);
    Blynk.virtualWrite(BLYNK_MIDIVALUE2,            banks[currentBank][currentPedal].midiValue2);
    Blynk.virtualWrite(BLYNK_MIDIVALUE3,            banks[currentBank][currentPedal].midiValue3);

    Blynk.virtualWrite(BLYNK_PEDAL_FUNCTION,        pedals[currentPedal].function + 1);
    switch (pedals[currentPedal].mode) {
      case PED_MOMENTARY1:
        Blynk.virtualWrite(BLYNK_PEDAL_MODE1,           1);
        Blynk.virtualWrite(BLYNK_PEDAL_MODE2,           2);
        break;
      case PED_MOMENTARY2:
        Blynk.virtualWrite(BLYNK_PEDAL_MODE1,           1);
        Blynk.virtualWrite(BLYNK_PEDAL_MODE2,           3);
        break;
      case PED_MOMENTARY3:
        Blynk.virtualWrite(BLYNK_PEDAL_MODE1,           1);
        Blynk.virtualWrite(BLYNK_PEDAL_MODE2,           4);
        break;
      case PED_LATCH1:
        Blynk.virtualWrite(BLYNK_PEDAL_MODE1,           2);
        Blynk.virtualWrite(BLYNK_PEDAL_MODE2,           2);
        break;
      case PED_LATCH2:
        Blynk.virtualWrite(BLYNK_PEDAL_MODE1,           2);
        Blynk.virtualWrite(BLYNK_PEDAL_MODE2,           3);
        break;
      case PED_ANALOG:
        Blynk.virtualWrite(BLYNK_PEDAL_MODE1,           3);
        Blynk.virtualWrite(BLYNK_PEDAL_MODE2,           1);
        break;
      case PED_JOG_WHEEL:
        Blynk.virtualWrite(BLYNK_PEDAL_MODE1,           4);
        Blynk.virtualWrite(BLYNK_PEDAL_MODE2,           1);
        break;
    }
    Blynk.virtualWrite(BLYNK_PEDAL_AUTOSENSING,     pedals[currentPedal].autoSensing);
    Blynk.virtualWrite(BLYNK_PEDAL_POLARITY,        pedals[currentPedal].invertPolarity);
    if (pedals[currentPedal].mode == PED_ANALOG) {
      Blynk.virtualWrite(BLYNK_PEDAL_ANALOGZERO,    pedals[currentPedal].expZero);
      Blynk.virtualWrite(BLYNK_PEDAL_ANALOGMAX,     pedals[currentPedal].expMax);
    }
    else {
      Blynk.virtualWrite(BLYNK_PEDAL_ANALOGZERO,    0);
      Blynk.virtualWrite(BLYNK_PEDAL_ANALOGMAX,     1023);
    }

    Blynk.virtualWrite(BLYNK_INTERFACE,             currentInterface + 1);
    Blynk.virtualWrite(BLYNK_INTERFACE_MIDIIN,      interfaces[currentInterface].midiIn);
    Blynk.virtualWrite(BLYNK_INTERFACE_MIDIOUT,     interfaces[currentInterface].midiOut);
    Blynk.virtualWrite(BLYNK_INTERFACE_MIDITHRU,    interfaces[currentInterface].midiThru);
    Blynk.virtualWrite(BLYNK_INTERFACE_MIDIROUTING, interfaces[currentInterface].midiRouting);
    Blynk.virtualWrite(BLYNK_INTERFACE_MIDICLOCK,   interfaces[currentInterface].midiClock);
  }
}

BLYNK_CONNECTED() {
  // This function is called when hardware connects to Blynk Cloud or private server.
  DPRINTLNF("Connected to Blynk");
  blynk_refresh();
}

BLYNK_APP_CONNECTED() {
  //  This function is called every time Blynk app client connects to Blynk server.
  DPRINTLNF("Blink App connected");
  blynkLCD.clear();
  screen_update(true);
  blynk_refresh();
}

BLYNK_APP_DISCONNECTED() {
  // This function is called every time the Blynk app disconnects from Blynk Cloud or private server.
  DPRINTLNF("Blink App disconnected");
}


BLYNK_WRITE(BLYNK_MIDI_TIME_CODE) {
  int mtc = param.asInt();
  PRINT_VIRTUAL_PIN(request.pin);
  DPRINTF(" - MTC ");
  DPRINTLN(mtc);
  switch (currentMidiTimeCode) {

    case PED_MTC_NONE:
    case PED_MTC_MASTER_24:
    case PED_MTC_MASTER_25:
    case PED_MTC_MASTER_30DF:
    case PED_MTC_MASTER_30:
    case PED_MIDI_CLOCK_MASTER:
      switch (mtc) {
        case 1:
          MTC.setMode(MidiTimeCode::SynchroNone);
          currentMidiTimeCode = PED_MTC_NONE;
          break;
        case 2:
          MTC.setMode(MidiTimeCode::SynchroClockMaster);
          bpm = (bpm == 0) ? 120 : bpm;
          MTC.setBpm(bpm);
          currentMidiTimeCode = PED_MIDI_CLOCK_MASTER;
          Blynk.virtualWrite(BLYNK_BPM, bpm);
          break;
        case 3:
          MTC.setMode(MidiTimeCode::SynchroMTCMaster);
          MTC.sendPosition(0, 0, 0, 0);
          currentMidiTimeCode = PED_MTC_MASTER_24;
          break;
      }
      break;

    case PED_MTC_SLAVE:
    case PED_MIDI_CLOCK_SLAVE:
      switch (mtc) {
        case 1:
          MTC.setMode(MidiTimeCode::SynchroNone);
          currentMidiTimeCode = PED_MTC_NONE;
          break;
        case 2:
          MTC.setMode(MidiTimeCode::SynchroClockSlave);
          currentMidiTimeCode = PED_MIDI_CLOCK_SLAVE;
          bpm = 0;
          Blynk.virtualWrite(BLYNK_BPM, bpm);
          break;
        case 3:
          MTC.setMode(MidiTimeCode::SynchroMTCSlave);
          currentMidiTimeCode = PED_MTC_SLAVE;
          break;
      }
      break;
  }
}

BLYNK_WRITE(BLYNK_CLOCK_MASTER_SLAVE) {
  int master_slave = param.asInt();
  PRINT_VIRTUAL_PIN(request.pin);
  DPRINTF(" - CLOCK ");
  DPRINTLN(master_slave);
  switch (currentMidiTimeCode) {

    case PED_MTC_SLAVE:
    case PED_MTC_MASTER_24:
    case PED_MTC_MASTER_25:
    case PED_MTC_MASTER_30DF:
    case PED_MTC_MASTER_30:
      if (master_slave == 1) {
        MTC.setMode(MidiTimeCode::SynchroMTCMaster);
        MTC.sendPosition(0, 0, 0, 0);
        currentMidiTimeCode = PED_MTC_MASTER_24;
      }
      else
        MTC.setMode(MidiTimeCode::SynchroMTCSlave);
        currentMidiTimeCode = PED_MTC_SLAVE;
      break;

    case PED_MIDI_CLOCK_SLAVE:
    case PED_MIDI_CLOCK_MASTER:
      if (master_slave == 1) {
        MTC.setMode(MidiTimeCode::SynchroClockMaster);
        bpm = (bpm == 0) ? 120 : bpm;
        MTC.setBpm(bpm);
        currentMidiTimeCode = PED_MIDI_CLOCK_MASTER;
      }
      else {        
        MTC.setMode(MidiTimeCode::SynchroClockSlave);
        bpm = 0;
        currentMidiTimeCode = PED_MIDI_CLOCK_SLAVE;
      }
      Blynk.virtualWrite(BLYNK_BPM, bpm);
      break;
  }
}

BLYNK_WRITE(BLYNK_BPM) {
  int beatperminute = param.asInt();
  PRINT_VIRTUAL_PIN(request.pin);
  DPRINTF(" - BPM ");
  DPRINTLN(beatperminute);
  switch (currentMidiTimeCode) {
    case PED_MIDI_CLOCK_MASTER:
      bpm = constrain(beatperminute, 40, 300);
      MTC.setBpm(bpm);
      break;   
  }
}

BLYNK_WRITE(BLYNK_CLOCK_START) {
  int pressed = param.asInt();
  PRINT_VIRTUAL_PIN(request.pin);
  DPRINTF(" - Clock Start ");
  DPRINTLN(pressed);
  if (pressed) {
    MTC.sendPosition(0, 0, 0, 0);
    MTC.sendPlay();
  }
}

BLYNK_WRITE(BLYNK_CLOCK_STOP) {
  int pressed = param.asInt();
  PRINT_VIRTUAL_PIN(request.pin);
  DPRINTF(" - Clock Stop ");
  DPRINTLN(pressed);
  if (pressed) MTC.sendStop();
}

BLYNK_WRITE(BLYNK_CLOCK_CONTINUE) {
  int pressed = param.asInt();
  PRINT_VIRTUAL_PIN(request.pin);
  DPRINTF(" - Clock Continue ");
  DPRINTLN(pressed);
  if (pressed) MTC.sendContinue();
}

BLYNK_WRITE(BLYNK_TAP_TEMPO) {
  int pressed = param.asInt();
  PRINT_VIRTUAL_PIN(request.pin);
  DPRINTF(" - Tap Tempo ");
  DPRINTLN(pressed);
  if (pressed) {
    bpm = MTC.tapTempo();
    if (bpm > 0) {
      MTC.setBpm(bpm);
      Blynk.virtualWrite(BLYNK_BPM, bpm);
    }
  }
}

BLYNK_WRITE(BLYNK_BANK) {
  int bank = param.asInt();
  PRINT_VIRTUAL_PIN(request.pin);
  DPRINTF(" - Bank ");
  DPRINTLN(bank);
  currentBank = constrain(bank - 1, 0, BANKS - 1);
  blynk_refresh();
}

BLYNK_WRITE(BLYNK_MIDIMESSAGE) {
  int msg = param.asInt();
  PRINT_VIRTUAL_PIN(request.pin);
  DPRINTF(" - ");
  switch (msg) {
    case 1:
      DPRINTF("Program Change");
      break;
    case 2:
      DPRINTF("Control Change");
      break;
    case 3:
      DPRINTF("Note On/Off");
      break;
    case 4:
      DPRINTF("Pitch Bend");
      break;
  }
  banks[currentBank][currentPedal].midiMessage = constrain(msg - 1, 0, 3);
}

BLYNK_WRITE(BLYNK_MIDICHANNEL) {
  int channel = param.asInt();
  PRINT_VIRTUAL_PIN(request.pin);
  DPRINTF(" - MIDI Channel ");
  DPRINTLN(channel);
  banks[currentBank][currentPedal].midiChannel = constrain(channel, 1, 16);
}

BLYNK_WRITE(BLYNK_MIDICODE) {
  int code = param.asInt();
  PRINT_VIRTUAL_PIN(request.pin);
  DPRINTF(" - MIDI Code ");
  DPRINTLN(code);
  banks[currentBank][currentPedal].midiCode = constrain(code, 0, 127);
}

BLYNK_WRITE(BLYNK_MIDIVALUE1) {
  int code = param.asInt();
  PRINT_VIRTUAL_PIN(request.pin);
  DPRINTF(" - MIDI Single Press ");
  DPRINTLN(code);
  banks[currentBank][currentPedal].midiValue1 = constrain(code, 0, 127);
}

BLYNK_WRITE(BLYNK_MIDIVALUE2) {
  int code = param.asInt();
  PRINT_VIRTUAL_PIN(request.pin);
  DPRINTF(" - MIDI Double Press ");
  DPRINTLN(code);
  banks[currentBank][currentPedal].midiValue2 = constrain(code, 0, 127);
}

BLYNK_WRITE(BLYNK_MIDIVALUE3) {
  int code = param.asInt();
  PRINT_VIRTUAL_PIN(request.pin);
  DPRINTF(" - MIDI Long Press ");
  DPRINTLN(code);
  banks[currentBank][currentPedal].midiValue3 = constrain(code, 0, 127);
}


BLYNK_WRITE(BLYNK_PEDAL) {
  int pedal = param.asInt();
  PRINT_VIRTUAL_PIN(request.pin);
  DPRINTF(" - Pedal ");
  DPRINTLN(pedal);
  currentPedal = constrain(pedal - 1, 0, PEDALS - 1);
  blynk_refresh();
}

BLYNK_WRITE(BLYNK_PEDAL_MODE1) {
  int mode = param.asInt();
  PRINT_VIRTUAL_PIN(request.pin);
  DPRINTF(" - Mode ");
  DPRINTLN(mode);
  switch (mode) {
    case 1:
      pedals[currentPedal].mode = PED_MOMENTARY1;
      break;
    case 2:
      pedals[currentPedal].mode = PED_LATCH1;
      break;
    case 3:
      pedals[currentPedal].mode = PED_ANALOG;
      break;
    case 4:
      pedals[currentPedal].mode = PED_JOG_WHEEL;
      break;
  }
}

BLYNK_WRITE(BLYNK_PEDAL_MODE2) {
  int mode = param.asInt();
  PRINT_VIRTUAL_PIN(request.pin);
  DPRINTF(" - Mode ");
  DPRINTLN(mode);
  switch (mode) {
    case 1:
      pedals[currentPedal].mode = PED_MOMENTARY1;
      break;
    case 2:
      pedals[currentPedal].mode = PED_LATCH1;
      break;
    case 3:
      pedals[currentPedal].mode = PED_ANALOG;
      break;
    case 4:
      pedals[currentPedal].mode = PED_JOG_WHEEL;
      break;
  }
}

BLYNK_WRITE(BLYNK_PEDAL_FUNCTION) {
  int function = param.asInt();
  PRINT_VIRTUAL_PIN(request.pin);
  DPRINTF(" - Function ");
  DPRINTLN(function);
  pedals[currentPedal].function = function - 1;
}

BLYNK_WRITE(BLYNK_PEDAL_AUTOSENSING) {
  int autosensing = param.asInt();
  PRINT_VIRTUAL_PIN(request.pin);
  DPRINTF(" - Autosensing ");
  DPRINTLN(autosensing);
  pedals[currentPedal].autoSensing = autosensing;
}

BLYNK_WRITE(BLYNK_PEDAL_POLARITY) {
  int polarity = param.asInt();
  PRINT_VIRTUAL_PIN(request.pin);
  DPRINTF(" - Polarity ");
  DPRINTLN(polarity);
  pedals[currentPedal].invertPolarity = polarity;
}

BLYNK_WRITE(BLYNK_PEDAL_CALIBRATE) {
  int calibration = param.asInt();
  PRINT_VIRTUAL_PIN(request.pin);
  DPRINTF(" - Calibrate ");
  DPRINTLN(calibration);
  if (calibration && pedals[currentPedal].mode == PED_ANALOG) calibrate();
  blynk_refresh();
}

BLYNK_WRITE(BLYNK_PEDAL_ANALOGZERO) {
  int analogzero = param.asInt();
  PRINT_VIRTUAL_PIN(request.pin);
  DPRINTF(" - Analog Zero ");
  DPRINTLN(analogzero);
  pedals[currentPedal].expZero = analogzero;
  pedals[currentPedal].expMax = max(pedals[currentPedal].expMax, analogzero);
  Blynk.virtualWrite(BLYNK_PEDAL_ANALOGMAX, pedals[currentPedal].expMax);
}

BLYNK_WRITE(BLYNK_PEDAL_ANALOGMAX) {
  int analogmax = param.asInt();
  PRINT_VIRTUAL_PIN(request.pin);
  DPRINTF(" - Analog Max ");
  DPRINTLN(analogmax);
  pedals[currentPedal].expZero = min(pedals[currentPedal].expZero, analogmax);
  pedals[currentPedal].expMax = analogmax;
  Blynk.virtualWrite(BLYNK_PEDAL_ANALOGZERO, pedals[currentPedal].expZero);
}


BLYNK_WRITE(BLINK_INTERFACE) {
  int interface = param.asInt();
  PRINT_VIRTUAL_PIN(request.pin);
  DPRINTF(" - ");
  switch (interface) {
    case 1:
      DPRINTF("USB");
      break;
    case 2:
      DPRINTF("DIN");
      break;
    case 3:
      DPRINTF("RTP");
      break;
    case 4:
      DPRINTF("BLE");
      break;
  }
  DPRINTLNF(" MIDI interface");
  currentInterface = constrain(interface - 1, 0, INTERFACES);
  blynk_refresh();
}

BLYNK_WRITE(BLINK_INTERFACE_MIDIIN) {
  int onoff = param.asInt();
  PRINT_VIRTUAL_PIN(request.pin);
  DPRINTF(" - MIDI IN ");
  DPRINTLN(onoff);
  interfaces[currentInterface].midiIn = onoff;
}

BLYNK_WRITE(BLINK_INTERFACE_MIDIOUT) {
  int onoff = param.asInt();
  PRINT_VIRTUAL_PIN(request.pin);
  DPRINTF(" - MIDI OUT ");
  DPRINTLN(onoff);
  interfaces[currentInterface].midiOut = onoff;
}

BLYNK_WRITE(BLINK_INTERFACE_MIDITHRU) {
  int onoff = param.asInt();
  PRINT_VIRTUAL_PIN(request.pin);
  DPRINTF(" - MIDI THRU ");
  DPRINTLN(onoff);
  interfaces[currentInterface].midiThru = onoff;
}

BLYNK_WRITE(BLINK_INTERFACE_MIDIROUTING) {
  int onoff = param.asInt();
  PRINT_VIRTUAL_PIN(request.pin);
  DPRINTF(" - MIDI Routing ");
  DPRINTLN(onoff);
  interfaces[currentInterface].midiRouting = onoff;
}

