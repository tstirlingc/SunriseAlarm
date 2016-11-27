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
    while (this->minute > 59) {
      ++this->hour;
      this->minute -= 60;
    }
    while (this->hour > 23) {
      this->hour -= 24;
    }
    while (this->second < 0) {
      --this->minute;
      this->second += 60;
    }
    while (this->minute < 0) {
      --this->hour;
      this->minute += 60;
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
    lhs.minute+=minutes;
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

const int LEDpinA = 9;

const int AlarmButton = 2;
const int TimeButton = 3;
const int FwdButton = 4;
const int RewButton = 5;
const int OffButton = 6;

bool alarmEnabled = true;
bool alarmActive = false;

TimerOne timer;
const int16_t maxBright = 1024; // resolution of TimerOne::pwm (10 bits)
const int thirtyminutes = 1800; // 1800 seconds = 30 minutes
const int timerPeriod = 1000; // 1000us = 1 kHz flicker
const int16_t maxTime = 3600; // 1.0 hours
const int forward_delay = 15;
const int rewind_delay = 150;

int defaultSunSetDelta = 15; // minutes

const unsigned long modeInactivePeriod = 30000; // 30 seconds

AT24Cxx eeprom;
// This is the time you want to get up.  Lights will start 30 minutes before.
ClockTime alarmTime(5,00); 
ClockTime startTime(4,30);

const int alarm_address = 0;
const int sundown_address = 2;
const int brightness_address = 3;

Adafruit_7segment matrix = Adafruit_7segment();
int matrixBrightness = 0;

uint8_t raw_C = 0x39;
uint8_t raw_l = 0x30;
uint8_t raw_o = 0x5C;
uint8_t raw_c = 0x58;
uint8_t raw_A = 0x77;
uint8_t raw_L = 0x38;
uint8_t raw_r = 0x50;
uint8_t raw_t = 0x78;
uint8_t raw_d = 0x5E;
uint8_t dot_bit = 0x80;

RTC_DS1307 RTC;
unsigned long synchronized_clock_millis;
ClockTime synchronized_clock_time;
ClockTime current_clock_time;

bool readAlarmTimeFromEEPROM() {
  if (eeprom.isPresent()) {
    char buf[2];
    eeprom.ReadMem(alarm_address,buf,2);
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
    eeprom.WriteMem(alarm_address,myBuf,2);
    return true;
  }
  return false;
}

bool readBrightnessFromEEPROM() {
  if (eeprom.isPresent()) {
    char buf[1];
    eeprom.ReadMem(brightness_address,buf,1);
    matrixBrightness = static_cast<int>(buf[0]);
    return true;
  }
  return false;
}

bool writeBrightnessToEEPROM() {
  if (eeprom.isPresent()) {
    char myBuf[1];
    myBuf[0] = static_cast<char>(matrixBrightness); 
    eeprom.WriteMem(brightness_address,myBuf,1);
    return true;
  }
  return false;
}


bool readSunDownDeltaFromEEPROM() {
  if (eeprom.isPresent()) {
    char buf[1];
    eeprom.ReadMem(sundown_address,buf,1);
    defaultSunSetDelta = static_cast<int>(buf[0]);
    return true;
  }
  return false;
}

bool writeSunDownDeltaToEEPROM() {
  if (eeprom.isPresent()) {
    char myBuf[1];
    myBuf[0] = static_cast<char>(defaultSunSetDelta); 
    eeprom.WriteMem(sundown_address,myBuf,1);
    return true;
  }
  return false;
}



int debounceInterval = 5;
int debounceDigitalRead(int pin) {
  int lastResult = digitalRead(pin);
  unsigned long lastRead = millis();
  while (static_cast<unsigned long>(millis() - lastRead) < debounceInterval) {
    int result = digitalRead(pin);
    if (result != lastResult) {
      lastResult = result;
      lastRead = millis();
    }
  }
  return lastResult;
}

void turnLightOn(int currentLevel) {
  for (int i=currentLevel ; i < maxBright ; ++i) {
    timer.pwm(LEDpinA,i);
    delay(1);
  }
  digitalWrite(LEDpinA,HIGH);
}

void turnLightOff(int currentLevel) {
  for (int i=currentLevel ; i>0 ; --i) {
    timer.pwm(LEDpinA,i);
    delay(1);
  }
  digitalWrite(LEDpinA,LOW);
}

void waitForButtonDepress(int pin) {
  while (debounceDigitalRead(pin)==LOW) {
    delay(5);
  }
}

// 10W LED with big heatsink:
//const int minLightLevel = 176;
//const int maxLightLevel = 1008;
// MR16 LED light with 3x3W LEDs:
const int minLightLevel = 320;
const int maxLightLevel = 800;

// (a1,b1) -> f_1 -> (a2,b2) -> f_2 -> (a3,b3) -> f_3 -> (a4,b4)
double a1 = 0.0;
double b1 = minLightLevel;
double a2 = thirtyminutes-10*60; // 10 minutes before
double b2 = maxLightLevel*.30; // 30% 
double a3 = thirtyminutes-5*60; // 5 minutes before
double b3 = maxLightLevel*.40; // 40%
double a4 = thirtyminutes;
double b4 = maxLightLevel;

double m1 = (b2-b1)/(a2-a1);
double c1 = (b1-m1*a1);
double m2 = (b3-b2)/(a3-a2);
double c2 = (b2-m2*a2);
double m3 = (b4-b3)/(a4-a3);
double c3 = (b3-m3*a3);

// This function maps the visible range of the LED to the time interval 0 to 1800 seconds.
// Visible range on LED at TimerOne Period of 1000us (1 kHz flicker) is [176,1008] at increment of 16.
// Therefore the two points are (0,176) and (1800,1008) which results in a slope of 0.4622222 and a y-intercept of 176.
// 2014-03-11:  (0,minlightLevel) -> (1800,maxLightLevel), slope = (maxLightLevel-minLightLevel)/1800
int lastAlarmLightLevel = 0;
int32_t linearBright(int32_t level) {
  //int linearLevel = ceil(maxBright*1.0/(1.0+exp(-(level*1.0-maxTime*0.5)/(maxTime/16.0))));
  //int linearLevel = level;
  //int linearLevel = 36.787944117144235*exp(exp(level*0.00075472484480368113));

  // (0,176) -> (1800,1008)
  //int linearLevel = 0.46222222*level + 176; // linear
  //int linearLevel = 176.0*exp(0.000969577*level); // exponential
  //int linearLevel = 64.746781646173858*exp(exp(0.00056103794655312409*level)); // exp(exp())

  //This method seems to work well for MR16 9W LED
  int linearLevel = ((maxLightLevel-minLightLevel)/1800.0)*level+minLightLevel;
  //This method worked well for 10W LED
//  int linearLevel = 0;
//  if (level < a2) {
//    linearLevel = m1*level+c1;
//  } else if (level < a3) {
//    linearLevel = m2*level+c2;
//  } else {
//    linearLevel = m3*level+c3;
//  }
  
  //Serial.print("linearLevel = ");
  //Serial.println(linearLevel);
  lastAlarmLightLevel = linearLevel;
  timer.pwm(LEDpinA,lastAlarmLightLevel);
  return linearLevel;
}

// DONE:  Update to display 12 hr time
void updateTimeDisplay(ClockTime t, bool military, bool dots) {
  matrix.print(0);
  int hour = t.hour;
  if (!military) {
    hour = (t.hour > 12) ? t.hour-12 : t.hour ;
    if (hour == 0) { hour += 12; }
  } 
  int digit0 = hour / 10;
  int digit1 = hour % 10;
  int digit3 = t.minute / 10;
  int digit4 = t.minute % 10;
  if (digit0 > 0) {
    matrix.writeDigitNum(0,digit0,dots);
  } 
  matrix.writeDigitNum(1,digit1, dots);
  matrix.writeDigitNum(2,1); // colon
  matrix.writeDigitNum(3,digit3,dots);
  matrix.writeDigitNum(4,digit4,dots);
  matrix.writeDisplay();
}

// wait is in milliseconds
// last is the last time this function returned true
bool isTimeNow(unsigned long & last, unsigned wait) {
  unsigned long time = millis();
  if (static_cast<unsigned long>(time-last) > wait) {
    last = time;
    return true;
  }
  return false;
}

const unsigned long updateInterval = 1000; // every second
unsigned long lastUpdate = millis();
void updateCurrentTime(bool force=false) {
  if (force || isTimeNow(lastUpdate,updateInterval)) {
    current_clock_time = synchronized_clock_time;
    const unsigned long seconds_delta = static_cast<unsigned long>(millis() - synchronized_clock_millis)/1000;
    const unsigned long minutes_delta = seconds_delta/60;
    current_clock_time.minute += minutes_delta;
    current_clock_time.second += (seconds_delta - minutes_delta*60);
    current_clock_time.fixTime();
    updateTimeDisplay(current_clock_time,false,false);
  }
}

const unsigned long clockUpdateInterval = 24*60*60*1000; // every 24 hours
unsigned long lastClockUpdate = millis();
void updateClockTime(bool force=false) {
  if (force || isTimeNow(lastClockUpdate,clockUpdateInterval)) {
    synchronized_clock_time = ClockTime(RTC.now());
    synchronized_clock_millis = millis();
    current_clock_time = synchronized_clock_time;
  }
}

void updateStartTime() {
  startTime = alarmTime;
  startTime.minute -= 30;
  startTime.fixTime();
}


void display_Cloc() {
  matrix.clear();
  matrix.writeDigitRaw(0,raw_C | dot_bit);
  matrix.writeDigitRaw(1,raw_l | dot_bit);
  matrix.writeDigitRaw(3,raw_o | dot_bit);
  matrix.writeDigitRaw(4,raw_c | dot_bit);
  matrix.writeDisplay();
}



void display_ALAr() {
  matrix.clear();
  matrix.writeDigitRaw(0,raw_A | dot_bit);
  matrix.writeDigitRaw(1,raw_L | dot_bit);
  matrix.writeDigitRaw(3,raw_A | dot_bit);
  matrix.writeDigitRaw(4,raw_r | dot_bit);
  matrix.writeDisplay();
}

void display_todd() {
  matrix.setBrightness(15);
  matrix.clear();
  matrix.writeDigitRaw(0,raw_t);
  matrix.writeDigitRaw(1,raw_o);
  matrix.writeDigitRaw(3,raw_d);
  matrix.writeDigitRaw(4,raw_d);
  matrix.writeDisplay();
  delay(5000);
}

void setup() {
  //Serial.begin(9600);
  Wire.begin();
  RTC.begin();
  //if (! RTC.isrunning()) {
  //  Serial.println("RTC is NOT running!");
  //}
  //RTC.adjust(DateTime(__DATE__, __TIME__));
  //RTC.adjust(
  //  DateTime( 2013, 
  //            03, 
  //            24,
  //            06,
  //            14,
  //            50
  //          )
  //);
  pinMode(LEDpinA,OUTPUT); digitalWrite(LEDpinA,0);
  pinMode(AlarmButton,INPUT); 
  pinMode(TimeButton,INPUT);
  pinMode(RewButton,INPUT); 
  pinMode(FwdButton,INPUT); 
  pinMode(OffButton,INPUT);
  timer.initialize(timerPeriod);
  timer.pwm(LEDpinA, 0); //set up pin 9
  matrix.begin(0x70);
  display_todd();
  readBrightnessFromEEPROM();
  matrix.setBrightness(matrixBrightness); // 0-15
  readSunDownDeltaFromEEPROM();
  updateClockTime(true);
  updateCurrentTime(true);
  if (!readAlarmTimeFromEEPROM()) {    
    alarmTime = ClockTime(5,45);
  }
  updateStartTime();
}

int32_t secondsBtwDates(ClockTime currentTime, ClockTime alarm) {
  int32_t s = (currentTime.hour - alarm.hour);
  s *= 3600;
  s += (currentTime.minute - alarm.minute)*60;
  s += (currentTime.second - alarm.second);
  //Serial.print("Seconds between dates:  currentTime = ");
  //Serial.print(currentTime.hour);
  //Serial.print(":");
  //Serial.print(currentTime.minute);
  //Serial.print(", startTime = ");
  //Serial.print(alarm.hour);
  //Serial.print(":");
  //Serial.print(alarm.minute);
  //Serial.print(", seconds = ");
  //Serial.println(s);
  return( s );
}


const unsigned lightUpdateInterval = 1000;
unsigned long lastLightUpdate = millis();
void updateLight() {
  if (!isTimeNow(lastLightUpdate,lightUpdateInterval)) {
    return;
  }
  //Serial.println("updateLight");
  ClockTime current = current_clock_time;
  int32_t seconds = secondsBtwDates(current,startTime);
  //Serial.print("  seconds = ");
  //Serial.println(seconds);
  if ((seconds < 0) || (seconds >= maxTime)) {
    //Serial.println("  outside alarm zone");
    if (alarmActive) {
      //Serial.println("  turning off light and setting alarmActive = false");
      turnLightOff(maxBright-1);
      alarmActive = false;
    }
    //Serial.println("  turning on alarmEnabled");
    alarmEnabled = true;
    return;
  }
  //Serial.println("inside alarm zone");
  if (!alarmEnabled) {
    //Serial.println("but alarmEnabled is false");
    return;
  }
  //Serial.println("turning on alarmActive");
  alarmActive = true;
  if (seconds >= thirtyminutes) {
    //Serial.println("past alarm time but still in alarm zone turning light on full and returning");
    lastAlarmLightLevel = maxLightLevel;
    digitalWrite(LEDpinA,HIGH);
    return;
  }
  //Serial.println("in alarm zone during ramp-up, calling linearBright");
  linearBright(seconds);
}


void forwardTime(ClockTime & t) {
  while (digitalRead(FwdButton)==LOW) {
    ++t.minute;
    if (t.minute > 59) {
      ++t.hour;
      t.minute = 0;
    }
    if (t.hour > 23) {
      t.hour = 0;
    }
    updateTimeDisplay(t,true,true);
    delay(forward_delay);
  }
}

void rewindTime(ClockTime & t) {
  while (digitalRead(RewButton)==LOW) {
    --t.minute;
    if (t.minute < 0) {
      --t.hour;
      t.minute = 59;
    }
    if (t.hour < 0) {
      t.hour = 23;
    }
    updateTimeDisplay(t,true,true);
    delay(rewind_delay);
  }

}

void forwardAlarm() {
  while (digitalRead(FwdButton)==LOW) {
    ++alarmTime;
    updateTimeDisplay(alarmTime,true,true);
    delay(forward_delay);
  }
}

void rewindAlarm() {
 while (digitalRead(RewButton)==LOW) {
    --alarmTime;
    updateTimeDisplay(alarmTime,true,true);
    delay(rewind_delay);
  }
}



void setTime() {
  DateTime currentTime = RTC.now();
  ClockTime t(currentTime);
  display_Cloc();
  delay(1000);
  updateTimeDisplay(t,true,true);
  waitForButtonDepress(TimeButton);
  unsigned long modeActive = millis();
  bool normalExit = true;
  while (debounceDigitalRead(TimeButton)==HIGH) {
    if (digitalRead(FwdButton)==LOW) {
      forwardTime(t);
      modeActive = millis();
    } else if (digitalRead(RewButton)==LOW) {
      rewindTime(t);
      modeActive = millis();
    }
    if (static_cast<unsigned long>(millis() - modeActive) > modeInactivePeriod) {
      normalExit = false;
      break;
    }
  }
  if (normalExit) {
    waitForButtonDepress(TimeButton);
  }
  updateTimeDisplay(t,false,false);
  RTC.adjust(
    DateTime( currentTime.year(), 
              currentTime.month(), 
              currentTime.day(),
              t.hour,
              t.minute,
              0
            )
  );
  //updateClockTime(true);
  // Avoid talking to RTC:
  synchronized_clock_time = t;
  synchronized_clock_millis = millis();
  current_clock_time = synchronized_clock_time;
  updateCurrentTime(true);
}

void setAlarm() {
  display_ALAr();
  delay(1000);
  updateTimeDisplay(alarmTime,true,true);
  waitForButtonDepress(AlarmButton);
  unsigned long modeActive = millis();
  bool normalExit = true;
  while (debounceDigitalRead(AlarmButton)==HIGH) {
    if (digitalRead(FwdButton)==LOW) {
      forwardAlarm();
      modeActive = millis();
    } else if (digitalRead(RewButton)==LOW) {
      rewindAlarm();
      modeActive = millis();
    }
    if (static_cast<unsigned long>(millis() - modeActive) > modeInactivePeriod) {
      normalExit = false;
      break;
    }
  }
  if (normalExit) {
    waitForButtonDepress(AlarmButton);
  }
  updateStartTime();
  //updateTimeDisplay(startTime,true,true);
  //delay(2000);
  updateCurrentTime(true);
  updateTimeDisplay(current_clock_time,false,false);
  writeAlarmTimeToEEPROM();
}

void forwardSunSet(int & minutes) {
  while (digitalRead(FwdButton)==LOW) {
    ++minutes;
    matrix.print(minutes);
    matrix.writeDisplay();
    delay(forward_delay);
  }
}

void rewindSunSet(int & minutes) {
 while (digitalRead(RewButton)==LOW) {
    --minutes;
    if (minutes < 0) {
      minutes = 0;
    }
    matrix.print(minutes);
    if (minutes == 0) {
      matrix.writeDigitNum(4,0);
    }
    matrix.writeDisplay();
    delay(rewind_delay);
  }
}


unsigned long millisAtStartOfSunSet = 0;
bool sunSetActive = false;
void setSunSet() {
  matrix.print(defaultSunSetDelta);
  if (defaultSunSetDelta == 0) {
    matrix.writeDigitNum(4,0);
  }
  matrix.writeDisplay();
  sunSetActive = true;
  turnLightOn(0);
  waitForButtonDepress(OffButton);
  unsigned long modeActive = millis();
  bool normalExit = true;
  while (debounceDigitalRead(OffButton)==HIGH) {
    if (digitalRead(FwdButton)==LOW) {
      forwardSunSet(defaultSunSetDelta);
      modeActive = millis();
    } 
    else if (digitalRead(RewButton)==LOW) {
      rewindSunSet(defaultSunSetDelta);
      modeActive = millis();
    }
    if (static_cast<unsigned long>(millis() - modeActive) > modeInactivePeriod) {
      normalExit = false;
      break;
    }
  }
  if (defaultSunSetDelta == 0) {
    sunSetActive = false;
    turnLightOff(maxBright-1);
  }
  if (normalExit) {
    waitForButtonDepress(OffButton);
  }
  writeSunDownDeltaToEEPROM();
  millisAtStartOfSunSet = millis();
}


unsigned long lastSunSetUpdate = millis();
unsigned sunSetUpdateInterval = 5;
int lastSunSetLightLevel = 0;
void updateSunSet() {
  if (!sunSetActive) {
    return;
  }
  if (!isTimeNow(lastSunSetUpdate,sunSetUpdateInterval)) {
    return;
  }
  unsigned long currentMillis = millis();
  unsigned long delta = static_cast<unsigned long>(currentMillis - millisAtStartOfSunSet);
  unsigned long finalMillis = defaultSunSetDelta;
  finalMillis *= 60;
  finalMillis *= 1000;
  if (delta > finalMillis) {
    sunSetActive = false;
    digitalWrite(LEDpinA,0);
  }
  else {
    double temp = -(1800.0*delta)/(finalMillis*1.0) + 1800.0;
    linearBright(temp);
  }
}


void loop() {
  if (digitalRead(AlarmButton)==LOW && debounceDigitalRead(AlarmButton)==LOW) {
    setAlarm();
  } 
  else if (digitalRead(TimeButton)==LOW && debounceDigitalRead(TimeButton)==LOW) {
    setTime();
  } 
  else if (digitalRead(OffButton)==LOW && debounceDigitalRead(OffButton)==LOW) {
    if (alarmActive) {
      alarmActive = false;
      alarmEnabled = false;
      turnLightOff(lastAlarmLightLevel);
      waitForButtonDepress(OffButton);
    } 
    else if (sunSetActive) {
      sunSetActive = false;
      turnLightOff(lastSunSetLightLevel);
      waitForButtonDepress(OffButton);
    }
    else {
      setSunSet();
    }
  } 
  else if (digitalRead(FwdButton)==LOW && debounceDigitalRead(FwdButton)==LOW) {
    matrixBrightness = max(min(15,matrixBrightness+1),0); 
    matrix.setBrightness(matrixBrightness); // 0-15
    waitForButtonDepress(FwdButton);
    writeBrightnessToEEPROM();
  } 
  else if (digitalRead(RewButton)==LOW && debounceDigitalRead(RewButton)==LOW) {
    matrixBrightness = max(min(15,matrixBrightness-1),0); 
    matrix.setBrightness(matrixBrightness); // 0-15
    waitForButtonDepress(RewButton);
    writeBrightnessToEEPROM();
  }
  updateLight();
  updateCurrentTime();
  updateClockTime();
  updateSunSet();
  // DONE 2013-03-30:  Add a feature so Fwd and Rew buttons adjust the display brightness
  // DONE 2013-03-30:  Add a feature so the OffButton can be used for SunDown mode when the SunRise mode is not currently active.
  //        Turn on light (ramp up quickly but not instantly) then slowly turn off after 15 minutes
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

