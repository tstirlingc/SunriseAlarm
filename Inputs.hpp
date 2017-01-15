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

bool alarmButtonPushed()
{
  return (digitalRead(AlarmButton) == LOW && debounceDigitalRead(AlarmButton) == LOW);
}

bool timeButtonPushed()
{
  return (digitalRead(TimeButton) == LOW && debounceDigitalRead(TimeButton) == LOW);
}

bool touchSensorTouched()
{
  return (digitalRead(TouchSensor) == HIGH && debounceDigitalRead(TouchSensor) == HIGH);
}

bool rotaryButtonPushed()
{
  return (digitalRead(RotaryButton) == LOW && debounceDigitalRead(RotaryButton) == LOW);
}

bool sliderMovedToAlarmEnabled()
{
  return (digitalRead(alarmMasterSwitch) == HIGH && debounceDigitalRead(alarmMasterSwitch) == HIGH);
}

bool sliderMovedToAlarmDisabled()
{
  return (digitalRead(alarmMasterSwitch) == LOW && debounceDigitalRead(alarmMasterSwitch) == LOW);
}

void waitForButtonDepress(int pin, uint8_t pressedState, int debounceInterval) {
  while (debounceDigitalRead(pin, debounceInterval) == pressedState) {
    delay(debounceInterval);
  }
}

int buttonDebounceInterval = 20; // milliseconds
int touchSensorDebounceInterval = 30; // milliseconds

void waitForAlarmButtonDepress()
{
  waitForButtonDepress(AlarmButton, LOW, buttonDebounceInterval);
}

void waitForTimeButtonDepress()
{
  waitForButtonDepress(TimeButton, LOW, buttonDebounceInterval);
}

void waitForRotaryButtonDepress()
{
  waitForButtonDepress(RotaryButton, LOW, buttonDebounceInterval);
}

void waitForTouchSensorDepress()
{
  waitForButtonDepress(TouchSensor, HIGH, touchSensorDebounceInterval);
}

bool waitForSliderToSettleToAlarmEnabled()
{
  waitForButtonDepress(alarmMasterSwitch, LOW, buttonDebounceInterval);
}

bool waitForSliderToSettleToAlarmDisabled()
{
  waitForButtonDepress(alarmMasterSwitch, HIGH, buttonDebounceInterval);
}

#endif // INPUTS_HPP
