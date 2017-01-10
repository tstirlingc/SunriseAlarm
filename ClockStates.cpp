#include "ClockStates.hpp"

void clockState() {
  if (alarmButtonPushed()) changeState_Clock_to_setAlarm();
  else if (timeButtonPushed()) changeState_Clock_to_setTime();
  else if (offButtonPushed()) turnOffLightsDueToOffButton();
  else if (rotaryButtonPushed()) changeState_Clock_to_setSunset();

  processAlarmMasterSwitch();
  updateAlarmLight();
  updateCurrentTime();
  updateClockTime();
  updateSunsetLight();
}

void changeState_Clock_to_setAlarm()
{
  analogWrite(AlarmLED, matrixBrightness * 255 / 16 + 1);
  display_ALAr();
  delay(1000);
  updateTimeDisplay(alarmTime, true, alarmMasterSwitchEnabled);
  waitForButtonDepress(AlarmButton, LOW);  
  setAlarmState();
}

void changeState_Clock_to_setTime()
{
  analogWrite(TimeLED, matrixBrightness * 255 / 16 + 15);
  display_Cloc();
  delay(1000);
  waitForButtonDepress(TimeButton, LOW);
  setTimeState();
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
    else if (sunsetActive) {
      sunsetActive = false;
      turnLightOff();
    }
    else {
      // do nothing
    }
    waitForButtonDepress(OffButton, HIGH);
}

void changeState_Clock_to_setSunset()
{
  setColorForSunset();
  matrix.print(sunsetDimLevel);
  if (sunsetDimLevel == 0) {
    matrix.writeDigitNum(4, 0);
  }
  matrix.writeDisplay();
  sunsetActive = true;
  turnLightOn();
  waitForButtonDepress(RotaryButton, LOW);
  setSunsetState();
}

