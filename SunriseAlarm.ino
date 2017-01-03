// Sunrise Alarm
// Todd Coffey
// Prototype in bedroom 2013-03-26
// 2x 2N3904 transistors connected to pin3 through 2x 10kOhm resistors
// These transistors are connected to 4x white LEDs through 100Ohm resistors to 5V.
#include "TimerOne.h"
#include <Wire.h>
#include "Adafruit_LEDBackpack.h"
#include "Adafruit_GFX.h"
#include "RTClib.h"
#include "AT24Cxx.h"
#include <ClickEncoder.h>
#include <Adafruit_NeoPixel.h>
#include <SoftwareSerial.h>
#include "Adafruit_Soundboard.h"

// 2013-04-06 TODO:  Add a temperature sensor to the lamp and if it exceeds a particular temperature, then turn it off or down
// 2013-04-06 TODO:  Add three LEDs inside the enclosure to indicate which mode you are in.  E.g. green for alarm, blue for time, and red for sundown.
// 2013-04-06 DONE 2013-04-06:  Store SunSet alarm setting in EEPROM
// 2013-04-06 DONE 2013-04-06:  Store brigness level in EEPROM
// 2013-04-06 DONE 2013-04-06:  Change Alarm, Time, and Sundown modes so they will automatically exit the mode after 60 seconds of inactivity.
// 2013-04-01 TODO:  Box enclosure:  Consider a OLED display behind a thin veneer of wood or paper so that the display can print
//                   the numbers in a particular font, e.g. the old Russian Nixie tube font, or a Victorian font.
//                   Also:  Change all the switches to flip switches or knobs for a more retro feel.
//                   Make sure each of the switches & knobs feels different so you can find them in the dark.
//                   Alternative to the OLED display:  Invest in nixie tubes
// 2013-04-01 TODO:  Change "OffButton" to allow you to see how much time is left and reset it
// 2013-04-01 TODO:  Rework SunRISE alarm to match the way the SunSET alarm works.  Basically, use millis() instead of clock
// DONE 2013-03-29:  Add alarm & time mode so you don't have to hold down the buttons.
// DONE 2013-03-30:  Add SunSet mode
// WONTFIX 2013-03-29:  Add AM/PM indicator
// DONE 2013-03-29:  Add method to toggle alarm on/off
// DONE 2013-03-28:  Add hardware buttons for setting time and alarm
// TODO:  Add audio:  chirping birds at alarm time
// TODO:  Add snooze button to delay chirping for 7 minutes or so
// DONE 2013-04-17:  Change light alarm so it is based on millis() rather than RTC
// TODO:  Get light updated more often, especially in third slope of alarm.

namespace SunriseAlarm {

struct ClockTime {
  ClockTime() : hour(0), minute(0), second(0) {}
  ClockTime(int h, int m) : hour(h), minute(m), second(0) {}
  ClockTime(int h, int m, int s) : hour(h), minute(m), second(s) {}
  ClockTime(DateTime dt) : hour(dt.hour()), minute(dt.minute()), second(dt.second()) {}
  ClockTime(const ClockTime & ct) : hour(ct.hour), minute(ct.minute), second(ct.second) {}

  ClockTime operator++(int) {
    ClockTime result(*this);
    ++this->minute;
    fixTime();
    return result;
  }

  ClockTime operator+=(int16_t delta)
  {
    ClockTime result(*this);
    this->minute += delta;
    fixTime();
    return result;
  }

  ClockTime operator-=(int16_t delta)
  {
    ClockTime result(*this);
    this->minute -= delta;
    fixTime();
    return result;
  }

  ClockTime operator--(int) {
    ClockTime result(*this);
    --this->minute;
    fixTime();
    return result;
  }

  ClockTime & operator++() {
    ++this->minute;
    fixTime();
    return *this;
  }

  ClockTime & operator--() {
    --this->minute;
    fixTime();
    return *this;
  }

  void fixTime() {
    while (this->second > 59) {
      ++this->minute;
      this->second -= 60;
    }
    while (this->second < 0) {
      --this->minute;
      this->second += 60;
    }

    while (this->minute > 59) {
      ++this->hour;
      this->minute -= 60;
    }
    while (this->minute < 0) {
      --this->hour;
      this->minute += 60;
    }

    while (this->hour > 23) {
      this->hour -= 24;
    }
    while (this->hour < 0) {
      this->hour += 24;
    }
  }

  int hour;
  int minute;
  int second;
};


ClockTime operator+(ClockTime lhs, int minutes) {
  lhs.minute += minutes;
  lhs.fixTime();
  return lhs;
}


bool operator<(ClockTime lhs, ClockTime rhs) {
  if (lhs.hour < rhs.hour) {
    return true;
  }
  if (lhs.hour > rhs.hour) {
    return false;
  }
  // lhs.hour == rhs.hour
  if (lhs.minute < rhs.minute) {
    return true;
  }
  if (lhs.minute > rhs.minute) {
    return false;
  }
  // lhs.minute == rhs.minute
  if (lhs.second < rhs.second) {
    return true;
  }
  return false;
}

bool operator<=(ClockTime lhs, ClockTime rhs) {
  if (lhs.hour < rhs.hour) {
    return true;
  }
  if (lhs.hour > rhs.hour) {
    return false;
  }
  // lhs.hour == rhs.hour
  if (lhs.minute < rhs.minute) {
    return true;
  }
  if (lhs.minute > rhs.minute) {
    return false;
  }
  // lhs.minute == rhs.minute
  if (lhs.second <= rhs.second) {
    return true;
  }
  return false;
}

bool operator==(ClockTime lhs, ClockTime rhs) {
  if ((lhs.hour == rhs.hour) && (lhs.minute == rhs.minute) && (lhs.second == rhs.second)) {
    return true;
  }
  return false;
}

bool operator!=(ClockTime lhs, ClockTime rhs) {
  return !(lhs == rhs);
}


ClickEncoder *encoder;
#define ENCODER_PIN0 11
#define ENCODER_PIN1 12


#define AlarmButton 7
#define AlarmLED 5
#define TimeButton 9
#define TimeLED 10
#define OffButton A0
#define alarmMasterSwitch 8
#define RotaryButton 4
#define soundAPin A1
#define soundBPin A2
bool soundAlarmAPlaying = false;
bool soundAlarmBPlaying = false;

#define NUM_LED 32
#define LED_PIN 6
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LED, LED_PIN, NEO_GRB + NEO_KHZ800);

bool alarmMasterSwitchEnabled = true;
bool alarmEnabled = true;
bool alarmActive = false;
bool snoozeActive = false;

//TimerOne timer;
const int16_t maxBright = 1024; // resolution of TimerOne::pwm (10 bits)
const int thirtyMinutes = 30;
const int16_t thirtyMinutesInSeconds = 1800; // 1800 seconds = 30 minutes
//const int timerPeriod = 1000; // 1000us = 1 kHz flicker
const int16_t maxTime = 3600; // 1.0 hours
const int forward_delay = 15;
const int rewind_delay = 150;
const uint32_t millisecondsIn30Minutes = 1800000;

int defaultSunSetDelta = 10; // minutes
int sunSetDimLevel = 100; // 100 = max bright, 0 = off

const uint32_t modeInactivePeriod = 30000; // 30 seconds

AT24Cxx eeprom;
// This is the time you want to get up.  Lights will start 30 minutes before.
ClockTime alarmTime(6, 00);
ClockTime startTime(5, 30);

const int alarm_address = 4;
const int sunset_address = 6;
const int brightness_address = 7;
const int sunset_dim_address = 8;

Adafruit_7segment matrix = Adafruit_7segment();
int matrixBrightness = 0;

// seven segment display:
//      A
//    -----
//   |     |
// F |     | B
//   |  G  |
//    -----
//   |     |
// E |     | C
//   |     |
//    -----
//      D   o P
// Turn on bits using B notation:
// BPGFEDCBA
// E.g. to turn on a capital C letter without the dot, use:
// B00111001

uint8_t raw_C = B00111001;
uint8_t raw_l = B00110000;
uint8_t raw_o = B01011100;
uint8_t raw_c = B01011000;
uint8_t raw_A = B01110111;
uint8_t raw_L = B00111000;
uint8_t raw_r = B01010000;
uint8_t raw_t = B01111000;
uint8_t raw_d = B01011110;
uint8_t dot_bit = B10000000;
uint8_t raw_S = B01101101;
uint8_t raw_F = B01110001;

RTC_DS3231 RTC;
uint32_t synchronized_clock_millis;
ClockTime synchronized_clock_time;
ClockTime current_clock_time;

void timerIsr() {
  encoder->service();
}


bool readAlarmTimeFromEEPROM() {
  if (eeprom.isPresent()) {
    char buf[2];
    eeprom.ReadMem(alarm_address, buf, 2);
    alarmTime.hour = static_cast<int>(buf[0]);
    alarmTime.minute = static_cast<int>(buf[1]);
    return true;
  }
  return false;
}

bool writeAlarmTimeToEEPROM() {
  if (eeprom.isPresent()) {
    char myBuf[2];
    myBuf[0] = static_cast<char>(alarmTime.hour);
    myBuf[1] = static_cast<char>(alarmTime.minute);
    eeprom.WriteMem(alarm_address, myBuf, 2);
    return true;
  }
  return false;
}

bool readBrightnessFromEEPROM() {
  if (eeprom.isPresent()) {
    char buf[1];
    eeprom.ReadMem(brightness_address, buf, 1);
    matrixBrightness = static_cast<int>(buf[0]);
    return true;
  }
  return false;
}

bool writeBrightnessToEEPROM() {
  if (eeprom.isPresent()) {
    char myBuf[1];
    myBuf[0] = static_cast<char>(matrixBrightness);
    eeprom.WriteMem(brightness_address, myBuf, 1);
    return true;
  }
  return false;
}


bool readSunSetDeltaFromEEPROM() {
  if (eeprom.isPresent()) {
    char buf[1];
    eeprom.ReadMem(sunset_address, buf, 1);
    defaultSunSetDelta = static_cast<int>(buf[0]);
    return true;
  }
  return false;
}

bool writeSunSetDeltaToEEPROM() {
  if (eeprom.isPresent()) {
    char myBuf[1];
    myBuf[0] = static_cast<char>(defaultSunSetDelta);
    eeprom.WriteMem(sunset_address, myBuf, 1);
    return true;
  }
  return false;
}

bool readSunSetDimFromEEPROM() {
  if (eeprom.isPresent()) {
    char buf[1];
    eeprom.ReadMem(sunset_dim_address, buf, 1);
    sunSetDimLevel = static_cast<int>(buf[0]);
    return true;
  }
  return false;
}

bool writeSunSetDimToEEPROM() {
  if (eeprom.isPresent()) {
    char myBuf[1];
    myBuf[0] = static_cast<char>(sunSetDimLevel);
    eeprom.WriteMem(sunset_dim_address, myBuf, 1);
    return true;
  }
  return false;
}



int debounceInterval = 30;
int debounceDigitalRead(int pin) {
  int lastResult = digitalRead(pin);
  uint32_t lastRead = millis();
  while (static_cast<uint32_t>(millis() - lastRead) < debounceInterval) {
    int result = digitalRead(pin);
    if (result != lastResult) {
      lastResult = result;
      lastRead = millis();
    }
  }
  return lastResult;
}

uint8_t computeMinRGBLevel(uint8_t* RGB)
{
  uint8_t myRGB[3] = {255, 255, 255};
  for (uint8_t i = 0 ; i < 3 ; ++i)
  {
    if (RGB[i] > 0) myRGB[i] = RGB[i];
  }
  return min(min(myRGB[0], myRGB[1]), myRGB[2]);
}


const uint8_t candleLight[3] = {255, 147, 41};
const uint8_t sunLight[3] = {255, 255, 255};
const uint8_t tungsten100W[3] = {255, 214, 170}; 
const uint8_t halogen[3] = {255, 241, 224};
const uint8_t testingLight[3] = {50, 0, 0};
uint8_t targetColor[3] = {255, 0, 0};
const uint8_t sunRiseDimLevel = 90; // Percentage of max [0, 100]

uint8_t minRGBLevel = computeMinRGBLevel(targetColor);
uint16_t totalDimmerSteps = minRGBLevel * NUM_LED;

void setColorForSunSet()
{
  for (uint8_t i = 0 ; i < 3 ; ++i)
  {
    targetColor[i] = (sunSetDimLevel*1.0/100.0)*(1.0*candleLight[i]);
  }
  minRGBLevel = computeMinRGBLevel(targetColor);
  totalDimmerSteps = minRGBLevel * NUM_LED;
}

void setColorForSunRise()
{
  for (uint8_t i = 0 ; i < 3 ; ++i)
  {
    targetColor[i] = (sunRiseDimLevel*1.0/100.0)*(1.0*tungsten100W[i]);
  }
  minRGBLevel = computeMinRGBLevel(targetColor);
  totalDimmerSteps = minRGBLevel * NUM_LED;
}

uint16_t currentDimmerStep = 0;
uint8_t baseRGBColor[3] = {0u, 0u, 0u};
uint8_t highRGBColor[3] = {0u, 0u, 0u};

// step range from 0 to minRGBLevel*NUM_LED
void linearBrightOfStep(uint16_t step)
{
  currentDimmerStep = step;
  uint16_t levelForAllLEDs = step / NUM_LED;
  uint8_t numLEDsAtHigherLevel = step % NUM_LED;
  for (uint8_t i = 0 ; i < 3 ; ++i)
  {
    baseRGBColor[i] = (targetColor[i] * levelForAllLEDs) / minRGBLevel;
    highRGBColor[i] = (targetColor[i] * (levelForAllLEDs + 1)) / minRGBLevel;
  }
  for (uint8_t i = 0 ; i < NUM_LED ; ++i)
  {
    if (i < numLEDsAtHigherLevel)
    {
      strip.setPixelColor(i, highRGBColor[0], highRGBColor[1], highRGBColor[2]);
    }
    else
    {
      strip.setPixelColor(i, baseRGBColor[0], baseRGBColor[1], baseRGBColor[2]);
    }
  }
  strip.show();
}

uint32_t linearBrightOfMilliseconds(uint32_t milliseconds)
{
  uint16_t stepNumber = (totalDimmerSteps * milliseconds) / millisecondsIn30Minutes;
  linearBrightOfStep(stepNumber);
}

void logisticBrightOfStep(uint16_t step)
{
  float p = ((float)step) / ((float)totalDimmerSteps);
  float L = 1.0;
  float k = 20.0;
  float x0 = 0.5;
  float logisticP = L / (1 + exp(-k * (p - x0)));
  uint16_t logisticStep = logisticP * totalDimmerSteps;
  linearBrightOfStep(logisticStep);
}

void logisticBrightOfMilliseconds(uint32_t milliseconds)
{
  uint32_t stepNumber = ((uint32_t)(totalDimmerSteps) * milliseconds) / millisecondsIn30Minutes;
  logisticBrightOfStep(stepNumber);
}

void turnLightOn() {
  int16_t delta = max(1, totalDimmerSteps / 500);
  for (int16_t i = currentDimmerStep; i < totalDimmerSteps; i += delta) {
    linearBrightOfStep(i);
    delay(1);
  }
  linearBrightOfStep(totalDimmerSteps);
}

void turnLightOff() {
  int16_t delta = max(1, totalDimmerSteps / 500);
  for (int16_t i = currentDimmerStep; i > 0 ; i -= delta) {
    linearBrightOfStep(i);
    delay(1);
  }
  linearBrightOfStep(0u);
}

void waitForButtonDepress(int pin, uint8_t onState) {
  while (debounceDigitalRead(pin) == onState) {
    delay(debounceInterval);
  }
}




// DONE:  Update to display 12 hr time
void updateTimeDisplay(ClockTime t, bool military, bool dots) {
  matrix.print(0);
  int hour = t.hour;
  if (!military) {
    hour = (t.hour > 12) ? t.hour - 12 : t.hour ;
    if (hour == 0) {
      hour += 12;
    }
  }
  int digit0 = hour / 10;
  int digit1 = hour % 10;
  int digit3 = t.minute / 10;
  int digit4 = t.minute % 10;
  if (digit0 > 0) {
    matrix.writeDigitNum(0, digit0, dots);
  }
  matrix.writeDigitNum(1, digit1, dots);
  matrix.writeDigitNum(2, 1); // colon
  matrix.writeDigitNum(3, digit3, dots);
  matrix.writeDigitNum(4, digit4, dots);
  matrix.writeDisplay();
}

// wait is in milliseconds
// last is the last time this function returned true
bool isTimeNow(uint32_t & last, unsigned wait) {
  uint32_t time = millis();
  if (static_cast<uint32_t>(time - last) > wait) {
    last = time;
    return true;
  }
  return false;
}

const uint32_t updateInterval = 1000; // every second
uint32_t lastUpdate = millis();
void updateCurrentTime(bool force = false) {
  if (force || isTimeNow(lastUpdate, updateInterval)) {
    current_clock_time = synchronized_clock_time;
    const uint32_t seconds_delta = static_cast<uint32_t>(millis() - synchronized_clock_millis) / 1000;
    const uint32_t minutes_delta = seconds_delta / 60;
    current_clock_time.minute += minutes_delta;
    current_clock_time.second += (seconds_delta - minutes_delta * 60);
    current_clock_time.fixTime();
    updateTimeDisplay(current_clock_time, false, alarmMasterSwitchEnabled);
  }
}

const uint32_t clockUpdateInterval = 24 * 60 * 60 * 1000; // every 24 hours
uint32_t lastClockUpdate = millis();
void updateClockTime(bool force = false) {
  if (force || isTimeNow(lastClockUpdate, clockUpdateInterval)) {
    synchronized_clock_time = ClockTime(RTC.now());
    synchronized_clock_millis = millis();
    current_clock_time = synchronized_clock_time;
  }
}

void updateStartTime() {
  startTime = alarmTime;
  startTime -= thirtyMinutes;
}


void display_Cloc() {
  matrix.clear();
  matrix.writeDigitRaw(0, raw_C | dot_bit);
  matrix.writeDigitRaw(1, raw_l | dot_bit);
  matrix.writeDigitRaw(3, raw_o | dot_bit);
  matrix.writeDigitRaw(4, raw_c | dot_bit);
  matrix.writeDisplay();
}



void display_ALAr() {
  matrix.clear();
  matrix.writeDigitRaw(0, raw_A | dot_bit);
  matrix.writeDigitRaw(1, raw_L | dot_bit);
  matrix.writeDigitRaw(3, raw_A | dot_bit);
  matrix.writeDigitRaw(4, raw_r | dot_bit);
  matrix.writeDisplay();
}

void display_todd() {
  matrix.setBrightness(15);
  matrix.clear();
  matrix.writeDigitRaw(0, raw_t);  
  matrix.writeDigitRaw(1, raw_o);
  matrix.writeDigitRaw(3, raw_d);
  matrix.writeDigitRaw(4, raw_d);
  matrix.writeDisplay();
  delay(5000);
}

void display_sfc() {
  matrix.setBrightness(15);
  matrix.clear();
  matrix.writeDigitRaw(1, raw_S);
  matrix.writeDigitRaw(3, raw_F);
  matrix.writeDigitRaw(4, raw_C);
  matrix.writeDisplay();
  delay(5000);  
}

void setup() {
  Wire.begin();
  RTC.begin();
  pinMode(AlarmButton, INPUT_PULLUP);
  pinMode(TimeButton, INPUT_PULLUP);
  pinMode(alarmMasterSwitch, INPUT_PULLUP);
  pinMode(RotaryButton, INPUT_PULLUP);
  pinMode(OffButton, INPUT);
  pinMode(AlarmLED, OUTPUT);
  pinMode(TimeLED, OUTPUT);
  pinMode(soundAPin, OUTPUT);
  pinMode(soundBPin, OUTPUT);
  digitalWrite(AlarmLED, LOW);
  digitalWrite(TimeLED, LOW);
  digitalWrite(soundAPin, HIGH);
  digitalWrite(soundBPin, HIGH);
  alarmMasterSwitchEnabled = (digitalRead(alarmMasterSwitch) == HIGH);
  strip.begin();
  for (int i=0 ; i<16 ; ++i) { strip.setPixelColor(i,0); }
  strip.show(); // Initialize all pixels to 'off'
  encoder = new ClickEncoder(ENCODER_PIN0, ENCODER_PIN1, -1, 4);
  Timer1.initialize(1000);
  Timer1.attachInterrupt(timerIsr);
  while (!eeprom.isPresent()) {
    delay(1);
  }
  matrix.begin(0x70);
  // Play an easter-egg message while displaying my name...
  display_sfc();
  display_todd();
  readBrightnessFromEEPROM();
  matrix.setBrightness(matrixBrightness); // 0-15
  readSunSetDeltaFromEEPROM();
  readSunSetDimFromEEPROM();
  updateClockTime(true);
  updateCurrentTime(true);
  readAlarmTimeFromEEPROM();
  updateStartTime();
}

int32_t secondsBtwDates(ClockTime currentTime, ClockTime alarm) {
  int32_t s = (currentTime.hour - alarm.hour);
  s *= 3600;
  s += (currentTime.minute - alarm.minute) * 60;
  s += (currentTime.second - alarm.second);
  return ( s );
}

void soundAlarmA(bool status) {
  digitalWrite(soundAPin, (status ? LOW : HIGH));
  soundAlarmAPlaying = status;
}

void soundAlarmB(bool status) {
  digitalWrite(soundBPin, (status ? LOW : HIGH));
  soundAlarmBPlaying = status;
}

uint32_t alarmStartMillis = 0;
const unsigned lightUpdateInterval = 50;
uint32_t lastLightUpdate = millis();
void updateAlarmLight() {
  if (!isTimeNow(lastLightUpdate, lightUpdateInterval)) {
    return;
  }
  if (!alarmMasterSwitchEnabled) return;
  ClockTime current = current_clock_time;
  int32_t seconds = secondsBtwDates(current, startTime);
  if ((seconds < 0) || (seconds >= maxTime)) {
    if (alarmActive) {
      soundAlarmA(false);
      alarmActive = false;
    }
    snoozeActive = false;
    alarmEnabled = true;
    return;
  }
  if (!alarmEnabled) {
    return;
  }
  if (!alarmActive)
  {
    setColorForSunRise();
    alarmStartMillis = static_cast<uint32_t>(millis() - 1000 * seconds);
    alarmActive = true;
  }
  if (seconds >= thirtyMinutesInSeconds) {
    linearBrightOfStep(totalDimmerSteps);
    if (!snoozeActive) soundAlarmA(true);
    return;
  }
  uint32_t debug_millis = static_cast<uint32_t>(millis() - alarmStartMillis);
  logisticBrightOfMilliseconds(debug_millis);
}


void updateTime(ClockTime & t, int16_t delta) {
  t += delta;
  updateTimeDisplay(t, true, alarmMasterSwitchEnabled);
}

void updateAlarm(int16_t delta) {
  alarmTime += delta;
  updateTimeDisplay(alarmTime, true, alarmMasterSwitchEnabled);
}

void changeState_Clock_to_setTime()
{
  analogWrite(TimeLED, matrixBrightness * 255 / 16 + 15);
  display_Cloc();
  delay(1000);
  DateTime currentTime = RTC.now();
  ClockTime t(currentTime);
  updateTimeDisplay(t, true, alarmMasterSwitchEnabled);
  waitForButtonDepress(TimeButton, LOW);
}

void changeState_setTime_to_Clock()
{
  updateCurrentTime(true);
  digitalWrite(TimeLED, LOW);
}

void setTimeState() {
  chnageState_Clock_to_setTime();
  
  uint32_t modeActive = millis();
  bool normalExit = true;
  while (!timeButtonPushed()) {
    int16_t encoderDelta = encoder->getValue();
    if (encoderDelta != 0) {
      updateTime(t, encoderDelta);
      modeActive = millis();
    }
    if (static_cast<uint32_t>(millis() - modeActive) > modeInactivePeriod) {
      normalExit = false;
      break;
    }
  }
  if (normalExit) {
    waitForButtonDepress(TimeButton, LOW);
  }
  updateTimeDisplay(t, false, alarmMasterSwitchEnabled);
  RTC.adjust(
    DateTime( currentTime.year(),
              currentTime.month(),
              currentTime.day(),
              t.hour,
              t.minute,
              0
            )
  );
  synchronized_clock_time = t;
  synchronized_clock_millis = millis();
  current_clock_time = synchronized_clock_time;

  changeState_setTime_to_Clock();
}

void changeState_Clock_to_setAlarm()
{
  analogWrite(AlarmLED, matrixBrightness * 255 / 16 + 1);
  display_ALAr();
  delay(1000);
  updateTimeDisplay(alarmTime, true, alarmMasterSwitchEnabled);
  waitForButtonDepress(AlarmButton, LOW);  
}

void changeState_setAlarm_to_Clock()
{
  updateStartTime();
  writeAlarmTimeToEEPROM();
  
  updateCurrentTime(true);
  updateTimeDisplay(current_clock_time, false, alarmMasterSwitchEnabled);
  digitalWrite(AlarmLED, LOW);
  alarmEnabled = true;
  snoozeActive = false;
}

void setAlarmState() {
  changeState_Clock_to_setAlarm();
  
  uint32_t modeActive = millis();
  bool normalExit = true;
  while (!alarmButtonPushed()) {
    int16_t encoderDelta = encoder->getValue();
    if (encoderDelta != 0) {
      updateAlarm(encoderDelta);
      modeActive = millis();
    }
    if (static_cast<uint32_t>(millis() - modeActive) > modeInactivePeriod) {
      normalExit = false;
      break;
    }
  }
  if (normalExit) {
    waitForButtonDepress(AlarmButton, LOW);
  }
  
  changeState_setAlarm_to_Clock();
}

void changeState_Clock_to_setSunset()
{
  setColorForSunSet();
  matrix.print(sunSetDimLevel);
  if (sunSetDimLevel == 0) {
    matrix.writeDigitNum(4, 0);
  }
  matrix.writeDisplay();
  sunSetActive = true;
  turnLightOn();
  waitForButtonDepress(RotaryButton, LOW);
}

void changeState_setSunset_to_Clock()
{
  if (sunSetDimLevel == 0) {
    sunSetActive = false;
    turnLightOff();
  }
  writeSunSetDimToEEPROM();
}

uint32_t millisAtStartOfSunSet = 0;
bool sunSetActive = false;
void setSunSetState() {
  changeState_Clock_to_setSunset();
  
  uint32_t modeActive = millis();
  bool normalExit = true;
  while (!rotaryButtonPushed()) {
    int16_t encoderDelta = encoder->getValue();
    if (encoderDelta != 0)
    {
      sunSetDimLevel += encoderDelta;
      sunSetDimLevel = min(100,max(0, sunSetDimLevel));
      matrix.print(sunSetDimLevel);
      if (sunSetDimLevel == 0) matrix.writeDigitNum(4, 0);
      matrix.writeDisplay();
      setColorForSunSet();
      turnLightOn();
      modeActive = millis();
    }
    if (static_cast<uint32_t>(millis() - modeActive) > modeInactivePeriod) {
      normalExit = false;
      break;
    }
  }
  if (normalExit) {
    waitForButtonDepress(RotaryButton, LOW);
  }
  millisAtStartOfSunSet = millis();
  
  changeState_setSunset_to_Clock();
}


uint32_t lastSunSetUpdate = millis();
unsigned sunSetUpdateInterval = 5;
int lastSunSetLightLevel = 0;
void updateSunsetLight() {
  if (!sunSetActive) {
    return;
  }
  if (!isTimeNow(lastSunSetUpdate, sunSetUpdateInterval)) {
    return;
  }
  uint32_t currentMillis = millis();
  uint32_t delta = static_cast<uint32_t>(currentMillis - millisAtStartOfSunSet);
  uint32_t finalMillis = defaultSunSetDelta;
  finalMillis *= 60;
  finalMillis *= 1000;
  if (delta > finalMillis) {
    sunSetActive = false;
    linearBrightOfStep(0);
  }
  else {
    uint32_t temp = totalDimmerSteps - totalDimmerSteps * delta / finalMillis;
    logisticBrightOfStep(temp);
  }
}


bool alarmButtonPushed()
{
  return (digitalRead(AlarmButton) == LOW && debounceDigitalRead(AlarmButton) == LOW);
}

bool timeButtonPushed()
{
  return (digitalRead(TimeButton) == LOW && debounceDigitalRead(TimeButton) == LOW);
}
bool offButtonPushed()
{
  return (digitalRead(OffButton) == HIGH && debounceDigitalRead(OffButton) == HIGH);
}

bool rotaryButtonPushed()
{
  return (digitalRead(RotaryButton) == LOW && debounceDigitalRead(RotaryButton) == LOW);
}

void turnOffLightsDueToOffButton()
{
    if (alarmActive && soundAlarmAPlaying) {
      soundAlarmA(false);
      snoozeActive = true;
    }
    else if (alarmActive) {
      alarmActive = false;
      alarmEnabled = false;
      soundAlarmA(false);
      snoozeActive = false;
      turnLightOff();
    }
    else if (sunSetActive) {
      sunSetActive = false;
      turnLightOff();
    }
    else {
      // do nothing
    }
    waitForButtonDepress(OffButton, HIGH);
}

void processAlarmMasterSwitch()
{
  if (digitalRead(alarmMasterSwitch) == LOW) {
    alarmMasterSwitchEnabled = false;
    if (alarmEnabled && alarmActive) {
      alarmActive = false;
      alarmEnabled = false;
      turnLightOff();
      soundAlarmA(false);
      snoozeActive = false;
    }
  }
  else {
    alarmMasterSwitchEnabled = true;
  }
}

void loop() {
  if (alarmButtonPushed()) setAlarmState();
  else if (timeButtonPushed()) setTimeState();
  else if (offButtonPushed()) turnOffLightsDueToOffButton();
  else if (rotaryButtonPushed()) setSunSetState();

  processAlarmMasterSwitch();
  updateAlarmLight();
  updateCurrentTime();
  updateClockTime();
  updateSunsetLight();
}

} // namespace SunriseAlarm

void setup()
{
  SunriseAlarm::setup();
}

void loop()
{
  SunriseAlarm::loop();
}

void 
