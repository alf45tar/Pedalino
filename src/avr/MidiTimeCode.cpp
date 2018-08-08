//                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                    /*
// Thanks to https://github.com/adanselm/padchokola
//
#include "MidiTimeCode.h"

// Allow 3 sec between taps at max (eq. to 20BPM)
#define TAP_TIMEOUT_MS 3000

///////////////////////////////////// TapTempo
TapTempo::TapTempo()
{
  reset();
}

TapTempo::~TapTempo()
{
}

void TapTempo::reset()
{
  mLastTap = 0;
  mCurrentReadingPos = 0;
  for ( int i = 0; i < TAP_NUM_READINGS; ++i )
  {
    mReadings[i] = 0.0f;
  }
}

float TapTempo::tap()
{
  const unsigned long currentTime = millis();
  if ( mLastTap > 0 )
  {
    if ( timeout(currentTime) )
      reset();

    mReadings[ mCurrentReadingPos % TAP_NUM_READINGS ] = calcBpmFromTime(currentTime);
    ++mCurrentReadingPos;

    if ( mCurrentReadingPos >= 2 )
    {
      mLastTap = currentTime;
      return computeAverage(); // Enough readings to compute average
    }
  }

  mLastTap = currentTime;
  return 0.0f;
}

bool TapTempo::timeout(const unsigned long currentTime) const
{
  if ( (currentTime - mLastTap) > TAP_TIMEOUT_MS)
    return true;

  return false;
}

float TapTempo::calcBpmFromTime(unsigned long currentTime) const
{
  if ( mLastTap == 0 || currentTime <= mLastTap )
    return 0.0f;

  const float msInAMinute = 1000 * 60.0;
  return msInAMinute / (currentTime - mLastTap);
}

float TapTempo::computeAverage() const
{
  float sum = 0.0f;
  const int count = min(mCurrentReadingPos, TAP_NUM_READINGS);
  for ( int i = 0; i < count; ++i )
  {
    sum += mReadings[i];
  }
  return sum / count;
}

///////////////////////////////////// MidiTimeCode
MidiTimeCode::MidiTimeCode()
{
}

MidiTimeCode::~MidiTimeCode()
{
}

void MidiTimeCode::setup()
{
  // Timer needed in setup even if no synchro occurring
  setTimer(1.0f);
}

void MidiTimeCode::doSendMidiClock()
{
  if ( mEventTime != 0 && (millis() - mEventTime) >= 1 )
  {
    // Reset timer giving slaves time to prepare for playback
    mEventTime = 0;
  }

  if ( mNextEvent != InvalidType )
  {
    mEventTime = millis();
    Serial.write(mNextEvent);
    Serial2.write(mNextEvent);
    Serial3.write(mNextEvent);
    mPlaying = (mNextEvent == Start) || (mNextEvent == Continue);
    mNextEvent = InvalidType;
  }

  if ( mEventTime == 0 )
  {
    Serial.write(Clock);
    Serial2.write(Clock);
    Serial3.write(Clock);
    mClick = (mClick + 1) % MidiTimeCode::mMidiClockPpqn;
    if (mClick == 0) mBeat = (mBeat + 1) % 4;
  }
}

void MidiTimeCode::sendPlay()
{
  noInterrupts();
  mNextEvent = Start;
  mClick = 0;
  mBeat = 0;
  interrupts();
}

void MidiTimeCode::sendStop()
{
  noInterrupts();
  mNextEvent = Stop;
  interrupts();
}

void MidiTimeCode::sendContinue()
{
  noInterrupts();
  mNextEvent = Continue;
  interrupts();
}

void MidiTimeCode::sendPosition(byte hours, byte minutes, byte seconds, byte frames)
{
  noInterrupts();
  setPlayhead(hours, minutes, seconds, frames);
  if (mMode == MidiTimeCode::SynchroMTCMaster) mNextEvent = SongPosition;
  //mCurrentQFrame = 0;
  interrupts();
}

byte MidiTimeCode::getHours()
{
  return mPlayhead.hours;
}

byte MidiTimeCode::getMinutes()
{
  return mPlayhead.minutes;
}

byte MidiTimeCode::getSeconds()
{
  return mPlayhead.seconds;
}

byte MidiTimeCode::getFrames()
{
  return mPlayhead.frames;
}

bool MidiTimeCode::isPlaying() const
{
  return (mNextEvent == Continue) || (mNextEvent == Start) || mPlaying;
}

void MidiTimeCode::doSendMTC()
{
  if ( mNextEvent == SongPosition)
  {
    sendMTCFullFrame();
    mNextEvent = Stop;
    return;
  }
  if ( mNextEvent != Stop )
  {
    if ( mNextEvent == Start)
    {
      resetPlayhead();
      //mCurrentQFrame = 0;
      mClick = 0;
      mBeat = 0;
      mNextEvent = Continue;
    }

    sendMTCQuarterFrame(mCurrentQFrame);
    mCurrentQFrame = (mCurrentQFrame + 1) % 8;

    if (mCurrentQFrame == 0)
      updatePlayhead();
  }
}

void MidiTimeCode::setMode(MidiTimeCode::MidiSynchro newMode)
{
  if ( mMode != newMode )
  {
    mMode = newMode;

    switch (mMode) {

      case MidiTimeCode::SynchroMTCMaster:
        setTimer(24 * 4);
        break;

      default:
        setTimer(1.0f);
        break;
    }

  }
}

MidiTimeCode::MidiSynchro MidiTimeCode::getMode()
{
  return mMode;
}

void MidiTimeCode::decodMTCQuarterFrame(byte MTCData)
{
  static byte b[8] = {0, 0, 0, 0, 0, 0, 0, 0};

  if (getMode() == MidiTimeCode::SynchroMTCSlave) {
    byte i = (MTCData & 0xf0) >> 4;   // MSB
    if (i > 7) return;

    b[i] = MTCData & 0x0f;            // LSB

    if (i == 7)
    {
      byte frameType;

      frameType = b[7] & 0x06;

      byte h = (b[7] & 0x01) << 4 | b[6];
      byte m = b[5] << 4 | b[4];
      byte s = b[3] << 4 | b[2];
      byte f = b[1] << 4 | b[0];

      if (h > 23)  h = 23;
      if (m > 59)  m = 59;
      if (s > 59)  s = 59;
      if (f > 30)  f = 30;

      sendPosition(h, m, s, f);
      for (i = 0; i < 8; i++)
        b[i] = 0;
    }
  }
}

void MidiTimeCode::decodeMTCFullFrame(unsigned size, byte* array)
{
  /// F0 7F cc 01 01 hr mn sc fr F7
  // cc -> channel (0x7f to broadcast)
  // hr -> hour, mn -> minutes, sc -> seconds, fr -> frames
  if (getMode() == MidiTimeCode::SynchroMTCSlave && size == 10)
    if (array[0] == 0xF0 && array[1] == 0x7F && array[2] == 0x7F && array[3] == 0x01 && array[4] == 0x01 && array[9] == 0xF7)
      sendPosition(array[5], array[6], array[7], array[8]);
}

void MidiTimeCode::sendMTCQuarterFrame(int index)
{
  Serial.write(TimeCodeQuarterFrame);
  Serial2.write(TimeCodeQuarterFrame);
  Serial3.write(TimeCodeQuarterFrame);

  byte MTCData = 0;
  switch (mMTCQuarterFrameTypes[index])
  {
    case FramesLow:
      MTCData = mPlayhead.frames & 0x0f;
      break;
    case FramesHigh:
      MTCData = (mPlayhead.frames & 0xf0) >> 4;
      break;
    case SecondsLow:
      MTCData = mPlayhead.seconds & 0x0f;
      break;
    case SecondsHigh:
      MTCData = (mPlayhead.seconds & 0xf0) >> 4;
      break;
    case MinutesLow:
      MTCData = mPlayhead.minutes & 0x0f;
      break;
    case MinutesHigh:
      MTCData = (mPlayhead.minutes & 0xf0) >> 4;
      break;
    case HoursLow:
      MTCData = mPlayhead.hours & 0x0f;
      break;
    case HoursHighAndSmpte:
      MTCData = (mPlayhead.hours & 0xf0) >> 4 | mCurrentSmpteType;
      break;
  }
  Serial.write( mMTCQuarterFrameTypes[index] | MTCData );
  Serial2.write( mMTCQuarterFrameTypes[index] | MTCData );
  Serial3.write( mMTCQuarterFrameTypes[index] | MTCData );
}

void MidiTimeCode::sendMTCFullFrame()
{
  /// F0 7F cc 01 01 hr mn sc fr F7
  // cc -> channel (0x7f to broadcast)
  // hr -> hour, mn -> minutes, sc -> seconds, fr -> frames
  static byte header[5] = { 0xf0, 0x7f, 0x7f, 0x01, 0x01 };
  Serial.write(header, 5);
  Serial.write(mPlayhead.hours);
  Serial.write(mPlayhead.minutes);
  Serial.write(mPlayhead.seconds);
  Serial.write(mPlayhead.frames);
  Serial.write(0xf7);

  Serial2.write(header, 5);
  Serial2.write(mPlayhead.hours);
  Serial2.write(mPlayhead.minutes);
  Serial2.write(mPlayhead.seconds);
  Serial2.write(mPlayhead.frames);
  Serial2.write(0xf7);

  Serial3.write(header, 5);
  Serial3.write(mPlayhead.hours);
  Serial3.write(mPlayhead.minutes);
  Serial3.write(mPlayhead.seconds);
  Serial3.write(mPlayhead.frames);
  Serial3.write(0xf7);
}

// To be called every two frames (so once a complete cycle of quarter frame messages have passed)
void MidiTimeCode::updatePlayhead()
{
  // Compute counter progress
  // update occurring every 2 frames
  mPlayhead.frames += 2;
  mPlayhead.seconds += mPlayhead.frames / 24;
  mPlayhead.frames = mPlayhead.frames % 24;
  mPlayhead.minutes += mPlayhead.seconds / 60;
  mPlayhead.seconds = mPlayhead.seconds % 60;
  mPlayhead.hours += mPlayhead.minutes / 60;
}

void MidiTimeCode::resetPlayhead()
{
  mPlayhead.frames = 0;
  mPlayhead.seconds = 0;
  mPlayhead.minutes = 0;
  mPlayhead.hours = 0;
}

void MidiTimeCode::setPlayhead(byte hours, byte minutes, byte seconds, byte frames)
{
  mPlayhead.frames = frames;
  mPlayhead.seconds = seconds;
  mPlayhead.minutes = minutes;
  mPlayhead.hours = hours;
}

void MidiTimeCode::setBpm(const float iBpm)
{
  if ( getMode() == SynchroClockMaster )
  {
    const double midiClockPerSec = mMidiClockPpqn * iBpm / 60;
    setTimer(midiClockPerSec);
  }
}

const float MidiTimeCode::tapTempo()
{
  static float bpm = 0.0f;

  switch (getMode()) {

    case SynchroClockMaster:
      return mTapTempo.tap();

    case SynchroClockSlave:
      mClick = (mClick + 1) % MidiTimeCode::mMidiClockPpqn;
      if (mClick == 0) {
        mBeat = (mBeat + 1) % 4;
        bpm = mTapTempo.tap();
      }
      return bpm;
  }

  return 0.0f;
}

byte MidiTimeCode::getBeat()
{
  return mBeat;
}

void MidiTimeCode::setTimer(const double frequency)
{
  if (frequency > 244.16f) // First value with cmp_match < 65536 (thus allowing to decrease prescaler for higher precision)
  {
    mPrescaler = 1;
    mSelectBits = (1 << CS10);
  }
  else if (frequency > 30.52f) // First value with cmp_match < 65536
  {
    mPrescaler = 8;
    mSelectBits = (1 << CS11);
  }
  else
  {
    mPrescaler = 64;
    mSelectBits = (1 << CS11) | (1 << CS10);
  }
  const uint16_t cmp_match = 16000000 / (frequency * mPrescaler) - 1 + 0.5f; // (must be < 65536)

  noInterrupts();
  TCCR1A = 0;// set entire TCCR1A register to 0
  TCCR1B = 0;// same for TCCR1B
  TCNT1  = 0;//initialize counter value to 0
  // set compare match register for given increments
  OCR1A = cmp_match;
  // turn on CTC mode
  TCCR1B |= (1 << WGM12);
  // Set CS10 for prescaler 1, CS11 for prescaler 8, and both for prescaler 64
  TCCR1B |= mSelectBits;
  // enable timer compare interrupt
  TIMSK1 |= (1 << OCIE1A);
  interrupts();
}

ISR(TIMER1_COMPA_vect) //timer1 interrupt
{
  if ( MidiTimeCode::getMode() == MidiTimeCode::SynchroMTCMaster )
    MidiTimeCode::doSendMTC();
  else if ( MidiTimeCode::getMode() == MidiTimeCode::SynchroClockMaster )
    MidiTimeCode::doSendMidiClock();
}

int                     MidiTimeCode::mPrescaler = 0;
unsigned char           MidiTimeCode::mSelectBits = 0;

const int               MidiTimeCode::mMidiClockPpqn = 24;
volatile unsigned long  MidiTimeCode::mEventTime = 0;
volatile MidiTimeCode::MidiType MidiTimeCode::mNextEvent = InvalidType;
volatile byte           MidiTimeCode::mClick = 0;
volatile byte           MidiTimeCode::mBeat = 0;
volatile bool           MidiTimeCode::mPlaying = false;


const         MidiTimeCode::SmpteMask MidiTimeCode::mCurrentSmpteType = Frames24;
volatile      MidiTimeCode::Playhead  MidiTimeCode::mPlayhead = MidiTimeCode::Playhead();
volatile int  MidiTimeCode::mCurrentQFrame = 0;
const         MidiTimeCode::MTCQuarterFrameType MidiTimeCode::mMTCQuarterFrameTypes[8] = { FramesLow, FramesHigh,
                                                                                           SecondsLow, SecondsHigh,
                                                                                           MinutesLow, MinutesHigh,
                                                                                           HoursLow, HoursHighAndSmpte
                                                                                         };
MidiTimeCode::MidiSynchro MidiTimeCode::mMode = MidiTimeCode::SynchroNone;
