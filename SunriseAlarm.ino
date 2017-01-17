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
#include "ClockTime.hpp"
#include "MillisDelta.hpp"

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
#define UPDATE_FROM_RTC_INTERVAL 3600000 // update from RTC every hour (in milliseconds)
#define CLOCK_DISPLAY_UPDATE_INTERVAL 1000 // update clock display every second

//#define DEBUG 1

namespace SunriseAlarm {
// Declarations
void updateClockDisplayNow();
void updateClockDisplayOneSecondAfterLastTime();
void updateTimeFromRTCNow();
void updateTimeFromRTC24HoursAfterLastTime();
void turnLightOn();
void turnLightOff();
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
bool isTimeNow(uint32_t & last, unsigned wait);

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
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LED, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);
AT24Cxx eeprom;
uint32_t alarmStartMillis = 0;

} // namespace SunriseAlarm 

#include "LightColor.h"
#include "Inputs.hpp"
#include "Outputs.hpp"
#include "ClockStates_decl.hpp"
#include "ClockStates_def.hpp"

namespace SunriseAlarm {

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


// wait is in milliseconds
// last is the last time this function returned true
bool isTimeNow(uint32_t & last, unsigned wait) {
  uint32_t time = millis();
  if (millis_delta(last, time) > wait) {
    last = time;
    return true;
  }
  return false;
}

void updateTimeFromRTCNow() {
  DateTime dt = RTC.now();
  synchronized_clock_time = ClockTime(dt.hour(), dt.minute(), dt.second());
  synchronized_clock_millis = millis();
  current_clock_time = synchronized_clock_time;
}

uint32_t lastClockUpdate = millis();
void updateTimeFromRTC24HoursAfterLastTime() {
  if (isTimeNow(lastClockUpdate, UPDATE_FROM_RTC_INTERVAL)) {
    updateTimeFromRTCNow();
  }
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

void loop() {
  changeState_Init_CE();
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


