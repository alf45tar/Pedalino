#define BLYNK_BANK                  V20
#define BLYNK_MIDIMESSAGE           V21
#define BLYNK_MIDICHANNEL           V22
#define BLYNK_MIDICODE              V23
#define BLYNK_MIDIVALUE1            V24
#define BLYNK_MIDIVALUE2            V25

#define BLYNK_PEDAL                 V30
#define BLYNK_PEDAL_MODE1           V31
#define BLYNK_PEDAL_MODE2           V32
#define BLYNK_PEDAL_FUNCTION        V33
#define BLYNK_PEDAL_AUTOSENSING     V34
#define BLYNK_PEDAL_SINGLEPRESS     V35
#define BLYNK_PEDAL_DOUBLEPRESS     V36
#define BLYNK_PEDAL_POLARITY        V38
#define BLYNK_PEDAL_CALIBRATE       V39
#define BLYNK_PEDAL_ANALOGZERO      V51
#define BLYNK_PEDAL_ANALOGMAX       V52

#define BLINK_INTERFACE             V40
#define BLINK_INTERFACE_MIDIIN      V41
#define BLINK_INTERFACE_MIDIOUT     V42
#define BLINK_INTERFACE_MIDITHRU    V43
#define BLINK_INTERFACE_MIDIROUTING V44

#define PRINT_VIRTUAL_PIN(vPin)     { DPRINTF("WRITE VirtualPIN "); DPRINT(vPin); }

void blynk_refresh()
{
  if (Blynk.connected())
  {
    Blynk.virtualWrite(BLYNK_BANK,                  currentBank + 1);
    Blynk.virtualWrite(BLYNK_PEDAL,                 currentPedal + 1);
    Blynk.virtualWrite(BLYNK_MIDIMESSAGE,           banks[currentBank][currentPedal].midiMessage + 1);
    Blynk.virtualWrite(BLYNK_MIDICHANNEL,           banks[currentBank][currentPedal].midiChannel);
    Blynk.virtualWrite(BLYNK_MIDICODE,              banks[currentBank][currentPedal].midiCode);
    Blynk.virtualWrite(BLYNK_MIDIVALUE1,            banks[currentBank][currentPedal].midiValue1);
    Blynk.virtualWrite(BLYNK_MIDIVALUE2,            banks[currentBank][currentPedal].midiValue2);

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
      Blynk.virtualWrite(BLYNK_PEDAL_ANALOGZERO,      pedals[currentPedal].expZero);
      Blynk.virtualWrite(BLYNK_PEDAL_ANALOGMAX,       pedals[currentPedal].expMax);
    }
    else {
      Blynk.virtualWrite(BLYNK_PEDAL_ANALOGZERO,      0);
      Blynk.virtualWrite(BLYNK_PEDAL_ANALOGMAX,       1023);
    }

    Blynk.virtualWrite(BLINK_INTERFACE,             currentInterface + 1);
    Blynk.virtualWrite(BLINK_INTERFACE_MIDIIN,      interfaces[currentInterface].midiIn);
    Blynk.virtualWrite(BLINK_INTERFACE_MIDIOUT,     interfaces[currentInterface].midiOut);
    Blynk.virtualWrite(BLINK_INTERFACE_MIDITHRU,    interfaces[currentInterface].midiThru);
    Blynk.virtualWrite(BLINK_INTERFACE_MIDIROUTING, interfaces[currentInterface].midiRouting);
  }
}

BLYNK_CONNECTED() {
  // This function is called when hardware connects to Blynk Cloud or private server.
  DPRINTLNF("Connected to Blynk");
  //blynkLCD.clear();
  blynk_refresh();
}

BLYNK_APP_CONNECTED() {
  //  This function is called every time Blynk app client connects to Blynk server.
  DPRINTLNF("Blink App connected");
  blynk_refresh();
}

BLYNK_APP_DISCONNECTED() {
  // This function is called every time the Blynk app disconnects from Blynk Cloud or private server.
  DPRINTLNF("Blink App disconnected");
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
  banks[currentBank][currentPedal].midiCode = constrain(code, 0, 255);
}

BLYNK_WRITE(BLYNK_MIDIVALUE1) {
  int code = param.asInt();
  PRINT_VIRTUAL_PIN(request.pin);
  DPRINTF(" - MIDI Single Press ");
  DPRINTLN(code);
  banks[currentBank][currentPedal].midiValue1 = constrain(code, 0, 255);
}

BLYNK_WRITE(BLYNK_MIDIVALUE2) {
  int code = param.asInt();
  PRINT_VIRTUAL_PIN(request.pin);
  DPRINTF(" - MIDI Double Press ");
  DPRINTLN(code);
  banks[currentBank][currentPedal].midiValue2 = constrain(code, 0, 255);
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

