// Sunrise Alarm
// Todd Coffey
// Prototype in bedroom 2013-03-26
// 2x 2N3904 transistors connected to pin3 through 2x 10kOhm resistors
// These transistors are connected to 4x white LEDs through 100Ohm resistors to 5V.
#include <avr/pgmspace.h>
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
#include "ClockTime.h"

#define AlarmButton 7
#define AlarmLED 5
#define TimeButton 9
#define TimeLED 10
#define TouchSensor A0
#define alarmMasterSwitch 8
#define RotaryButton 4
#define soundAPin A1
#define soundBPin A2
#define ENCODER_PIN0 11
#define ENCODER_PIN1 12
#define NEOPIXEL_PIN 6

#define NUM_LED 32
#define SNOOZE_MINUTES 5
#define ALARM_DELTA 30 // 30 minutes before and after alarmTime
#define ALARM_DELTA_MILLIS 1800000 // 30 minutes in milliseconds
#define MODE_INACTIVE_PERIOD_MILLIS 30000 // 30 seconds
#define ALARM_ADDRESS 4
#define SUNSET_DIM_ADDRESS 8

#define DEBUG 1

namespace SunriseAlarm {
// Declarations
void updateClockDisplayNow();
void updateClockDisplayOneSecondAfterLastTime();
void updateTimeFromRTCNow();
void updateTimeFromRTC24HoursAfterLastTime();
void turnLightOn();
void turnLightOff();
void alarmAMusicOn();
void alarmAMusicOff();
void display_Cloc();
void display_ALAr();
void updateTimeDisplay(ClockTime t, bool military, bool dots);
void updateAlarmLight();
void updateAlarm(int16_t delta);
bool writeSunsetDimToEEPROM();
void updateTime(ClockTime & t, int16_t delta);
void updateAlarmStartEndTime();
bool writeAlarmTimeToEEPROM();
void updateSunsetLight();

// Globals
bool alarmMasterSwitchEnabled = true;
int matrixBrightness = 0;
// This is the time you want to get up.  Lights will start 30 minutes before.
ClockTime alarmTime     (6, 00);
ClockTime alarmStartTime(5, 30);
ClockTime alarmEndTime  (6, 30);
Adafruit_7segment matrix = Adafruit_7segment();
ClickEncoder *encoder;
RTC_DS3231 RTC;
ClockTime synchronized_clock_time;
uint32_t synchronized_clock_millis;
ClockTime current_clock_time;
uint32_t millisAtStartOfSunset = 0;
uint32_t sunsetDeltaInMilliseconds = 600000; // 10 minutes in milliseconds
int sunsetDimLevel = 100; // 100 = max bright, 0 = off
ClockTime snoozeStartTime;
ClockTime snoozeEndTime;

uint32_t alarmStartMillis = 0;

} // namespace SunriseAlarm 

#include "LightColor.h"
#include "Inputs.hpp"
#include "ClockStates_decl.hpp"
#include "ClockStates_def.hpp"

// 2013-04-06 TODO:  Add a temperature sensor to the lamp and if it exceeds a particular temperature, then turn it off or down
// 2013-04-06 TODO:  Add three LEDs inside the enclosure to indicate which mode you are in.  E.g. green for alarm, blue for time, and red for sundown.
// 2013-04-06 DONE 2013-04-06:  Store Sunset alarm setting in EEPROM
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
// DONE 2013-03-30:  Add Sunset mode
// WONTFIX 2013-03-29:  Add AM/PM indicator
// DONE 2013-03-29:  Add method to toggle alarm on/off
// DONE 2013-03-28:  Add hardware buttons for setting time and alarm
// TODO:  Add audio:  chirping birds at alarm time
// TODO:  Add snooze button to delay chirping for 7 minutes or so
// DONE 2013-04-17:  Change light alarm so it is based on millis() rather than RTC
// TODO:  Get light updated more often, especially in third slope of alarm.

namespace SunriseAlarm {

bool soundAlarmBPlaying = false;

Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LED, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);



AT24Cxx eeprom;



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


void timerIsr() {
  encoder->service();
}


bool readAlarmTimeFromEEPROM() {
  if (eeprom.isPresent()) {
    char buf[2];
    eeprom.ReadMem(ALARM_ADDRESS, buf, 2);
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
    eeprom.WriteMem(ALARM_ADDRESS, myBuf, 2);
    return true;
  }
  return false;
}

bool readSunsetDimFromEEPROM() {
  if (eeprom.isPresent()) {
    char buf[1];
    eeprom.ReadMem(SUNSET_DIM_ADDRESS, buf, 1);
    sunsetDimLevel = static_cast<int>(buf[0]);
    return true;
  }
  return false;
}

bool writeSunsetDimToEEPROM() {
  if (eeprom.isPresent()) {
    char myBuf[1];
    myBuf[0] = static_cast<char>(sunsetDimLevel);
    eeprom.WriteMem(SUNSET_DIM_ADDRESS, myBuf, 1);
    return true;
  }
  return false;
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
  uint16_t stepNumber = (totalDimmerSteps * milliseconds) / ALARM_DELTA_MILLIS;
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
  uint32_t stepNumber = ((uint32_t)(totalDimmerSteps) * milliseconds) / ALARM_DELTA_MILLIS;
  logisticBrightOfStep(stepNumber);
}

void turnLightOn() {
  int16_t delta = max(1, totalDimmerSteps / 250);
  for (int16_t i = currentDimmerStep; i < totalDimmerSteps; i += delta) {
    linearBrightOfStep(i);
    delay(1);
  }
  linearBrightOfStep(totalDimmerSteps);
}

void turnLightOff() {
  int16_t delta = max(1, totalDimmerSteps / 250);
  for (int16_t i = currentDimmerStep; i > 0 ; i -= delta) {
    linearBrightOfStep(i);
    delay(1);
  }
  linearBrightOfStep(0u);
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

void updateClockDisplayNow() {
  current_clock_time = synchronized_clock_time;
  const uint32_t seconds_delta = static_cast<uint32_t>(millis() - synchronized_clock_millis) / 1000;
  const uint32_t minutes_delta = seconds_delta / 60;
  current_clock_time.minute += minutes_delta;
  current_clock_time.second += (seconds_delta - minutes_delta * 60);
  current_clock_time.fixTime();
  updateTimeDisplay(current_clock_time, false, alarmMasterSwitchEnabled);
}

const uint32_t updateInterval = 1000; // every second
uint32_t lastUpdate = millis();
void updateClockDisplayOneSecondAfterLastTime() {
  if (isTimeNow(lastUpdate, updateInterval)) {
    updateClockDisplayNow();
  }
}

void updateTimeFromRTCNow() {
  DateTime dt = RTC.now();
  synchronized_clock_time = ClockTime(dt.hour(), dt.minute(), dt.second());
  synchronized_clock_millis = millis();
  current_clock_time = synchronized_clock_time;
}

const uint32_t clockUpdateInterval = 24 * 60 * 60 * 1000; // every 24 hours
uint32_t lastClockUpdate = millis();
void updateTimeFromRTC24HoursAfterLastTime() {
  if (isTimeNow(lastClockUpdate, clockUpdateInterval)) {
    updateTimeFromRTCNow();
  }
}

void updateAlarmStartEndTime() {
  alarmStartTime = alarmTime;
  alarmStartTime -= ALARM_DELTA;
  alarmEndTime = alarmTime;
  alarmEndTime += ALARM_DELTA;
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
#ifdef DEBUG
  Serial.begin(9600);
#endif
  Wire.begin();
  RTC.begin();
  pinMode(AlarmButton, INPUT_PULLUP);
  pinMode(TimeButton, INPUT_PULLUP);
  pinMode(alarmMasterSwitch, INPUT_PULLUP);
  pinMode(RotaryButton, INPUT_PULLUP);
  pinMode(TouchSensor, INPUT);
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
  //readBrightnessFromEEPROM();
  matrix.setBrightness(matrixBrightness); // 0-15
  //readSunsetDeltaFromEEPROM();
  readSunsetDimFromEEPROM();
  updateTimeFromRTCNow();
  updateClockDisplayNow();
  readAlarmTimeFromEEPROM();
  updateAlarmStartEndTime();
}

void alarmAMusicOn()
{
  //digitalWrite(soundAPin, LOW);
}

void alarmAMusicOff()
{
  //digitalWrite(soundAPin, HIGH);
}

void alarmBMusicOn()
{
  digitalWrite(soundBPin, LOW);
  soundAlarmBPlaying = true;
}

void alarmBMusicOff()
{
  digitalWrite(soundBPin, HIGH);
  soundAlarmBPlaying = false;
}

const unsigned lightUpdateInterval = 50;
uint32_t lastLightUpdate = millis();
void updateAlarmLight() {
  if (!isTimeNow(lastLightUpdate, lightUpdateInterval)) {
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

uint32_t lastSunsetUpdate = millis();
unsigned sunsetUpdateInterval = 5;
void updateSunsetLight() {
  if (!isTimeNow(lastSunsetUpdate, sunsetUpdateInterval)) {
    return;
  }
  uint32_t currentMillis = millis();
  uint32_t delta = static_cast<uint32_t>(currentMillis - millisAtStartOfSunset);
  uint32_t temp = totalDimmerSteps - totalDimmerSteps * delta / sunsetDeltaInMilliseconds;
  logisticBrightOfStep(temp);
}

void loop() {
  turnLightOff();
  stateCE();
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


