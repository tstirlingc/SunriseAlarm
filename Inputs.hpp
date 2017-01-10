#ifndef INPUTS_HPP
#define INPUTS_HPP

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

#endif // INPUTS_HPP
