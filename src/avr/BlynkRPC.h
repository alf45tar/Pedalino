#define BANK_SLIDER                 V20
#define PEDAL_SLIDER                V30
#define INTERFACE_SEGMENTEDSWITCH   V40

BLYNK_CONNECTED() {
  // This function is called when hardware connects to Blynk Cloud or private server.
  DPRINTLNF("Connected to Blynk");
  blynkLCD.clear();
}

BLYNK_APP_CONNECTED() {
  //  This function is called every time Blynk app client connects to Blynk server.
  DPRINTLNF("Blink App connected");
}

BLYNK_APP_DISCONNECTED() {
  // This function is called every time the Blynk app disconnects from Blynk Cloud or private server.
  DPRINTLNF("Blink App disconnected");
}

BLYNK_READ(V40) {

}

BLYNK_READ(V41) {
  Blynk.virtualWrite(V41, interfaces[currentInterface].midiIn);
}

BLYNK_READ(V42) {
  Blynk.virtualWrite(V42, interfaces[currentInterface].midiOut);
}

BLYNK_READ(V43) {
  Blynk.virtualWrite(V43, interfaces[currentInterface].midiThru);
}

BLYNK_READ(V44) {
  Blynk.virtualWrite(V44, interfaces[currentInterface].midiRouting);
}

WidgetLED led1(V2);

BLYNK_WRITE(V20) {
  int bank = param.asInt();
  DPRINTF("WRITE VirtualPIN 20");
  DPRINTF(" - Bank ");
  DPRINTLN(bank);
  currentBank = constrain(bank - 1, 0, BANKS - 1);
}

BLYNK_WRITE(V30) {
  int pedal = param.asInt();
  DPRINTF("WRITE VirtualPIN 30");
  DPRINTF(" - Pedal ");
  DPRINTLN(pedal);
  currentPedal = constrain(pedal - 1, 0, PEDALS - 1);
}

BLYNK_WRITE(V40) {
  int interface = param.asInt();
  DPRINTF("WRITE VirtualPIN 40");
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
}

BLYNK_WRITE(V41) {
  int onoff = param.asInt();
  DPRINTF("WRITE VirtualPIN 41");
  DPRINTF(" - ");
  DPRINTLN(onoff);
  interfaces[currentInterface].midiIn = onoff;
  led1.on();
}

BLYNK_WRITE(V42) {
  int onoff = param.asInt();
  DPRINTF("WRITE VirtualPIN 42");
  DPRINTF(" - ");
  DPRINTLN(onoff);
  interfaces[currentInterface].midiOut = onoff;
  led1.off();
}

BLYNK_WRITE(V43) {
  int onoff = param.asInt();
  DPRINTF("WRITE VirtualPIN 43");
  DPRINTF(" - ");
  DPRINTLN(onoff);
  interfaces[currentInterface].midiThru = onoff;
  Blynk.notify("{DEVICE_NAME} V43");
}

BLYNK_WRITE(V44) {
  int onoff = param.asInt();
  DPRINTF("WRITE VirtualPIN 44");
  DPRINTF(" - ");
  DPRINTLN(onoff);
  interfaces[currentInterface].midiRouting = onoff;
}

void myTimerEvent()
{
  /*
    Blynk.virtualWrite(V41, interfaces[currentInterface].midiIn);
    Blynk.virtualWrite(V42, interfaces[currentInterface].midiOut);
    Blynk.virtualWrite(V43, interfaces[currentInterface].midiThru);
    Blynk.virtualWrite(V44, interfaces[currentInterface].midiRouting);
  */
  led1.on();
}
