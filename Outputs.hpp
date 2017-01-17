#ifndef OUTPUTS_HPP
#define OUTPUTS_HPP

namespace SunriseAlarm {

void alarmAMusicOn()
{
  digitalWrite(soundAPin, LOW);
}

void alarmAMusicOff()
{
  digitalWrite(soundAPin, HIGH);
}

void alarmBMusicOn()
{
  digitalWrite(soundBPin, LOW);
}

void alarmBMusicOff()
{
  digitalWrite(soundBPin, HIGH);
}

void alarmButtonLEDOn() {
  analogWrite(AlarmLED, 1);
}

void alarmButtonLEDOff() {
  analogWrite(AlarmLED, 0);
}

void timeButtonLEDOn() {
  analogWrite(TimeLED, 40);
}

void timeButtonLEDOff() {
  analogWrite(TimeLED, 0);
}

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


void updateTimeDisplay(ClockTime t, bool military, bool dot) {
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
    matrix.writeDigitNum(0, digit0, false);
  }
  matrix.writeDigitNum(1, digit1, false);
  matrix.writeDigitNum(2, 1); // colon
  matrix.writeDigitNum(3, digit3, false);
  matrix.writeDigitNum(4, digit4, dot);
  matrix.writeDisplay();
}


void updateClockDisplayNow() {
  current_clock_time = synchronized_clock_time;
  const uint32_t seconds_delta = millis_delta(synchronized_clock_millis, millis()) / 1000;
  current_clock_time.second += seconds_delta;
  current_clock_time.fixTime();
  updateTimeDisplay(current_clock_time, false, alarmMasterSwitchEnabled);
}

uint32_t lastUpdate = millis();
void updateClockDisplayOneSecondAfterLastTime() {
  if (isTimeNow(lastUpdate, CLOCK_DISPLAY_UPDATE_INTERVAL)) {
    updateClockDisplayNow();
  }
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


} // namespace SunriseAlarm

#endif // OUTPUTS_HPP
