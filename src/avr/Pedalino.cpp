/*  __________           .___      .__  .__                   ___ ________________    ___
 *  \______   \ ____   __| _/____  |  | |__| ____   ____     /  / \__    ___/     \   \  \   
 *   |     ___// __ \ / __ |\__  \ |  | |  |/    \ /  _ \   /  /    |    | /  \ /  \   \  \  
 *   |    |   \  ___// /_/ | / __ \|  |_|  |   |  (  <_> ) (  (     |    |/    Y    \   )  )
 *   |____|    \___  >____ |(____  /____/__|___|  /\____/   \  \    |____|\____|__  /  /  /
 *                 \/     \/     \/             \/           \__\                 \/  /__/
 *                                                                (c) 2018 alf45star
 *                                                        https://github.com/alf45tar/Pedalino
 */

//#define DEBUG_PEDALINO
//#define BLYNK_DEBUG

#include "Pedalino.h"
#include "Serialize.h"
#include "Controller.h"
#include "BlynkRPC.h"
#include "Config.h"
#include "MIDIRouting.h"
#include "Display.h"
#include "Menu.h"

// Standard setup() and loop()

void setup(void)
{
#ifdef DEBUG_PEDALINO
  SERIALDEBUG.begin(115200);

  DPRINTLNF("");
  DPRINTLNF("  __________           .___      .__  .__                   ___ ________________    ___");
  DPRINTLNF("  \\______   \\ ____   __| _/____  |  | |__| ____   ____     /  / \\__    ___/     \\   \\  \\");
  DPRINTLNF("   |     ___// __ \\ / __ |\\__  \\ |  | |  |/    \\ /  _ \\   /  /    |    | /  \\ /  \\   \\  \\");
  DPRINTLNF("   |    |   \\  ___// /_/ | / __ \\|  |_|  |   |  (  <_> ) (  (     |    |/    Y    \\   )  )");
  DPRINTLNF("   |____|    \\___  >____ |(____  /____/__|___|  /\\____/   \\  \\    |____|\\____|__  /  /  /");
  DPRINTLNF("                 \\/     \\/     \\/             \\/           \\__\\                 \\/  /__/");
  DPRINTLNF("                                                                (c) 2018 alf45star");
  DPRINTLNF("                                                        https://github.com/alf45tar/Pedalino");
  DPRINTLNF("");

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
  mtc_setup();
  midi_routing_start();

  pinMode(LCD_BACKLIGHT, OUTPUT);
  analogWrite(LCD_BACKLIGHT, backlight);

  irrecv.enableIRIn(); // Start the IR receiver
  irrecv.blink13(true);

  bluetooth.begin(9600); // Start the Bluetooth receiver
  //bluetooth.println(F("AT+NAME=PedalinoMega"));       // Set bluetooth device name
  //Serial.begin(9600);
  //Blynk.begin(Serial, blynkAuthToken);
  //Blynk.begin(bluetooth, blynkAuthToken);
  Blynk.config(bluetooth, blynkAuthToken);

  //Serial1.begin(115200);
  //delay(10);

  //Blynk.begin(blynkAuthToken, wifi, "MyGuest", "0123456789");

  menu_setup();
}

void loop(void)
{
  // Display menu on request
  menu_run();

  // Process Blynk messages
  Blynk.run();

  // Check whether the input has changed since last time, if so, send the new value over MIDI
  midi_refresh();
  midi_routing();
}
