#include "ClockStates_decl.hpp"

namespace SunriseAlarm {

void stateCE() {
  if (alarmButtonPushed()) changeState_CE_AE();
  else if (timeButtonPushed()) changeState_CE_TE();
  else if (rotaryButtonPushed()) changeState_CE_SE();
  else if (sliderMovedToAlarmDisabled()) changeState_CE_CD();
  
  //updateAlarmLight();
  //updateSunsetLight();
  updateCurrentTime();
  updateClockTime();
}

void changeState_CE_CD()
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

void changeState_CE_AE()
{
  analogWrite(AlarmLED, matrixBrightness * 255 / 16 + 1);
  display_ALAr();
  delay(1000);
  updateTimeDisplay(alarmTime, true, alarmMasterSwitchEnabled);
  waitForAlarmButtonDepress();  
  stateAE();
}

void changeState_CE_TE()
{
  analogWrite(TimeLED, matrixBrightness * 255 / 16 + 15);
  display_Cloc();
  delay(1000);
  waitForTimeButtonDepress();
  stateTE();
}

void processTouchSensor()
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
    waitForTouchSensorDepress();
}

void changeState_CE_SE()
{
  setColorForSunset();
  matrix.print(sunsetDimLevel);
  if (sunsetDimLevel == 0) {
    matrix.writeDigitNum(4, 0);
  }
  matrix.writeDisplay();
  sunsetActive = true;
  turnLightOn();
  waitForRotaryButtonDepress();
  stateSE();
}

void stateAE() {
  
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
    waitForAlarmButtonDepress();
  }
  
  changeState_AE_CE();
}

void stateSE() {
  
  uint32_t modeActive = millis();
  bool normalExit = true;
  while (!rotaryButtonPushed()) {
    int16_t encoderDelta = encoder->getValue();
    if (encoderDelta != 0)
    {
      sunsetDimLevel += encoderDelta;
      sunsetDimLevel = min(100,max(0, sunsetDimLevel));
      matrix.print(sunsetDimLevel);
      if (sunsetDimLevel == 0) matrix.writeDigitNum(4, 0);
      matrix.writeDisplay();
      setColorForSunset();
      turnLightOn();
      modeActive = millis();
    }
    if (static_cast<uint32_t>(millis() - modeActive) > modeInactivePeriod) {
      normalExit = false;
      break;
    }
  }
  if (normalExit) {
    waitForRotaryButtonDepress();
  }
  millisAtStartOfSunset = millis();
  
  changeState_SE_CE();
}

void changeState_SE_CE()
{
  if (sunsetDimLevel == 0) {
    sunsetActive = false;
    turnLightOff();
  }
  writeSunsetDimToEEPROM();
  stateSE();
}

void changeState_TE_CE()
{
  updateCurrentTime(true);
  digitalWrite(TimeLED, LOW);
  stateSE();
}

void stateTE() {

  DateTime currentTime = RTC.now();
  ClockTime t(currentTime.hour(), currentTime.minute(), currentTime.second());
  updateTimeDisplay(t, true, alarmMasterSwitchEnabled);
  
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
    waitForTimeButtonDepress();
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

  changeState_TE_CE();
}

void changeState_AE_CE()
{
  updateStartTime();
  writeAlarmTimeToEEPROM();
  
  updateCurrentTime(true);
  updateTimeDisplay(current_clock_time, false, alarmMasterSwitchEnabled);
  digitalWrite(AlarmLED, LOW);
  alarmEnabled = true;
  snoozeActive = false;
  stateSE();
}

} // namespace SunriseAlarm

