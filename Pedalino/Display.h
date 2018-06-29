const bool      AUTO_START = true;    // auto start the menu, manual detect and start if false
const uint16_t  MENU_TIMEOUT = 5000;  // in milliseconds

// Function prototypes for Navigation/Display
bool display(MD_Menu::userDisplayAction_t, char* = nullptr);
MD_Menu::userNavAction_t navigation(uint16_t &incDelta);

// Function prototypes for variable get/set functions
MD_Menu::value_t *mnuValueRqst(MD_Menu::mnuId_t id, bool bGet);

#define M_ROOT            10
#define M_BANKSETUP       11
#define M_PEDALSETUP      12
#define M_INTERFACESETUP  13
#define M_PROFILE         14
#define M_OPTIONS         15

#define II_BANK           20
#define II_PEDAL          21
#define II_MIDICHANNEL    22
#define II_MIDIMESSAGE    23
#define II_MIDICODE       24
#define II_MIDINOTE       25
#define II_FUNCTION       26
#define II_AUTOSENSING    27
#define II_MODE           28
#define II_PRESS_MODE     29
#define II_VALUE_SINGLE   30
#define II_VALUE_DOUBLE   31
#define II_VALUE_LONG     32
#define II_POLARITY       33
#define II_CALIBRATE      34
#define II_ZERO           35
#define II_MAX            36
#define II_RESPONSECURVE  37
#define II_INTERFACE      38
#define II_MIDI_IN        39
#define II_MIDI_OUT       40
#define II_MIDI_THRU      41
#define II_MIDI_ROUTING   42
#define II_MIDI_CLOCK     43
#define II_LEGACY_MIDI    44
#define II_PROFILE_LOAD   45
#define II_PROFILE_SAVE   46
#define II_WIFI           47
#define II_DEFAULT        48

// Global menu data and definitions

MD_Menu::value_t vBuf;  // interface buffer for values

// Menu Headers --------
const PROGMEM MD_Menu::mnuHeader_t mnuHdr[] =
{
  { M_ROOT,           SIGNATURE,         10, 13, 0 },
  { M_BANKSETUP,      "Banks Setup",     20, 34, 0 },
  { M_PEDALSETUP,     "Pedals Setup",    40, 52, 0 },
  { M_INTERFACESETUP, "Interface Setup", 60, 65, 0 },
  { M_PROFILE,        "Profiles",        70, 71, 0 },
  { M_OPTIONS,        "Options",         80, 81, 0 },
};

// Menu Items ----------
const PROGMEM MD_Menu::mnuItem_t mnuItm[] =
{
  // Starting (Root) menu
  { 10, "Banks Setup",     MD_Menu::MNU_MENU,  M_BANKSETUP },
  { 11, "Pedals Setup",    MD_Menu::MNU_MENU,  M_PEDALSETUP },
  { 12, "Interface Setup", MD_Menu::MNU_MENU,  M_INTERFACESETUP },
  { 13, "Profiles",        MD_Menu::MNU_MENU,  M_PROFILE },
  { 14, "Options",         MD_Menu::MNU_MENU,  M_OPTIONS },
  // Banks Setup
  { 20, "Select Bank",     MD_Menu::MNU_INPUT, II_BANK },
  { 30, "Select Pedal",    MD_Menu::MNU_INPUT, II_PEDAL },
  { 31, "SetMIDIChannel",  MD_Menu::MNU_INPUT, II_MIDICHANNEL },
  { 32, "SetMIDIMessage",  MD_Menu::MNU_INPUT, II_MIDIMESSAGE },
  { 33, "Set MIDI Code",   MD_Menu::MNU_INPUT, II_MIDICODE },
  { 34, "Set MIDI Note",   MD_Menu::MNU_INPUT, II_MIDINOTE },
  // Pedals Setup
  { 40, "Select Pedal",    MD_Menu::MNU_INPUT, II_PEDAL },
  { 41, "Auto Sensing",    MD_Menu::MNU_INPUT, II_AUTOSENSING },
  { 42, "Set Mode",        MD_Menu::MNU_INPUT, II_MODE },
  { 43, "Set Function",    MD_Menu::MNU_INPUT, II_FUNCTION },
  { 44, "Set Press Mode",  MD_Menu::MNU_INPUT, II_PRESS_MODE },
  { 45, "Set Polarity",    MD_Menu::MNU_INPUT, II_POLARITY },
  { 46, "Calibrate",       MD_Menu::MNU_INPUT, II_CALIBRATE },
  { 47, "Set Zero",        MD_Menu::MNU_INPUT, II_ZERO },
  { 48, "Set Max",         MD_Menu::MNU_INPUT, II_MAX },
  { 49, "Response Curve",  MD_Menu::MNU_INPUT, II_RESPONSECURVE },
  { 50, "Single Press",    MD_Menu::MNU_INPUT, II_VALUE_SINGLE },
  { 51, "Double Press",    MD_Menu::MNU_INPUT, II_VALUE_DOUBLE },
  { 52, "Long Press",      MD_Menu::MNU_INPUT, II_VALUE_LONG },
  // Interface Setup
  { 60, "Select Interf.",  MD_Menu::MNU_INPUT, II_INTERFACE },
  { 61, "MIDI IN",         MD_Menu::MNU_INPUT, II_MIDI_IN },
  { 62, "MIDI OUT",        MD_Menu::MNU_INPUT, II_MIDI_OUT },
  { 63, "MIDI THRU",       MD_Menu::MNU_INPUT, II_MIDI_THRU },
  { 64, "MIDI Routing",    MD_Menu::MNU_INPUT, II_MIDI_ROUTING },
  { 65, "MIDI Clock",      MD_Menu::MNU_INPUT, II_MIDI_CLOCK },
  // Profiles Setup
  { 70, "Load Profile",    MD_Menu::MNU_INPUT, II_INTERFACE },
  { 71, "Save Profile",    MD_Menu::MNU_INPUT, II_MIDI_OUT },
  // Options
  { 80, "WiFi Mode",       MD_Menu::MNU_INPUT, II_WIFI },
  { 81, "Factory default", MD_Menu::MNU_INPUT, II_DEFAULT }
};

// Input Items ---------
const PROGMEM char listMidiMessage[]     = "Program Change| Control Code |  Note On/Off |  Pitch Bend  ";
const PROGMEM char listPedalFunction[]   = "     MIDI     |    Bank +    |    Bank -    |  Start/Pause |     Stop     |     Tap      |     Menu     |    Confirm   |    Escape    |     Next     |   Previous   ";
const PROGMEM char listPedalMode[]       = "   Momentary  |     Latch    |    Analog    |   Jog Wheel  |  Momentary 2 |  Momentary 3 |    Latch 2   ";
const PROGMEM char listPedalPressMode[]  = "    Single    |    Double    |     Long     |      1+2     |      1+L     |     1+2+L    |      2+L     ";
const PROGMEM char listPolarity[]        = " No|Yes";
const PROGMEM char listResponseCurve[]   = "    Linear    |      Log     |   Anti-Log   ";
const PROGMEM char listInterface[]       = "     USB      |  Legacy MIDI |   AppleMIDI  |   Bluetooth  ";
const PROGMEM char listEnableDisable[]   = "   Disable    |    Enable    ";
const PROGMEM char listWiFiMode[] =        " Smart Config | Access Point ";

const PROGMEM MD_Menu::mnuInput_t mnuInp[] =
{
  { II_BANK,          ">1-10:      ", MD_Menu::INP_INT,   mnuValueRqst,  2, 1, 0,  BANKS, 0, 10, nullptr },
#if defined(__AVR_ATmega168__) || defined(__AVR_ATmega328P__)
  { II_PEDAL,         ">1-8:       ", MD_Menu::INP_INT,   mnuValueRqst,  2, 1, 0, PEDALS, 0, 10, nullptr },
#elif defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
  { II_PEDAL,         ">1-16:      ", MD_Menu::INP_INT,   mnuValueRqst,  2, 1, 0, PEDALS, 0, 10, nullptr },
#endif
  { II_MIDICHANNEL,   ">1-16:      ", MD_Menu::INP_INT,   mnuValueRqst,  2, 1, 0,                 16, 0, 10, nullptr },
  { II_MIDIMESSAGE,   ""            , MD_Menu::INP_LIST,  mnuValueRqst, 14, 0, 0,                  0, 0,  0, listMidiMessage },
  { II_MIDICODE,      ""            , MD_Menu::INP_LIST,  mnuValueRqst, 14, 0, 0,                  0, 0,  0, listMidiControlChange },
  { II_MIDINOTE,      ""            , MD_Menu::INP_LIST,  mnuValueRqst, 14, 0, 0,                  0, 0,  0, listMidiNoteNumbers },
  { II_FUNCTION,      ""            , MD_Menu::INP_LIST,  mnuValueRqst, 14, 0, 0,                  0, 0,  0, listPedalFunction },
  { II_AUTOSENSING,   ""            , MD_Menu::INP_LIST,  mnuValueRqst, 14, 0, 0,                  0, 0,  0, listEnableDisable },
  { II_MODE,          ""            , MD_Menu::INP_LIST,  mnuValueRqst, 14, 0, 0,                  0, 0,  0, listPedalMode },
  { II_PRESS_MODE,    ""            , MD_Menu::INP_LIST,  mnuValueRqst, 14, 0, 0,                  0, 0,  0, listPedalPressMode },
  { II_VALUE_SINGLE,  "0-127:     " , MD_Menu::INP_INT,   mnuValueRqst,  3, 0, 0,                127, 0, 10, nullptr },
  { II_VALUE_DOUBLE,  "0-127:     " , MD_Menu::INP_INT,   mnuValueRqst,  3, 0, 0,                127, 0, 10, nullptr },
  { II_VALUE_LONG,    "0-127:     " , MD_Menu::INP_INT,   mnuValueRqst,  3, 0, 0,                127, 0, 10, nullptr },
  { II_POLARITY,      "Invert:    " , MD_Menu::INP_LIST,  mnuValueRqst,  3, 0, 0,                  0, 0,  0, listPolarity },
  { II_CALIBRATE,     "Confirm"     , MD_Menu::INP_RUN,   mnuValueRqst,  0, 0, 0,                  0, 0,  0, nullptr },
  { II_ZERO,          ">0-1023:  "  , MD_Menu::INP_INT,   mnuValueRqst,  4, 0, 0, ADC_RESOLUTION - 1, 0, 10, nullptr },
  { II_MAX,           ">0-1023:  "  , MD_Menu::INP_INT,   mnuValueRqst,  4, 0, 0, ADC_RESOLUTION - 1, 0, 10, nullptr },
  { II_RESPONSECURVE, ""            , MD_Menu::INP_LIST,  mnuValueRqst, 14, 0, 0,                  0, 0,  0, listResponseCurve },
  { II_INTERFACE,     ""            , MD_Menu::INP_LIST,  mnuValueRqst, 14, 0, 0,                  0, 0,  0, listInterface },
  { II_MIDI_IN,       ""            , MD_Menu::INP_LIST,  mnuValueRqst, 14, 0, 0,                  0, 0,  0, listEnableDisable },
  { II_MIDI_OUT,      ""            , MD_Menu::INP_LIST,  mnuValueRqst, 14, 0, 0,                  0, 0,  0, listEnableDisable },
  { II_MIDI_THRU,     ""            , MD_Menu::INP_LIST,  mnuValueRqst, 14, 0, 0,                  0, 0,  0, listEnableDisable },
  { II_MIDI_ROUTING,  ""            , MD_Menu::INP_LIST,  mnuValueRqst, 14, 0, 0,                  0, 0,  0, listEnableDisable },
  { II_MIDI_CLOCK,    ""            , MD_Menu::INP_LIST,  mnuValueRqst, 14, 0, 0,                  0, 0,  0, listEnableDisable },
  { II_PROFILE_LOAD,  "1-9:      "  , MD_Menu::INP_INT,   mnuValueRqst,  1, 0, 1,                  9, 0, 10, nullptr },
  { II_PROFILE_SAVE,  "1-9:      "  , MD_Menu::INP_INT,   mnuValueRqst,  1, 0, 1,                  9, 0, 10, nullptr },
  { II_WIFI,          ""            , MD_Menu::INP_LIST,  mnuValueRqst, 14, 0, 0,                  0, 0,  0, listWiFiMode },
  { II_DEFAULT,       "Confirm"     , MD_Menu::INP_RUN,   mnuValueRqst,  0, 0, 0,                  0, 0,  0, nullptr }
};

// bring it all together in the global menu object
MD_Menu M(navigation, display,        // user navigation and display
          mnuHdr, ARRAY_SIZE(mnuHdr), // menu header data
          mnuItm, ARRAY_SIZE(mnuItm), // menu item data
          mnuInp, ARRAY_SIZE(mnuInp));// menu input data

// Callback code for menu set/get input values

MD_Menu::value_t *mnuValueRqst(MD_Menu::mnuId_t id, bool bGet)
{
  MD_Menu::value_t *r = &vBuf;

  switch (id)
  {
    case II_BANK:
      if (bGet) vBuf.value = currentBank + 1;
      else currentBank = vBuf.value - 1;
      break;

    case II_PEDAL:
      if (bGet) vBuf.value = currentPedal + 1;
      else currentPedal = vBuf.value - 1;
      break;

    case II_MIDICHANNEL:
      if (bGet) vBuf.value = banks[currentBank][currentPedal].midiChannel;
      else banks[currentBank][currentPedal].midiChannel = vBuf.value;
      break;

    case II_MIDIMESSAGE:
      if (bGet) vBuf.value = banks[currentBank][currentPedal].midiMessage;
      else banks[currentBank][currentPedal].midiMessage = vBuf.value;
      break;

    case II_MIDICODE:
    case II_MIDINOTE:
      if (bGet) vBuf.value = banks[currentBank][currentPedal].midiCode;
      else banks[currentBank][currentPedal].midiCode = vBuf.value;
      break;

    case II_FUNCTION:
      if (bGet) vBuf.value = pedals[currentPedal].function;
      else pedals[currentPedal].function = vBuf.value;
      break;

    case II_AUTOSENSING:
      if (bGet) vBuf.value = pedals[currentPedal].autoSensing;
      else pedals[currentPedal].autoSensing = vBuf.value;
      break;

    case II_MODE:
      if (bGet) vBuf.value = pedals[currentPedal].mode;
      else pedals[currentPedal].mode = vBuf.value;
      break;

    case II_PRESS_MODE:
      if (bGet) vBuf.value = pedals[currentPedal].pressMode;
      else pedals[currentPedal].pressMode = vBuf.value;
      break;

    case II_VALUE_SINGLE:
      if (bGet) vBuf.value = pedals[currentPedal].value_single;
      else pedals[currentPedal].value_single = vBuf.value;
      break;

    case II_VALUE_DOUBLE:
      if (bGet) vBuf.value = pedals[currentPedal].value_double;
      else pedals[currentPedal].value_double = vBuf.value;
      break;

    case II_VALUE_LONG:
      if (bGet) vBuf.value = pedals[currentPedal].value_long;
      else pedals[currentPedal].value_long = vBuf.value;
      break;

    case II_POLARITY:
      if (bGet) vBuf.value = pedals[currentPedal].invertPolarity;
      else pedals[currentPedal].invertPolarity = vBuf.value;
      break;

    case II_CALIBRATE:
      if (!bGet && pedals[currentPedal].mode == PED_ANALOG) calibrate();
      r = nullptr;
      break;

    case II_ZERO:
      if (bGet)
        if (pedals[currentPedal].mode == PED_ANALOG)
          vBuf.value = pedals[currentPedal].expZero;
        else r = nullptr;
      else pedals[currentPedal].expZero = vBuf.value;
      break;

    case II_MAX:
      if (bGet)
        if (pedals[currentPedal].mode == PED_ANALOG)
          vBuf.value = pedals[currentPedal].expMax;
        else r = nullptr;
      else pedals[currentPedal].expMax = vBuf.value;
      break;

    case II_RESPONSECURVE:
      if (bGet)
        if (pedals[currentPedal].mode == PED_ANALOG)
          vBuf.value = pedals[currentPedal].mapFunction;
        else r = nullptr;
      else pedals[currentPedal].mapFunction = vBuf.value;
      break;

    case II_INTERFACE:
      if (bGet) vBuf.value = currentInterface;
      else {
        currentInterface = vBuf.value;

      }
      break;

    case II_MIDI_IN:
      if (bGet) vBuf.value = interfaces[currentInterface].midiIn;
      else interfaces[currentInterface].midiIn = vBuf.value;
      break;

    case II_MIDI_OUT:
      if (bGet) vBuf.value = interfaces[currentInterface].midiOut;
      else interfaces[currentInterface].midiOut = vBuf.value;
      break;

    case II_MIDI_THRU:
      if (bGet) vBuf.value = interfaces[currentInterface].midiThru;
      else {
        interfaces[currentInterface].midiThru = vBuf.value;
        interfaces[PED_USBMIDI].midiThru    ? USB_MIDI.turnThruOn() : USB_MIDI.turnThruOff();
        interfaces[PED_LEGACYMIDI].midiThru ? DIN_MIDI.turnThruOn() : DIN_MIDI.turnThruOff();
        interfaces[PED_APPLEMIDI].midiThru  ? RTP_MIDI.turnThruOn() : RTP_MIDI.turnThruOff();
      }
      break;

    case II_MIDI_ROUTING:
      if (bGet) vBuf.value = interfaces[currentInterface].midiRouting;
      else interfaces[currentInterface].midiRouting = vBuf.value;
      break;

    case II_MIDI_CLOCK:
      if (bGet) vBuf.value = interfaces[currentInterface].midiClock;
      else interfaces[currentInterface].midiClock = vBuf.value;
      break;

    case II_LEGACY_MIDI:
      if (bGet) vBuf.value = currentLegacyMIDIPort;
      else currentLegacyMIDIPort = vBuf.value;
      break;

    case II_WIFI:
      if (bGet) vBuf.value = currentWiFiMode;
      else {
        currentWiFiMode = vBuf.value;
        switch (currentWiFiMode) {
          case PED_STA:
            RTP_MIDI.sendSysEx(5, "+STA");
            break;
          case PED_AP:
            RTP_MIDI.sendSysEx(4, "+AP");
            break;
        }
      }
      break;

    case II_DEFAULT:
      if (!bGet) {
        lcd.clear();
        // Sets all of the bytes of the EEPROM to 0.
        for (int i = 0 ; i < EEPROM.length() ; i++) {
          EEPROM.write(i, 0);
          lcd.setCursor(map(i, 0, EEPROM.length(), 0, LCD_COLS - 1), 0);
          lcd.print(char(B10100101));
        }
        Reset_AVR();
      }

    default:
      r = nullptr;
      break;
  }

  if (!bGet) {
    update_eeprom();
    controller_setup();
  }

  return (r);
}

bool display(MD_Menu::userDisplayAction_t action, char *msg)
{
  static char szLine[LCD_COLS + 1] = { '\0' };

  switch (action)
  {
    case MD_Menu::DISP_INIT:
      lcd.begin(LCD_COLS, LCD_ROWS);
      lcd.clear();
      lcd.noCursor();
      memset(szLine, ' ', LCD_COLS);
      break;

    case MD_Menu::DISP_CLEAR:
      lcd.clear();
      break;

    case MD_Menu::DISP_L0:
      lcd.setCursor(0, 0);
      lcd.print(szLine);
      lcd.setCursor(0, 0);
      if (strcmp(msg, "Pedals Setup") == 0) {
        lcd.print("Pedal ");
        if (currentPedal < 9) lcd.print(" ");
        lcd.print(currentPedal + 1);
      }
      else if (strcmp(msg, "Banks Setup") == 0) {
        lcd.print("Bank ");
        if (currentPedal < 9) lcd.print(" ");
        lcd.print(currentBank + 1);
        lcd.print(" Pedal ");
        if (currentPedal < 9) lcd.print(" ");
        lcd.print(currentPedal + 1);
      }
      else if (strcmp(msg, "Interface Setup") == 0) {
        switch (currentInterface) {
          case PED_USBMIDI:
            lcd.print("USB MIDI");
            break;
          case PED_LEGACYMIDI:
            lcd.print("Legacy MIDI");
            break;
          case PED_APPLEMIDI:
            lcd.print("AppleMIDI");
            break;
          case PED_BLUETOOTHMIDI:
            lcd.print("Bluetooth MIDI");
            break;
        }
      }
      else
        lcd.print(msg);
      break;

    case MD_Menu::DISP_L1:
      lcd.setCursor(0, 1);
      lcd.print(szLine);
      lcd.setCursor(0, 1);
      lcd.print(msg);
      break;
  }

  return (true);
}


char foot_char (byte footswitch)
{
  footswitch = constrain(footswitch, 0, PEDALS - 1);
  if (pedals[footswitch].function != PED_MIDI) return ' ';
  if ((footswitch == lastUsedPedal) ||

      (pedals[footswitch].mode == PED_MOMENTARY1 || pedals[footswitch].mode == PED_LATCH1)
      && pedals[footswitch].pedalValue[0] == LOW ||

      (pedals[footswitch].mode == PED_MOMENTARY2 || pedals[footswitch].mode == PED_MOMENTARY3 || pedals[footswitch].mode == PED_LATCH2)
      && (pedals[footswitch].pedalValue[0] == LOW || pedals[footswitch].pedalValue[1] == LOW)) return bar1[footswitch % 10];
  return ' ';
}


void screen_update(bool force = false) {

  static char screen1[LCD_COLS + 1];
  static char screen2[LCD_COLS + 1];

  if (!powersaver) {
    char buf[LCD_COLS + 1];
    // Line 1
    memset(buf, 0, sizeof(buf));
    for (byte i = 0; i < PEDALS; i++) {
      buf[i] = foot_char(i);
    }
    if (force || strcmp(screen1, buf) != 0) {
      memset(screen1, 0, LCD_COLS + 1);
      strncpy(screen1, buf, LCD_COLS);
      lcd.setCursor(0, 0);
      lcd.print(buf);
    }
    // Line 2
    memset(buf, 0, sizeof(buf));
    sprintf(&buf[strlen(buf)], "Bank%2d", currentBank + 1);
    //sprintf(&buf[strlen(buf)], "%2x%2x%2x", currentBank + 1, banks[currentBank][lastUsedSwitch].midiChannel, banks[currentBank][lastUsedSwitch].midiCode);
    if (lastUsedPedal >= 0 && lastUsedPedal < PEDALS) {
      strncpy(&buf[strlen(buf)], &bar2[0], map(pedals[lastUsedPedal].pedalValue[0], 0, MIDI_RESOLUTION - 1, 0, 10));
      strncpy(&buf[strlen(buf)], "          ", 10 - map(pedals[lastUsedPedal].pedalValue[0], 0, MIDI_RESOLUTION - 1, 0, 10));
    }
    if (force || strcmp(screen2, buf) != 0) {
      memset(screen2, 0, LCD_COLS + 1);
      strncpy(screen2, buf, LCD_COLS);
      lcd.setCursor(0, 1);
      lcd.print(buf);
    }
    if (selectBank) {
      lcd.setCursor(5, 1);
      lcd.cursor();
    }
    else
      lcd.noCursor();
  }
}


MD_Menu::userNavAction_t navigation(uint16_t &incDelta)
{
  incDelta = 1;
  static unsigned long  previous_value;
  static byte           count = 0;
  unsigned long         ircode;
  bool                  repeat;
  byte                  numberPressed = 127;

  MD_UISwitch::keyResult_t k1;
  MD_UISwitch::keyResult_t k2;
  byte                     k;

  for (byte i = 0; i < PEDALS; i++) {
    if (pedals[i].function == PED_MIDI) continue;
    k = 0;
    k1 = MD_UISwitch::KEY_NULL;
    k2 = MD_UISwitch::KEY_NULL;
    if (pedals[i].footSwitch[0] != nullptr) k1 = pedals[i].footSwitch[0]->read();
    if (pedals[i].footSwitch[1] != nullptr) k2 = pedals[i].footSwitch[1]->read();
    if ((k1 == MD_UISwitch::KEY_PRESS || k1 == MD_UISwitch::KEY_DPRESS || k1 == MD_UISwitch::KEY_LONGPRESS) && k2 == MD_UISwitch::KEY_NULL) k = 1;
    if ((k2 == MD_UISwitch::KEY_PRESS || k2 == MD_UISwitch::KEY_DPRESS || k2 == MD_UISwitch::KEY_LONGPRESS) && k1 == MD_UISwitch::KEY_NULL) k = 2;
    if ((k1 == MD_UISwitch::KEY_PRESS || k1 == MD_UISwitch::KEY_DPRESS || k1 == MD_UISwitch::KEY_LONGPRESS) &&
        (k2 == MD_UISwitch::KEY_PRESS || k2 == MD_UISwitch::KEY_DPRESS || k2 == MD_UISwitch::KEY_LONGPRESS)) k = 3;
    if (k > 0 && (k1 == MD_UISwitch::KEY_PRESS || k2 == MD_UISwitch::KEY_PRESS)) {
      switch (pedals[i].function) {

        case PED_BANK_PLUS:
          switch (k) {
            case 1:
              if (currentBank < BANKS - 1) currentBank++;
              break;
            case 2:
              if (currentBank > 0) currentBank--;
              break;
            case 3:
              break;
          }
          break;

        case PED_BANK_MINUS:
          switch (k) {
            case 1:
              if (currentBank > 0) currentBank--;
              break;
            case 2:
              if (currentBank < BANKS - 1) currentBank++;
              break;
            case 3:
              break;
          }
          break;

        case PED_START:
          //uClock.start();
          break;
        case PED_STOP:
          //uClock.stop();
          break;
        case PED_TAP:
          break;

        case PED_MENU:
          return MD_Menu::NAV_INC;
          break;
        case PED_CONFIRM:
          return MD_Menu::NAV_SEL;
          break;
        case PED_ESCAPE:
          return MD_Menu::NAV_ESC;
          break;
        case PED_NEXT:
          return MD_Menu::NAV_INC;
          break;
        case PED_PREVIOUS:
          return MD_Menu::NAV_DEC;
          break;
      }
    }

    if (pedals[i].footSwitch[0] != nullptr)
      switch (k1) {
        case MD_UISwitch::KEY_NULL:
          pedals[i].footSwitch[0]->setDoublePressTime(300);
          pedals[i].footSwitch[0]->setLongPressTime(500);
          pedals[i].footSwitch[0]->setRepeatTime(500);
          pedals[i].footSwitch[0]->enableDoublePress(true);
          pedals[i].footSwitch[0]->enableLongPress(true);
          break;
        case MD_UISwitch::KEY_RPTPRESS:
          pedals[i].footSwitch[0]->setDoublePressTime(0);
          pedals[i].footSwitch[0]->setLongPressTime(0);
          pedals[i].footSwitch[0]->setRepeatTime(10);
          pedals[i].footSwitch[0]->enableDoublePress(false);
          pedals[i].footSwitch[0]->enableLongPress(false);
          break;
        case MD_UISwitch::KEY_DPRESS:
          if (pedals[i].function == PED_MENU) return MD_Menu::NAV_SEL;
          break;
        case MD_UISwitch::KEY_LONGPRESS:
          if (pedals[i].function == PED_MENU) return MD_Menu::NAV_ESC;
          break;
      }

    if (pedals[i].footSwitch[1] != nullptr)
      switch (k2) {
        case MD_UISwitch::KEY_NULL:
          pedals[i].footSwitch[1]->setDoublePressTime(300);
          pedals[i].footSwitch[1]->setLongPressTime(500);
          pedals[i].footSwitch[1]->setRepeatTime(500);
          pedals[i].footSwitch[1]->enableDoublePress(true);
          pedals[i].footSwitch[1]->enableLongPress(true);
          break;
        case MD_UISwitch::KEY_RPTPRESS:
          pedals[i].footSwitch[1]->setDoublePressTime(0);
          pedals[i].footSwitch[1]->setLongPressTime(0);
          pedals[i].footSwitch[1]->setRepeatTime(10);
          pedals[i].footSwitch[1]->enableDoublePress(false);
          pedals[i].footSwitch[1]->enableLongPress(false);
          break;
        case MD_UISwitch::KEY_DPRESS:
          if (pedals[i].function == PED_MENU) return MD_Menu::NAV_SEL;
          break;
        case MD_UISwitch::KEY_LONGPRESS:
          if (pedals[i].function == PED_MENU) return MD_Menu::NAV_ESC;
          break;
      }

    if (k1 != MD_UISwitch::KEY_NULL) {
      switch (pedals[i].footSwitch[0]->getKey()) {
        case 'L':
          return MD_Menu::NAV_INC;
          break;
        case 'R':
          return MD_Menu::NAV_DEC;
          break;
      }
    }
  }

  if (irrecv.decode(&results)) {
    ircode = results.value;
    irrecv.resume();
    repeat = (ircode == REPEAT && count > 4);
    if (repeat) ircode = previous_value;
#ifdef DEBUG_PEDALINO
    if (ircode > 0xFFFFFF && ircode != REPEAT) return MD_Menu::NAV_NULL;
    Serial.print("Key pressed: ");
    Serial.println(ircode, HEX);
#endif
    switch (ircode) {
      case 0xFF22DD:
      case IR_RIGHT:
      case IR_DOWN:
        if (!repeat) count = 0;
        previous_value = ircode;
        return MD_Menu::NAV_DEC;

      case 0xFFC23D:
      case IR_LEFT:
      case IR_UP:
        if (!repeat) count = 0;
        previous_value = ircode;
        return MD_Menu::NAV_INC;

      case 0xFF02FD:
      case IR_OK:
        if (!repeat) count = 0;
        previous_value = ircode;
        return MD_Menu::NAV_SEL;

      case 0xFFE21D:
      case IR_ESC:
        if (!repeat) count = 0;
        previous_value = ircode;
        return MD_Menu::NAV_ESC;

      case 0xFFA25D:
      case IR_ON_OFF:
        if (powersaver) {
          lcd.on();
          powersaver = false;
        }
        else {
          lcd.off();
          powersaver = true;
        }
        return MD_Menu::NAV_NULL;

      case REPEAT:
        count++;
        break;

      case IR_KEY_1: // 1
        numberPressed = 1;
        break;
      case IR_KEY_2: // 2
        numberPressed = 2;
        break;
      case IR_KEY_3: // 3
        numberPressed = 3;
        break;
      case IR_KEY_4: // 4
        numberPressed = 4;
        break;
      case IR_KEY_5: // 5
        numberPressed = 5;
        break;
      case IR_KEY_6: // 6
        numberPressed = 6;
        break;
      case IR_KEY_7: // 7
        numberPressed = 7;
        break;
      case IR_KEY_8: // 8
        numberPressed = 8;
        break;
      case IR_KEY_9: // 9
        numberPressed = 9;
        break;
      case IR_KEY_0: // 0
        numberPressed = 10;
        break;

      case IR_SWITCH:
        selectBank = !selectBank;
        break;

      default:
        previous_value = 0;
        break;
    }
  }
  if (numberPressed != 127) {
    if (selectBank) {
      currentBank = numberPressed - 1;
      update_eeprom();
      controller_setup();
    }
    else {
      pedals[numberPressed - 1].pedalValue[0] = LOW;
      //pedals[numberPressed - 1].midiController->push();
      screen_update();
      delay(200);
      pedals[numberPressed - 1].pedalValue[0] = HIGH;
      //pedals[numberPressed - 1].midiController->release();
    }
  }

  if (bluetooth.available()) {
    switch (char(bluetooth.read())) {
      case 'L':
        return MD_Menu::NAV_DEC;
      case 'R':
        return MD_Menu::NAV_INC;
      case 'C':
        return MD_Menu::NAV_SEL;
      case 'E':
        return MD_Menu::NAV_ESC;
    }
  }
  return MD_Menu::NAV_NULL;
}

