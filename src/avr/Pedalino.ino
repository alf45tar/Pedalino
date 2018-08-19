/*
  //  __________           .___      .__  .__                   ___ ________________    ___
  //  \______   \ ____   __| _/____  |  | |__| ____   ____     /  / \__    ___/     \   \  \
  //   |     ___// __ \ / __ |\__  \ |  | |  |/    \ /  _ \   /  /    |    | /  \ /  \   \  \
  //   |    |   \  ___// /_/ | / __ \|  |_|  |   |  (  <_> ) (  (     |    |/    Y    \   )  )
  //   |____|    \___  >____ |(____  /____/__|___|  /\____/   \  \    |____|\____|__  /  /  /
  //                 \/     \/     \/             \/           \__\                 \/  /__/
*/

//#define DEBUG_PEDALINO

#include "Pedalino.h"
#include "Serialize.h"
#include "Config.h"
#include "MIDIRouting.h"
#include "Controller.h"
#include "Display.h"
#include "Menu.h"
#include "BlynkRPC.h"

// Standard setup() and loop()

void setup(void)
{
#ifdef DEBUG_PEDALINO
  SERIALDEBUG.begin(115200);

  DPRINTLNF("");
  DPRINTLNF("");
  DPRINTLNF("  __________           .___      .__  .__                   ___ ________________    ___");
  DPRINTLNF("  \\______   \\ ____   __| _/____  |  | |__| ____   ____     /  / \\__    ___/     \\   \\  \\");
  DPRINTLNF("   |     ___// __ \\ / __ |\\__  \\ |  | |  |/    \\ /  _ \\   /  /    |    | /  \\ /  \\   \\  \\");
  DPRINTLNF("   |    |   \\  ___// /_/ | / __ \\|  |_|  |   |  (  <_> ) (  (     |    |/    Y    \\   )  )");
  DPRINTLNF("   |____|    \\___  >____ |(____  /____/__|___|  /\\____/   \\  \\    |____|\\____|__  /  /  /");
  DPRINTLNF("                 \\/     \\/     \\/             \\/           \\__\\                 \\/  /__/");
  DPRINTLNF("                                                                       (c) 2018 alf45star");

#endif

  read_eeprom();

  // Initiate serial MIDI communications, listen to all channels and turn Thru off
#ifndef DEBUG_PEDALINO
  USB_MIDI.begin(MIDI_CHANNEL_OMNI);
  interfaces[PED_USBMIDI].midiThru ? USB_MIDI.turnThruOn() : USB_MIDI.turnThruOff();
#endif
  DIN_MIDI.begin(MIDI_CHANNEL_OMNI);
  interfaces[PED_LEGACYMIDI].midiThru ? DIN_MIDI.turnThruOn() : DIN_MIDI.turnThruOff();
  RTP_MIDI.begin(MIDI_CHANNEL_OMNI);
  interfaces[PED_APPLEMIDI].midiThru ? RTP_MIDI.turnThruOn() : RTP_MIDI.turnThruOff();

  autosensing_setup();
  controller_setup();
  MTC.setup();
  switch (currentMidiTimeCode) {

    case PED_MTC_NONE:
      DPRINTLNF("MTC None");
      MTC.setMode(MidiTimeCode::SynchroNone);
      break;

    case PED_MTC_SLAVE:
      DPRINTLNF("MTC Slave");
      MTC.setMode(MidiTimeCode::SynchroMTCSlave);
      break;

    case PED_MTC_MASTER_24:
    case PED_MTC_MASTER_25:
    case PED_MTC_MASTER_30DF:
    case PED_MTC_MASTER_30:
      DPRINTLNF("MTC Master");
      MTC.setMode(MidiTimeCode::SynchroMTCMaster);
      MTC.sendPosition(0, 0, 0, 0);
      break;

    case PED_MIDI_CLOCK_SLAVE:
      DPRINTLNF("MIDI Clock Slave");
      MTC.setMode(MidiTimeCode::SynchroClockSlave);
      bpm = 0;
      break;

    case PED_MIDI_CLOCK_MASTER:
      DPRINTLNF("MIDI Clock Master");
      MTC.setMode(MidiTimeCode::SynchroClockMaster);
      MTC.setBpm(bpm);
      break;
  }
  midi_routing_start();

  pinMode(LCD_BACKLIGHT, OUTPUT);
  analogWrite(LCD_BACKLIGHT, backlight);

  irrecv.enableIRIn();                                // Start the IR receiver
  irrecv.blink13(true);

  bluetooth.begin(9600);                              // Start the Bluetooth receiver
  bluetooth.println(F("AT+NAME=PedalinoMega"));       // Set bluetooth device name
  //Serial.begin(9600);
  //Blynk.begin(Serial, blynkAuthToken);
  //Blynk.begin(bluetooth, blynkAuthToken);
  //Blynk.config(bluetooth, blynkAuthToken);

  Serial1.begin(115200);
  delay(10);

  Blynk.begin(blynkAuthToken, wifi, "MyGuest", "0123456789");
  //blynkTimer.setInterval(3000L, myTimerEvent);
  //Blynk.notify("{DEVICE_NAME} is connected!");

  display(MD_Menu::DISP_INIT);
  M.begin();
  M.setMenuWrap(true);
  M.setAutoStart(AUTO_START);
  //M.setTimeout(MENU_TIMEOUT);
}


void loop(void)
{
  static bool prevMenuRun = true;

  // Detect if we need to initiate running normal user code
  if (prevMenuRun && !M.isInMenu())
    screen_update(true);
  prevMenuRun = M.isInMenu();

  // If we are not running and not autostart
  // check if there is a reason to start the menu
  if (!M.isInMenu() && !AUTO_START)
  {
    uint16_t dummy;

    if (navigation(dummy) == MD_Menu::NAV_SEL)
      M.runMenu(true);
  }
  if (!M.isInMenu()) screen_update();
  else lcd.noCursor();

  M.runMenu();   // just run the menu code

  Blynk.run();
  //blynkTimer.run();

  // Check whether the input has changed since last time, if so, send the new value over MIDI
  midi_refresh();
  midi_routing();
}

