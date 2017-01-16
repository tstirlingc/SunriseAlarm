#ifndef INPUTS_HPP
#define INPUTS_HPP

int debounceDigitalRead(int pin, int debounceInterval) {
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

int buttonDebounceInterval = 10; // milliseconds
int touchSensorDebounceInterval = 10; // milliseconds
int waitForDepressInterval = 30; // milliseconds

bool alarmButtonPushed()
{
  bool pushed = (digitalRead(AlarmButton) == LOW && debounceDigitalRead(AlarmButton, buttonDebounceInterval) == LOW);
#ifdef DEBUG
  if (pushed) Serial.println(F("alarm button pushed"));
#endif
  return pushed;
}

bool timeButtonPushed()
{
  bool pushed = (digitalRead(TimeButton) == LOW && debounceDigitalRead(TimeButton, buttonDebounceInterval) == LOW);
#ifdef DEBUG
  if (pushed) Serial.println(F("time button pushed"));
#endif
  return pushed;
}

bool touchSensorTouched()
{
  bool pushed = (digitalRead(TouchSensor) == HIGH && debounceDigitalRead(TouchSensor, touchSensorDebounceInterval) == HIGH);
#ifdef DEBUG
  if (pushed) Serial.println(F("touch sensor touched"));
#endif
  return pushed;
}

bool rotaryButtonPushed()
{
  bool pushed = (digitalRead(RotaryButton) == LOW && debounceDigitalRead(RotaryButton, buttonDebounceInterval) == LOW);
#ifdef DEBUG
  if (pushed) Serial.println(F("rotary button pushed"));
#endif
  return pushed;
}

bool sliderMovedToAlarmEnabled()
{
  bool enabled = (digitalRead(alarmMasterSwitch) == HIGH && debounceDigitalRead(alarmMasterSwitch, buttonDebounceInterval) == HIGH);
#ifdef DEBUG
  if (enabled) Serial.println(F("slider enabled"));
#endif
  return enabled;
}

bool sliderMovedToAlarmDisabled()
{
  bool disabled = (digitalRead(alarmMasterSwitch) == LOW && debounceDigitalRead(alarmMasterSwitch, buttonDebounceInterval) == LOW);
#ifdef DEBUG
  if (disabled) Serial.println(F("slider disabled"));
#endif
  return disabled;
}

void waitForButtonDepress(int pin, uint8_t pressedState, int waitInterval) {
  while (debounceDigitalRead(pin, waitInterval) == pressedState) {
    delay(waitInterval);
  }
}



void waitForAlarmButtonDepress()
{
  waitForButtonDepress(AlarmButton, LOW, waitForDepressInterval);
#ifdef DEBUG
  Serial.println(F("alarm button depressed"));
#endif
}

void waitForTimeButtonDepress()
{
  waitForButtonDepress(TimeButton, LOW, waitForDepressInterval);
#ifdef DEBUG
  Serial.println(F("time button depressed"));
#endif
}

void waitForRotaryButtonDepress()
{
  waitForButtonDepress(RotaryButton, LOW, waitForDepressInterval);
#ifdef DEBUG
  Serial.println(F("rotary button depressed"));
#endif
}

void waitForTouchSensorDepress()
{
  waitForButtonDepress(TouchSensor, HIGH, waitForDepressInterval);
#ifdef DEBUG
  Serial.println(F("touch sensor depressed"));
#endif
}

bool waitForSliderToSettleToAlarmEnabled()
{
  waitForButtonDepress(alarmMasterSwitch, LOW, waitForDepressInterval);
#ifdef DEBUG
  Serial.println(F("slider settled to enabled"));
#endif
}

bool waitForSliderToSettleToAlarmDisabled()
{
  waitForButtonDepress(alarmMasterSwitch, HIGH, waitForDepressInterval);
#ifdef DEBUG
  Serial.println(F("slider settled to disabled"));
#endif
}

#endif // INPUTS_HPP
