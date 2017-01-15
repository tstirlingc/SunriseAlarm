#include "ClockStates_decl.hpp"

namespace SunriseAlarm {

bool timeInAlarmWindow() { 
  ClockTime current = current_clock_time;
  int32_t seconds = secondsBtwDates(current, startTime);
  if ((seconds >= 0) && (seconds < oneHourInSeconds)) {
    return true;
  }
  return false; 
}

bool timeForMusicToStart() { 
  ClockTime current = current_clock_time;
  int32_t seconds = secondsBtwDates(current, startTime);
  if ((seconds >= thirtyMinutesInSeconds) && (seconds < oneHourInSeconds)) {
    return true;
  }
  return false;
}

bool timeToEndSnooze() {
  ClockTime current = current_clock_time;
  int32_t seconds = secondsBtwDates(current, snoozeStartTime);
  if (seconds > fiveMinutesInSeconds) {
    return true;
  }
  return false;
}

bool timeForSunsetToBeFinished() { 
  uint32_t currentMillis = millis();
  uint32_t delta = static_cast<uint32_t>(currentMillis - millisAtStartOfSunset);
  uint32_t finalMillis = defaultSunsetDelta;
  finalMillis *= 60;
  finalMillis *= 1000;
  if (delta > finalMillis) return true;
  return false;
}

void stateCE() {
  if (sliderMovedToAlarmDisabled()) changeState_CE_CD();
  if (alarmButtonPushed()) changeState_CE_AE();
  if (timeButtonPushed()) changeState_CE_TE();
  if (rotaryButtonPushed()) changeState_CE_SE();
  if (timeInAlarmWindow()) changeState_CE_CEA(); 
  updateTimeFromRTC24HoursAfterLastTime();
  updateClockDisplayOneSecondAfterLastTime();
}

void changeState_CE_CEA() {
  setColorForSunRise();
  ClockTime current = current_clock_time;
  int32_t seconds = secondsBtwDates(current, startTime);
  alarmStartMillis = static_cast<uint32_t>(millis() - 1000 * seconds);
  stateCEA();
}

void stateCD() {
  if (sliderMovedToAlarmEnabled()) changeState_CD_CE();
  if (alarmButtonPushed()) changeState_CD_AD();
  if (timeButtonPushed()) changeState_CD_TD();
  if (rotaryButtonPushed()) changeState_CD_SD();
  updateTimeFromRTC24HoursAfterLastTime();
  updateClockDisplayOneSecondAfterLastTime();
}

void changeState_CD_CE() {
  alarmMasterSwitchEnabled = true;
  updateClockDisplayNow();
  waitForSliderToSettleToAlarmEnabled();
  stateCE();
}

void changeState_CE_CD()
{
  alarmMasterSwitchEnabled = false;
  updateClockDisplayNow();
  waitForSliderToSettleToAlarmDisabled();
  stateCD();
}

void changeState_C_A_Impl()
{
  analogWrite(AlarmLED, matrixBrightness * 255 / 16 + 1);
  display_ALAr();
  delay(1000);
  updateTimeDisplay(alarmTime, true, alarmMasterSwitchEnabled);
  waitForAlarmButtonDepress();  
}

void changeState_CE_AE()
{
  changeState_C_A_Impl();
  stateAE();
}

void changeState_CD_AD()
{
  changeState_C_A_Impl();
  stateAD();
}

void changeState_C_T_Impl()
{
  analogWrite(TimeLED, matrixBrightness * 255 / 16 + 15);
  display_Cloc();
  delay(1000);
  waitForTimeButtonDepress();
}

void changeState_CE_TE()
{
  changeState_C_T_Impl();
  stateTE();
}

void changeState_CD_TD()
{
  changeState_C_T_Impl();
  stateTD();
}

void changeState_C_S_Impl()
{
  setColorForSunset();
  matrix.print(sunsetDimLevel);
  if (sunsetDimLevel == 0) {
    matrix.writeDigitNum(4, 0);
  }
  matrix.writeDisplay();
  turnLightOn();
  waitForRotaryButtonDepress();
}

void changeState_CE_SE()
{
  changeState_C_S_Impl();
  stateSE();
}

void changeState_CD_SD()
{
  changeState_C_S_Impl();
  stateSD();
}

void stateAImpl()
{
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
}

void stateAE() {
  stateAImpl();
  changeState_AE_CE();
}

void stateAD() {
  stateAImpl();
  changeState_AD_CD();
}

int stateSImpl() {
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
  writeSunsetDimToEEPROM();
  return sunsetDimLevel;
}

void stateSE() {
  if (0 == stateSImpl()) changeState_SE_CE();
  changeState_SE_CES();
}

void stateSD() {
  if (0 == stateSImpl()) changeState_SD_CD();
  changeState_SD_CDS();
}

void changeState_S_C_Impl() {
  turnLightOff();
  updateClockDisplayNow();
}

void changeState_SD_CD() {
  changeState_S_C_Impl();
  stateCD();
}

void changeState_SE_CE()
{
  changeState_S_C_Impl();
  stateCE();
}

void changeState_SE_CES() {
  updateClockDisplayNow();
  stateCES();
}

void changeState_SD_CDS() {
  updateClockDisplayNow();
  stateCDS();
}

void changeState_T_C_Impl()
{
  updateClockDisplayNow();
  digitalWrite(TimeLED, LOW);
}

void changeState_TE_CE()
{
  changeState_T_C_Impl();
  stateSE();
}

void changeState_TD_CD()
{
  changeState_T_C_Impl();
  stateSD();
}

void stateTImpl() {
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
}

void stateTE() {
  stateTImpl();
  changeState_TE_CE();
}

void stateTD() {
  stateTImpl();
  changeState_TD_CD();
}

void changeState_A_C_Impl()
{
  updateStartTime();
  writeAlarmTimeToEEPROM();
  
  updateClockDisplayNow();
  updateTimeDisplay(current_clock_time, false, alarmMasterSwitchEnabled);
  digitalWrite(AlarmLED, LOW);
  alarmEnabled = true;
  snoozeActive = false;
}

void changeState_AE_CE()
{
  changeState_A_C_Impl();
  stateCE();
}

void changeState_AD_CD()
{
  changeState_A_C_Impl();
  stateCD();
}

void stateCEA() {
  if (sliderMovedToAlarmDisabled()) changeState_CEA_CD();
  if (alarmButtonPushed()) changeState_CEA_AE();
  if (timeButtonPushed()) changeState_CEA_TE();
  if (rotaryButtonPushed()) changeState_CEA_SE();
  if (touchSensorTouched() || !timeInAlarmWindow()) changeState_CEA_CE();
  if (timeForMusicToStart()) changeState_CEA_CEAM();
  updateAlarmLight();
  updateTimeFromRTC24HoursAfterLastTime();
  updateClockDisplayOneSecondAfterLastTime();
}

void changeState_CEA_CE() {
  turnLightOff();
  stateCE();
}

void changeState_CEA_CD() {
  turnLightOff();
  alarmMasterSwitchEnabled = false;
  updateClockDisplayNow();
  waitForSliderToSettleToAlarmDisabled();
  stateCD();
}

void changeState_CEA_AE() {
  turnLightOff();
  changeState_C_A_Impl();
  stateAE();
}

void changeState_CEA_TE() {
  turnLightOff();
  changeState_C_T_Impl();
  stateTE();
}

void changeState_CEA_SE() {
  turnLightOff();
  changeState_C_S_Impl();
  stateSE();
}

void changeState_CEA_CEAM() {
  alarmAMusicOn();
  turnLightOn();
  stateCEAM();
}

void stateCEAM() {
  if (sliderMovedToAlarmDisabled()) changeState_CEAM_CD();
  if (alarmButtonPushed()) changeState_CEAM_AE();
  if (timeButtonPushed()) changeState_CEAM_TE();
  if (rotaryButtonPushed()) changeState_CEAM_SE();
  if (touchSensorTouched()) changeState_CEAM_CEAMS();
  if (!timeInAlarmWindow()) changeState_CEAM_CE(); 
  updateTimeFromRTC24HoursAfterLastTime();
  updateClockDisplayOneSecondAfterLastTime();
}

void changeState_CEAM_CEAMS() {
  alarmAMusicOff();
  waitForTouchSensorDepress();
  snoozeStartTime = current_clock_time;
  stateCEAMS();
}

void changeState_CEAM_CE() {
  alarmAMusicOff();
  turnLightOff();
  stateCE();
}

void changeState_CEAM_CD() {
  alarmAMusicOff();
  turnLightOff();
  alarmMasterSwitchEnabled = false;
  updateClockDisplayNow();
  waitForSliderToSettleToAlarmDisabled();
  stateCD();
}

void changeState_CEAM_AE() {
  alarmAMusicOff();
  turnLightOff();
  changeState_C_A_Impl();
  stateAE();
}

void changeState_CEAM_TE() {
  alarmAMusicOff();
  turnLightOff();
  changeState_C_T_Impl();
  stateTE();
}

void changeState_CEAM_SE() {
  alarmAMusicOff();
  turnLightOff();
  changeState_C_S_Impl();
  stateSE();
}

void stateCEAMS() {
  if (sliderMovedToAlarmDisabled()) changeState_CEAMS_CD();
  if (alarmButtonPushed()) changeState_CEAMS_AE();
  if (timeButtonPushed()) changeState_CEAMS_TE();
  if (rotaryButtonPushed()) changeState_CEAMS_SE();
  if (touchSensorTouched() || !timeInAlarmWindow()) changeState_CEAMS_CE();
  if (timeToEndSnooze()) changeState_CEAMS_CEAM();
  updateTimeFromRTC24HoursAfterLastTime();
  updateClockDisplayOneSecondAfterLastTime();
}

void changeState_CEAMS_CD() {
  turnLightOff();
  alarmMasterSwitchEnabled = false;
  updateClockDisplayNow();
  waitForSliderToSettleToAlarmDisabled();
  stateCD();
}

void changeState_CEAMS_AE() {
  turnLightOff();
  changeState_C_A_Impl();
  stateAE();
}

void changeState_CEAMS_TE() {
  turnLightOff();
  changeState_C_T_Impl();
  stateTE();
}

void changeState_CEAMS_SE() {
  turnLightOff();
  changeState_C_S_Impl();
  stateSE();
}

void changeState_CEAMS_CE() {
  turnLightOff();
  waitForTouchSensorDepress();
  stateCE();
}

void changeState_CEAMS_CEAM() {
  alarmAMusicOn();
  stateCEAM();
}



void stateCES() {
  if (sliderMovedToAlarmDisabled()) changeState_CES_CDS();
  if (alarmButtonPushed()) changeState_CES_AE();
  if (timeButtonPushed()) changeState_CES_TE();
  if (rotaryButtonPushed()) changeState_CES_SE();
  if (touchSensorTouched() || timeForSunsetToBeFinished()) changeState_CES_CE();
  updateSunsetLight();
  updateTimeFromRTC24HoursAfterLastTime();
  updateClockDisplayOneSecondAfterLastTime();
}

void changeState_CES_CE() {
  turnLightOff();
  waitForTouchSensorDepress();
  stateCE();
}

void changeState_CES_TE() {
  changeState_C_T_Impl();
  stateTE();
}

void changeState_CES_AE() {
  changeState_C_A_Impl();
  stateAE();
}

void changeState_CES_SE() {
  changeState_C_S_Impl();
  stateSE();
}

void changeState_CES_CDS() {
  alarmMasterSwitchEnabled = false;
  updateClockDisplayNow();
  waitForSliderToSettleToAlarmDisabled();
  stateCDS();
}

void stateCDS() {
  if (sliderMovedToAlarmEnabled()) changeState_CDS_CES();
  if (alarmButtonPushed()) changeState_CDS_AD();
  if (timeButtonPushed()) changeState_CDS_TD();
  if (rotaryButtonPushed()) changeState_CDS_SD();
  if (touchSensorTouched() || timeForSunsetToBeFinished()) changeState_CDS_CD();
  updateSunsetLight();
  updateTimeFromRTC24HoursAfterLastTime();
  updateClockDisplayOneSecondAfterLastTime();
}

void changeState_CDS_CD() {
  turnLightOff();
  waitForTouchSensorDepress();
  stateCD();
}

void changeState_CDS_TD() {
  changeState_C_T_Impl();
  stateTD();
}

void changeState_CDS_AD() {
  changeState_C_A_Impl();
  stateAD();
}

void changeState_CDS_SD() {
  changeState_C_S_Impl();
  stateSD();
}

void changeState_CDS_CES() {
  alarmMasterSwitchEnabled = true;
  updateClockDisplayNow();
  waitForSliderToSettleToAlarmEnabled();
  stateCES();
}


} // namespace SunriseAlarm

