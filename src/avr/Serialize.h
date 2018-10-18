/*  __________           .___      .__  .__                   ___ ________________    ___
 *  \______   \ ____   __| _/____  |  | |__| ____   ____     /  / \__    ___/     \   \  \   
 *   |     ___// __ \ / __ |\__  \ |  | |  |/    \ /  _ \   /  /    |    | /  \ /  \   \  \  
 *   |    |   \  ___// /_/ | / __ \|  |_|  |   |  (  <_> ) (  (     |    |/    Y    \   )  )
 *   |____|    \___  >____ |(____  /____/__|___|  /\____/   \  \    |____|\____|__  /  /  /
 *                 \/     \/     \/             \/           \__\                 \/  /__/
 *                                                                (c) 2018 alf45star
 *                                                        https://github.com/alf45tar/Pedalino
 */

#include <ArduinoJson.h>                // https://arduinojson.org/


void serialize_lcd1(const char *l) {

  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  //char          originalSysEx[128];
  //byte          encodedSysEx[128];
  //unsigned int  encodedSize;

  root["lcd1"] = String(l);

  Serial3.write(0xF0);
  root.printTo(Serial3);
  Serial3.write(0xF7);
  Serial3.flush();
  /*
    root.printTo(originalSysEx, sizeof(originalSysEx));
    for (unsigned int i = 0; i < strlen(originalSysEx); i++)
    originalSysEx[i] &= 0x7F;
    RTP_MIDI.sendSysEx(originalSysEx, strlen(originalSysEx));
  */
  /*
    DPRINTLN(originalSysEx);
    encodedSize = midi::encodeSysEx((const byte *)originalSysEx, encodedSysEx, strlen(originalSysEx));
    DPRINTLN(encodedSize);
    DPRINTLN((char *)encodedSysEx);
    RTP_MIDI.sendSysEx(encodedSize, encodedSysEx);
  */
}


void serialize_lcd2(const char *l) {

  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();

  root["lcd2"] = String(l);

  Serial3.write(0xF0);
  root.printTo(Serial3);
  Serial3.write(0xF7);
  Serial3.flush();
}

void serialize_lcd_clear() {

  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();

  root["lcd.clear"] = true;

  Serial3.write(0xF0);
  root.printTo(Serial3);
  Serial3.write(0xF7);
  Serial3.flush();
}

void serialize_factory_default() {

  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();

  root["factory.default"] = true;

  Serial3.write(0xF0);
  root.printTo(Serial3);
  Serial3.write(0xF7);
  Serial3.flush();

  DPRINTLN("JSON: factory.default");
}

void serialize_bank(byte b = currentBank, byte p = currentPedal) {

  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();

  root["bank"]    = b;
  root["pedal"]   = p;
  root["message"] = banks[b][p].midiMessage;
  root["channel"] = banks[b][p].midiChannel;
  root["code"]    = banks[b][p].midiCode;
  root["value1"]  = banks[b][p].midiValue1;
  root["value2"]  = banks[b][p].midiValue2;
  root["value3"]  = banks[b][p].midiValue3;

  Serial3.write(0xF0);
  root.printTo(Serial3);
  Serial3.write(0xF7);
  Serial3.flush();
}

void serialize_banks() {

  for (byte b = 0; b < BANKS; b++)
    for ( byte p = 0; p < PEDALS; p++)
      serialize_bank(b, p);
}

void serialize_pedal(byte p = currentPedal) {

  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();

  root["pedal"]           = p;
  root["function"]        = pedals[p].function;
  root["autosensing"]     = pedals[p].autoSensing;
  root["mode"]            = pedals[p].mode;
  root["pressmode"]       = pedals[p].pressMode;
  root["invertpolarity"]  = pedals[p].invertPolarity;
  root["mapfunction"]     = pedals[p].mapFunction;
  root["expzero"]         = pedals[p].expZero;
  root["expmax"]          = pedals[p].expMax;

  Serial3.write(0xF0);
  root.printTo(Serial3);
  Serial3.write(0xF7);
  Serial3.flush();
}

void serialize_pedals() {

  for (byte p = 0; p < PEDALS; p++)
    serialize_pedal(p);
}

void serialize_interface(byte i = currentInterface) {

  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();

  root["interface"] = i;
  root["in"]        = interfaces[i].midiIn;
  root["out"]       = interfaces[i].midiOut;
  root["thru"]      = interfaces[i].midiThru;
  root["routing"]   = interfaces[i].midiRouting;
  root["clock"]     = interfaces[i].midiClock;

  Serial3.write(0xF0);
  root.printTo(Serial3);
  Serial3.write(0xF7);
  Serial3.flush();
}

void serialize_interfaces() {

  for (byte i = 0; i < INTERFACES; i++)
    serialize_interface(i);
}

void serialize_wifi_credentials(const char *ssid, const char *password) {

  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();

  root["ssid"]      = String(ssid);
  root["password"]  = String(password);
  
  Serial3.write(0xF0);
  root.printTo(Serial3);
  Serial3.write(0xF7);
  Serial3.flush();
}