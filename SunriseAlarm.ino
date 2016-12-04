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


ClickEncoder *encoder;
#define ENCODER_PIN0 11
#define ENCODER_PIN1 12


#define AlarmButton 7
#define AlarmLED 5
#define TimeButton 9
#define TimeLED 10
#define OffButton A0
#define alarmMasterSwitch 8
#define RotaryButton A1

// Sound effects serial TX/RX pins:
#define SFX_TX 5
#define SFX_RX 6
// Sound effects reset pin:
#define SFX_RST 4
SoftwareSerial ss = SoftwareSerial(SFX_TX, SFX_RX);
Adafruit_Soundboard sfx = Adafruit_Soundboard(&ss, NULL, SFX_RST);

#define NUM_LED 32
#define LED_PIN 6
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LED, LED_PIN, NEO_GRB + NEO_KHZ800);

bool alarmMasterSwitchEnabled = true;
bool alarmEnabled = true;
bool alarmActive = false;

//TimerOne timer;
const int16_t maxBright = 1024; // resolution of TimerOne::pwm (10 bits)
const int thirtyminutes = 1800; // 1800 seconds = 30 minutes
//const int timerPeriod = 1000; // 1000us = 1 kHz flicker
const int16_t maxTime = 3600; // 1.0 hours
const int forward_delay = 15;
const int rewind_delay = 150;

int defaultSunSetDelta = 15; // minutes

const unsigned long modeInactivePeriod = 30000; // 30 seconds

AT24Cxx eeprom;
// This is the time you want to get up.  Lights will start 30 minutes before.
ClockTime alarmTime(6,00); 
ClockTime startTime(5,30);

const int alarm_address = 4;
const int sundown_address = 6;
const int brightness_address = 7;

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

RTC_DS3231 RTC;
unsigned long synchronized_clock_millis;
ClockTime synchronized_clock_time;
ClockTime current_clock_time;

void timerIsr() {
  encoder->service();
}


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



int debounceInterval = 10;
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

uint8_t computeMinRGBLevel(uint8_t* RGB)
{
  uint8_t myRGB[3] = {255, 255, 255};
  for (uint8_t i=0 ; i<3 ; ++i) 
  {
    if (RGB[i]>0) myRGB[i] = RGB[i];
  }
  return min(min(myRGB[0],myRGB[1]),myRGB[2]);
}


const uint8_t candleLight[3] = {255, 147, 41};
const uint8_t sunLight[3] = {255, 255, 255};
const uint8_t testingLight[3] = {50, 0, 0};
uint8_t targetColor[3] = {255, 0, 0};

unsigned long millisecondsIn30Minutes = 1800000;
uint8_t minRGBLevel = computeMinRGBLevel(targetColor);
uint16_t totalDimmerSteps = minRGBLevel*NUM_LED;

void setColorForSunDown()
{
  for (uint8_t i=0 ; i<3 ; ++i)
  {
    targetColor[i] = candleLight[i];    
  }
  minRGBLevel = computeMinRGBLevel(targetColor);
  totalDimmerSteps = minRGBLevel*NUM_LED;
}

void setColorForSunRise()
{
  for (uint8_t i=0 ; i<3 ; ++i)
  {
    targetColor[i] = sunLight[i];    
  }
  minRGBLevel = computeMinRGBLevel(targetColor);
  totalDimmerSteps = minRGBLevel*NUM_LED;  
}

uint16_t currentDimmerStep = 0;
uint8_t baseRGBColor[3] = {0u, 0u, 0u};
uint8_t highRGBColor[3] = {0u, 0u, 0u};

void linearBrightOfStep(uint16_t step)
{
  currentDimmerStep = step;
  uint16_t levelForAllLEDs = step / NUM_LED;
  uint8_t numLEDsAtHigherLevel = step % NUM_LED;
  for (uint8_t i=0 ; i<3 ; ++i)
  {
    baseRGBColor[i] = (targetColor[i]*levelForAllLEDs)/minRGBLevel;
    highRGBColor[i] = (targetColor[i]*(levelForAllLEDs+1))/minRGBLevel;
  }
  for (uint8_t i=0 ; i<NUM_LED ; ++i) 
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

unsigned long linearBrightOfMilliseconds(unsigned long milliseconds)
{
  uint16_t stepNumber = (totalDimmerSteps*milliseconds)/millisecondsIn30Minutes;
  linearBrightOfStep(stepNumber);
}

void logisticBrightOfStep(uint16_t step)
{
  float p = ((float)step)/((float)totalDimmerSteps);
  float L = 1.0;
  float k = 20.0;
  float x0 = 0.5;
  float logisticP = L/(1+exp(-k*(p-x0)));
  uint16_t logisticStep = logisticP*totalDimmerSteps;
  linearBrightOfStep(logisticStep);
}

void logisticBrightOfMilliseconds(unsigned long milliseconds)
{
  uint16_t stepNumber = (totalDimmerSteps*milliseconds)/millisecondsIn30Minutes;
  logisticBrightOfStep(stepNumber);
}

void turnLightOn() {
  int16_t delta = max(1,totalDimmerSteps/500);
  for (int16_t i=currentDimmerStep; i < totalDimmerSteps; i+=delta) {
    linearBrightOfStep(i);
    delay(1);
  }
  linearBrightOfStep(totalDimmerSteps);
}

void turnLightOff() {
  int16_t delta = max(1,totalDimmerSteps/500);
  for (int16_t i=currentDimmerStep; i>0 ; i-=delta) {
    linearBrightOfStep(i);
    delay(1);
  }
  linearBrightOfStep(0u);
}

void waitForButtonDepress(int pin, uint8_t onState) {
  while (debounceDigitalRead(pin)==onState) {
    delay(debounceInterval);
  }
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
    updateTimeDisplay(current_clock_time,false,alarmMasterSwitchEnabled);
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
  //pinMode(LEDpinA,OUTPUT); digitalWrite(LEDpinA,0);
  pinMode(AlarmButton,INPUT_PULLUP); 
  pinMode(TimeButton,INPUT_PULLUP);
  pinMode(alarmMasterSwitch, INPUT_PULLUP);
  pinMode(RotaryButton, INPUT_PULLUP);
  pinMode(OffButton,INPUT);
  pinMode(AlarmLED, OUTPUT);
  pinMode(TimeLED, OUTPUT);
  alarmMasterSwitchEnabled = (digitalRead(alarmMasterSwitch) == HIGH);
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
  encoder = new ClickEncoder(ENCODER_PIN0, ENCODER_PIN1, -1, 4);
  Timer1.initialize(1000);
  Timer1.attachInterrupt(timerIsr); 
  //timer.initialize(timerPeriod);
  //timer.pwm(LEDpinA, 0); //set up pin 9
  while (!eeprom.isPresent()) { delay(1); }
  matrix.begin(0x70);
  ss.begin(9600);
  if (!sfx.reset()) {
    Serial.println("SFX board not found");
    while(1);
  }
  // Reduce volume on SFX board:
  if (sfx.volDown() > 128) {
    while (sfx.volDown() > 128) {}
  }
  // Play an easter-egg message while displaying my name...
  display_todd();
  readBrightnessFromEEPROM();
  matrix.setBrightness(matrixBrightness); // 0-15
  readSunDownDeltaFromEEPROM();
  updateClockTime(true);
  updateCurrentTime(true);
  readAlarmTimeFromEEPROM();
  updateStartTime();
  
//  turnLightOn();
//  delay(1000);
//  turnLightOff();
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

unsigned long alarmStartMillis = 0;
const unsigned lightUpdateInterval = 50;
unsigned long lastLightUpdate = millis();
bool sfx_playing = false;
void updateLight() {
  if (!isTimeNow(lastLightUpdate,lightUpdateInterval)) {
    return;
  }
  if (digitalRead(alarmMasterSwitch) == LOW) return;
  ClockTime current = current_clock_time;
  int32_t seconds = secondsBtwDates(current,startTime);
  if ((seconds < 0) || (seconds >= maxTime)) {
    if (alarmActive) {
      turnLightOff();
      sfx.stop();
      alarmActive = false;
    }
    alarmEnabled = true;
    return;
  }
  if (!alarmEnabled) {
    return;
  }
  if (!alarmActive)
  {
    setColorForSunRise();
    alarmStartMillis = static_cast<unsigned long>(millis()-1000*seconds);
    alarmActive = true;
  }
  if (seconds >= thirtyminutes) {
    linearBrightOfStep(totalDimmerSteps);
    return;
  }
  if (seconds == thirtyminutes && !sfx_playing) {
    sfx_playing = sfx.playTrack("WAKE.OGG");
  }
  logisticBrightOfMilliseconds(static_cast<unsigned long>(millis()-alarmStartMillis));
}


void updateTime(ClockTime & t, int16_t delta) {
  t.minute += delta;
  updateTimeDisplay(t,true,alarmMasterSwitchEnabled);
}

void updateAlarm(int16_t delta) {
  alarmTime += delta;
  updateTimeDisplay(alarmTime,true,alarmMasterSwitchEnabled);
}


void setTime() {
  analogWrite(TimeLED,matrixBrightness*255/16+15);
  DateTime currentTime = RTC.now();
  ClockTime t(currentTime);
  display_Cloc();
  delay(1000);
  updateTimeDisplay(t,true,alarmMasterSwitchEnabled);
  waitForButtonDepress(TimeButton, LOW);
  unsigned long modeActive = millis();
  bool normalExit = true;
  while (debounceDigitalRead(TimeButton)==HIGH) {
    int16_t encoderDelta = encoder->getValue();
    updateTime(t,encoderDelta);
    modeActive = millis();
    if (static_cast<unsigned long>(millis() - modeActive) > modeInactivePeriod) {
      normalExit = false;
      break;
    }
  }
  if (normalExit) {
    waitForButtonDepress(TimeButton, LOW);
  }
  updateTimeDisplay(t,false,alarmMasterSwitchEnabled);
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
  digitalWrite(TimeLED,LOW);
}

void setAlarm() {
  analogWrite(AlarmLED,matrixBrightness*255/16+1);
  display_ALAr();
  delay(1000);
  updateTimeDisplay(alarmTime,true,alarmMasterSwitchEnabled);
  waitForButtonDepress(AlarmButton, LOW);
  unsigned long modeActive = millis();
  bool normalExit = true;
  while (debounceDigitalRead(AlarmButton)==HIGH) {
    int16_t encoderDelta = encoder->getValue();
    updateAlarm(encoderDelta);
    modeActive = millis();
    if (static_cast<unsigned long>(millis() - modeActive) > modeInactivePeriod) {
      normalExit = false;
      break;
    }
  }
  if (normalExit) {
    waitForButtonDepress(AlarmButton, LOW);
  }
  updateStartTime();
  //updateTimeDisplay(startTime,true,alarmMasterSwitchEnabled);
  //delay(2000);
  updateCurrentTime(true);
  updateTimeDisplay(current_clock_time,false,alarmMasterSwitchEnabled);
  writeAlarmTimeToEEPROM();
  digitalWrite(AlarmLED,LOW);
  alarmEnabled = true;
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
  turnLightOn();
  waitForButtonDepress(OffButton, HIGH);
  unsigned long modeActive = millis();
  bool normalExit = true;
  while (debounceDigitalRead(OffButton)==LOW) {
    int16_t encoderDelta = encoder->getValue();
    if (encoderDelta != 0) 
    {
      defaultSunSetDelta += encoderDelta;
      defaultSunSetDelta = max(0,defaultSunSetDelta);
      matrix.print(defaultSunSetDelta);
      if (defaultSunSetDelta == 0) matrix.writeDigitNum(4,0); 
      matrix.writeDisplay();
    }
    if (static_cast<unsigned long>(millis() - modeActive) > modeInactivePeriod) {
      normalExit = false;
      break;
    }
  }
  if (defaultSunSetDelta == 0) {
    sunSetActive = false;
    turnLightOff();
  }
  if (normalExit) {
    waitForButtonDepress(OffButton, HIGH);
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
    linearBrightOfStep(0);
  }
  else {
    unsigned long temp = totalDimmerSteps - totalDimmerSteps*delta/finalMillis;
    logisticBrightOfStep(temp);
  }
}

void updateBrightness()
{
  int16_t encoderDelta = encoder->getValue();
  if (encoderDelta != 0) 
  {
    int delta = (encoderDelta>0 ? 1 : -1);
    matrixBrightness = max(min(15,matrixBrightness+delta),0); 
    matrix.setBrightness(matrixBrightness); // 0-15
    writeBrightnessToEEPROM();
  }  
}

void loop() {
  if (digitalRead(AlarmButton)==LOW && debounceDigitalRead(AlarmButton)==LOW) {
    setAlarm();
  } 
  else if (digitalRead(TimeButton)==LOW && debounceDigitalRead(TimeButton)==LOW) {
    setTime();
  } 
  else if (digitalRead(OffButton)==HIGH && debounceDigitalRead(OffButton)==HIGH) {
    if (alarmActive) {
      alarmActive = false;
      alarmEnabled = false;
      turnLightOff();
      sfx.stop();
      waitForButtonDepress(OffButton, HIGH);
    } 
    else if (sunSetActive) {
      sunSetActive = false;
      turnLightOff();
      waitForButtonDepress(OffButton, HIGH);
    }
    else {
      setColorForSunDown();
      setSunSet();
    }
  } 
  if (digitalRead(alarmMasterSwitch) == LOW) {
    alarmMasterSwitchEnabled = false;
    if (alarmEnabled && alarmActive) {
      alarmActive = false;
      alarmEnabled = false;
      turnLightOff();
      sfx.stop();
    }
  }
  else {
    alarmMasterSwitchEnabled = true;
  }
  updateBrightness();
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

