#include "ClockStates_decl.hpp"

namespace SunriseAlarm {

void alarmButtonLEDOn() {
  analogWrite(AlarmLED, 1);
}

void alarmButtonLEDOff() {
  analogWrite(AlarmLED, 0);
}

void timeButtonLEDOn() {
  analogWrite(TimeLED, 10);
}

void timeButtonLEDOff() {
  analogWrite(TimeLED, 0);
}

bool timeInAlarmWindow() { 
  return isTimeInWindow(current_clock_time, alarmStartTime, alarmEndTime);
}

bool timeForMusicToStart() { 
  return isTimeInWindow(current_clock_time, alarmTime, alarmEndTime);
}

bool timeToEndSnooze() {
  ClockTime current = current_clock_time;
  return !isTimeInWindow(current_clock_time, snoozeStartTime, snoozeEndTime);
}

bool timeForSunsetToBeFinished() { 
  uint32_t currentMillis = millis();
  uint32_t delta = static_cast<uint32_t>(currentMillis - millisAtStartOfSunset);
  if (delta > sunsetDeltaInMilliseconds) return true;
  return false;
}

void stateCE() {
#ifdef DEBUG
  Serial.println(F("stateCE"));
#endif
  while (1) {
    if (sliderMovedToAlarmDisabled()) changeState_CE_CD();
    if (alarmButtonPushed()) changeState_CE_AE();
    if (timeButtonPushed()) changeState_CE_TE();
    if (rotaryButtonPushed()) changeState_CE_SE();
    if (timeInAlarmWindow()) changeState_CE_CEA(); 
    updateTimeFromRTC24HoursAfterLastTime();
    updateClockDisplayOneSecondAfterLastTime();
  }
}

void changeState_CE_CEA() {
#ifdef DEBUG
  Serial.println(F("changeState_CE_CEA"));
#endif
  setColorForSunRise();
  ClockTime current = current_clock_time;
  int32_t seconds = numSecondsFromWindowStartForTimeInWindow(current, alarmStartTime, alarmEndTime);
  alarmStartMillis = static_cast<uint32_t>(millis() - 1000 * seconds);
  stateCEA();
}

void stateCD() {
#ifdef DEBUG
  Serial.println(F("stateCD"));
#endif
  while (1) {
    if (sliderMovedToAlarmEnabled()) changeState_CD_CE();
    if (alarmButtonPushed()) changeState_CD_AD();
    if (timeButtonPushed()) changeState_CD_TD();
    if (rotaryButtonPushed()) changeState_CD_SD();
    updateTimeFromRTC24HoursAfterLastTime();
    updateClockDisplayOneSecondAfterLastTime();
  }
}

void changeState_CD_CE() {
#ifdef DEBUG
  Serial.println(F("changeState_CD_CE"));
#endif
  alarmMasterSwitchEnabled = true;
  updateClockDisplayNow();
  waitForSliderToSettleToAlarmEnabled();
  stateCE();
}

void changeState_CE_CD()
{
#ifdef DEBUG
  Serial.println(F("changeState_CE_CD"));
#endif
  alarmMasterSwitchEnabled = false;
  updateClockDisplayNow();
  waitForSliderToSettleToAlarmDisabled();
  stateCD();
}

void changeState_C_A_Impl()
{
  alarmButtonLEDOn();
  display_ALAr();
  delay(1000);
  updateTimeDisplay(alarmTime, true, alarmMasterSwitchEnabled);
  waitForAlarmButtonDepress();  
}

void changeState_CE_AE()
{
#ifdef DEBUG
  Serial.println(F("changeState_CE_AE"));
#endif
  changeState_C_A_Impl();
  stateAE();
}

void changeState_CD_AD()
{
#ifdef DEBUG
  Serial.println(F("changeState_CD_AD"));
#endif
  changeState_C_A_Impl();
  stateAD();
}

void changeState_C_T_Impl()
{
  timeButtonLEDOn();
  display_Cloc();
  delay(1000);
  waitForTimeButtonDepress();
}

void changeState_CE_TE()
{
#ifdef DEBUG
  Serial.println(F("changeState_CE_TE"));
#endif
  changeState_C_T_Impl();
  stateTE();
}

void changeState_CD_TD()
{
#ifdef DEBUG
  Serial.println(F("changeState_CD_TD"));
#endif
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
#ifdef DEBUG
  Serial.println(F("changeState_CE_SE"));
#endif
  changeState_C_S_Impl();
  stateSE();
}

void changeState_CD_SD()
{
#ifdef DEBUG
  Serial.println(F("changeState_CD_SD"));
#endif
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
    if (static_cast<uint32_t>(millis() - modeActive) > MODE_INACTIVE_PERIOD_MILLIS) {
      normalExit = false;
      break;
    }
  }
  if (normalExit) {
    waitForAlarmButtonDepress();
  }
}

void stateAE() {
#ifdef DEBUG
  Serial.println(F("stateAE"));
#endif
  stateAImpl();
  changeState_AE_CE();
}

void stateAD() {
#ifdef DEBUG
  Serial.println(F("stateAD"));
#endif
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
    if (static_cast<uint32_t>(millis() - modeActive) > MODE_INACTIVE_PERIOD_MILLIS) {
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
#ifdef DEBUG
  Serial.println(F("stateSE"));
#endif
  int dimLevel = stateSImpl();
  if (0 == dimLevel) changeState_SE_CE();
  changeState_SE_CES();
}

void stateSD() {
#ifdef DEBUG
  Serial.println(F("stateSD"));
#endif
  if (0 == stateSImpl()) changeState_SD_CD();
  changeState_SD_CDS();
}

void changeState_S_C_Impl() {
  turnLightOff();
  updateClockDisplayNow();
}

void changeState_SD_CD() {
#ifdef DEBUG
  Serial.println(F("changeState_SD_CD"));
#endif
  changeState_S_C_Impl();
  stateCD();
}

void changeState_SE_CE()
{
#ifdef DEBUG
  Serial.println(F("changeState_SE_CE"));
#endif
  changeState_S_C_Impl();
  stateCE();
}

void changeState_SE_CES() {
#ifdef DEBUG
  Serial.println(F("changeState_SE_CES"));
#endif
  updateClockDisplayNow();
  stateCES();
}

void changeState_SD_CDS() {
#ifdef DEBUG
  Serial.println(F("changeState_SD_CDS"));
#endif
  updateClockDisplayNow();
  stateCDS();
}

void changeState_T_C_Impl()
{
  updateClockDisplayNow();
  timeButtonLEDOff();
}

void changeState_TE_CE()
{
#ifdef DEBUG
  Serial.println(F("changeState_TE_CE"));
#endif
  changeState_T_C_Impl();
  stateCE();
}

void changeState_TD_CD()
{
#ifdef DEBUG
  Serial.println(F("changeState_TD_CD"));
#endif
  changeState_T_C_Impl();
  stateCD();
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
    if (static_cast<uint32_t>(millis() - modeActive) > MODE_INACTIVE_PERIOD_MILLIS) {
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
#ifdef DEBUG
  Serial.println(F("stateTE"));
#endif
  stateTImpl();
  changeState_TE_CE();
}

void stateTD() {
#ifdef DEBUG
  Serial.println(F("stateTD"));
#endif
  stateTImpl();
  changeState_TD_CD();
}

void changeState_A_C_Impl()
{
  updateAlarmStartEndTime();
  writeAlarmTimeToEEPROM();
  
  updateClockDisplayNow();
  updateTimeDisplay(current_clock_time, false, alarmMasterSwitchEnabled);
  alarmButtonLEDOff();
}

void changeState_AE_CE()
{
#ifdef DEBUG
  Serial.println(F("changeState_AE_CE"));
#endif
  changeState_A_C_Impl();
  stateCE();
}

void changeState_AD_CD()
{
#ifdef DEBUG
  Serial.println(F("changeState_AD_CD"));
#endif
  changeState_A_C_Impl();
  stateCD();
}

void stateCEA() {
#ifdef DEBUG
  Serial.println(F("stateCEA"));
#endif
  while (1) {
    if (sliderMovedToAlarmDisabled()) changeState_CEA_CD();
    if (alarmButtonPushed()) changeState_CEA_AE();
    if (timeButtonPushed()) changeState_CEA_TE();
    if (rotaryButtonPushed()) changeState_CEA_SE();
    if (touchSensorTouched()) changeState_CEA_CEOff();
    if (!timeInAlarmWindow()) changeState_CEA_CE();
    if (timeForMusicToStart()) changeState_CEA_CEAM();
    updateAlarmLight();
    updateTimeFromRTC24HoursAfterLastTime();
    updateClockDisplayOneSecondAfterLastTime();
  }
}

void changeState_CEA_CE() {
#ifdef DEBUG
  Serial.println(F("changeState_CEA_CE"));
#endif
  turnLightOff();
  stateCE();
}

void changeState_CEA_CEOff() {
#ifdef DEBUG
  Serial.println(F("changeState_CEA_CEOff"));
#endif
  turnLightOff();
  stateCEOff();
}

void changeState_CEA_CD() {
#ifdef DEBUG
  Serial.println(F("changeState_CEA_CD"));
#endif
  turnLightOff();
  alarmMasterSwitchEnabled = false;
  updateClockDisplayNow();
  waitForSliderToSettleToAlarmDisabled();
  stateCD();
}

void changeState_CEA_AE() {
#ifdef DEBUG
  Serial.println(F("changeState_CEA_AE"));
#endif
  turnLightOff();
  changeState_C_A_Impl();
  stateAE();
}

void changeState_CEA_TE() {
#ifdef DEBUG
  Serial.println(F("changeState_CEA_TE"));
#endif
  turnLightOff();
  changeState_C_T_Impl();
  stateTE();
}

void changeState_CEA_SE() {
#ifdef DEBUG
  Serial.println(F("changeState_CEA_SE"));
#endif
  turnLightOff();
  changeState_C_S_Impl();
  stateSE();
}

void changeState_CEA_CEAM() {
#ifdef DEBUG
  Serial.println(F("changeState_CEA_CEAM"));
#endif
  alarmAMusicOn();
  turnLightOn();
  stateCEAM();
}

void stateCEAM() {
#ifdef DEBUG
  Serial.println(F("stateCEAM"));
#endif
  while (1) {
    if (sliderMovedToAlarmDisabled()) changeState_CEAM_CD();
    if (alarmButtonPushed()) changeState_CEAM_AE();
    if (timeButtonPushed()) changeState_CEAM_TE();
    if (rotaryButtonPushed()) changeState_CEAM_SE();
    if (touchSensorTouched()) changeState_CEAM_CEAMS();
    if (!timeInAlarmWindow()) changeState_CEAM_CE(); 
    updateTimeFromRTC24HoursAfterLastTime();
    updateClockDisplayOneSecondAfterLastTime();
  }
}

void changeState_CEAM_CEAMS() {
#ifdef DEBUG
  Serial.println(F("changeState_CEAM_CEAMS"));
#endif
  alarmAMusicOff();
  waitForTouchSensorDepress();
  snoozeStartTime = current_clock_time;
  snoozeEndTime = snoozeStartTime + snoozeMinutes;
  stateCEAMS();
}

void changeState_CEAM_CE() {
#ifdef DEBUG
  Serial.println(F("changeState_CEAM_CE"));
#endif
  alarmAMusicOff();
  turnLightOff();
  stateCE();
}

void changeState_CEAM_CD() {
#ifdef DEBUG
  Serial.println(F("changeState_CEAM_CD"));
#endif
  alarmAMusicOff();
  turnLightOff();
  alarmMasterSwitchEnabled = false;
  updateClockDisplayNow();
  waitForSliderToSettleToAlarmDisabled();
  stateCD();
}

void changeState_CEAM_AE() {
#ifdef DEBUG
  Serial.println(F("changeState_CEAM_AE"));
#endif
  alarmAMusicOff();
  turnLightOff();
  changeState_C_A_Impl();
  stateAE();
}

void changeState_CEAM_TE() {
#ifdef DEBUG
  Serial.println(F("changeState_CEAM_TE"));
#endif
  alarmAMusicOff();
  turnLightOff();
  changeState_C_T_Impl();
  stateTE();
}

void changeState_CEAM_SE() {
#ifdef DEBUG
  Serial.println(F("changeState_CEAM_SE"));
#endif
  alarmAMusicOff();
  turnLightOff();
  changeState_C_S_Impl();
  stateSE();
}

void stateCEAMS() {
#ifdef DEBUG
  Serial.println(F("stateCEAMS"));
#endif
  while (1) {
    if (sliderMovedToAlarmDisabled()) changeState_CEAMS_CD();
    if (alarmButtonPushed()) changeState_CEAMS_AE();
    if (timeButtonPushed()) changeState_CEAMS_TE();
    if (rotaryButtonPushed()) changeState_CEAMS_SE();
    if (touchSensorTouched()) changeState_CEAMS_CEOff();
    if (!timeInAlarmWindow()) changeState_CEAMS_CE();
    if (timeToEndSnooze()) changeState_CEAMS_CEAM();
    updateTimeFromRTC24HoursAfterLastTime();
    updateClockDisplayOneSecondAfterLastTime();
  }
}

void changeState_CEAMS_CD() {
#ifdef DEBUG
  Serial.println(F("changeState_CEAMS_CD"));
#endif
  turnLightOff();
  alarmMasterSwitchEnabled = false;
  updateClockDisplayNow();
  waitForSliderToSettleToAlarmDisabled();
  stateCD();
}

void changeState_CEAMS_AE() {
#ifdef DEBUG
  Serial.println(F("changeState_CEAMS_AE"));
#endif
  turnLightOff();
  changeState_C_A_Impl();
  stateAE();
}

void changeState_CEAMS_TE() {
#ifdef DEBUG
  Serial.println(F("changeState_CEAMS_TE"));
#endif
  turnLightOff();
  changeState_C_T_Impl();
  stateTE();
}

void changeState_CEAMS_SE() {
#ifdef DEBUG
  Serial.println(F("changeState_CEAMS_SE"));
#endif
  turnLightOff();
  changeState_C_S_Impl();
  stateSE();
}

void changeState_CEAMS_CE() {
#ifdef DEBUG
  Serial.println(F("changeState_CEAMS_CE"));
#endif
  turnLightOff();
  stateCE();
}

void changeState_CEAMS_CEOff() {
#ifdef DEBUG
  Serial.println(F("changeState_CEAMS_CEOff"));
#endif
  turnLightOff();
  waitForTouchSensorDepress();
  stateCEOff();
}

void changeState_CEAMS_CEAM() {
#ifdef DEBUG
  Serial.println(F("changeState_CEAMS_CEAM"));
#endif
  alarmAMusicOn();
  stateCEAM();
}

void stateCES() {
#ifdef DEBUG
  Serial.println(F("stateCES"));
#endif
  while (1) {
    if (sliderMovedToAlarmDisabled()) changeState_CES_CDS();
    if (alarmButtonPushed()) changeState_CES_AE();
    if (timeButtonPushed()) changeState_CES_TE();
    if (rotaryButtonPushed()) changeState_CES_SE();
    if (touchSensorTouched()) changeState_CES_CE();
    if (timeForSunsetToBeFinished()) changeState_CES_CE();
    updateSunsetLight();
    updateTimeFromRTC24HoursAfterLastTime();
    updateClockDisplayOneSecondAfterLastTime();
  }
}

void changeState_CES_CE() {
#ifdef DEBUG
  Serial.println(F("changeState_CES_CE"));
#endif
  turnLightOff();
  waitForTouchSensorDepress();
  stateCE();
}

void changeState_CES_TE() {
#ifdef DEBUG
  Serial.println(F("changeState_CES_TE"));
#endif
  timeButtonLEDOn();
  turnLightOff();
  changeState_C_T_Impl();
  stateTE();
}

void changeState_CES_AE() {
#ifdef DEBUG
  Serial.println(F("changeState_CES_AE"));
#endif
  alarmButtonLEDOn();
  turnLightOff();
  changeState_C_A_Impl();
  stateAE();
}

void changeState_CES_SE() {
#ifdef DEBUG
  Serial.println(F("changeState_CES_SE"));
#endif
  changeState_C_S_Impl();
  stateSE();
}

void changeState_CES_CDS() {
#ifdef DEBUG
  Serial.println(F("changeState_CES_CDS"));
#endif
  alarmMasterSwitchEnabled = false;
  updateClockDisplayNow();
  waitForSliderToSettleToAlarmDisabled();
  stateCDS();
}

void stateCDS() {
#ifdef DEBUG
  Serial.println(F("stateCDS"));
#endif
  while (1) {
    if (sliderMovedToAlarmEnabled()) changeState_CDS_CES();
    if (alarmButtonPushed()) changeState_CDS_AD();
    if (timeButtonPushed()) changeState_CDS_TD();
    if (rotaryButtonPushed()) changeState_CDS_SD();
    if (touchSensorTouched()) changeState_CDS_CD();
    if (timeForSunsetToBeFinished()) changeState_CDS_CD();
    updateSunsetLight();
    updateTimeFromRTC24HoursAfterLastTime();
    updateClockDisplayOneSecondAfterLastTime();
  }
}

void changeState_CDS_CD() {
#ifdef DEBUG
  Serial.println(F("changeState_CDS_CD"));
#endif
  turnLightOff();
  waitForTouchSensorDepress();
  stateCD();
}

void changeState_CDS_TD() {
#ifdef DEBUG
  Serial.println(F("changeState_CDS_TD"));
#endif
  timeButtonLEDOn();
  turnLightOff();
  changeState_C_T_Impl();
  stateTD();
}

void changeState_CDS_AD() {
#ifdef DEBUG
  Serial.println(F("changeState_CDS_AD"));
#endif
  alarmButtonLEDOn();
  turnLightOff();
  changeState_C_A_Impl();
  stateAD();
}

void changeState_CDS_SD() {
#ifdef DEBUG
  Serial.println(F("changeState_CDS_SD"));
#endif
  turnLightOff();
  changeState_C_S_Impl();
  stateSD();
}

void changeState_CDS_CES() {
#ifdef DEBUG
  Serial.println(F("changeState_CDS_CES"));
#endif
  alarmMasterSwitchEnabled = true;
  updateClockDisplayNow();
  waitForSliderToSettleToAlarmEnabled();
  stateCES();
}

void stateCEOff() {
#ifdef DEBUG
  Serial.println(F("stateCEOff"));
#endif
  while (1) {
    if (sliderMovedToAlarmDisabled()) changeState_CEOff_CD();
    if (alarmButtonPushed()) changeState_CEOff_AE();
    if (timeButtonPushed()) changeState_CEOff_TE();
    if (rotaryButtonPushed()) changeState_CEOff_SE();
    if (!timeInAlarmWindow()) changeState_CEOff_CE();
    updateTimeFromRTC24HoursAfterLastTime();
    updateClockDisplayOneSecondAfterLastTime();
  }
}

void changeState_CEOff_CE() {
#ifdef DEBUG
  Serial.println(F("changeState_CEOff_CE"));
#endif
  stateCE();
}

void changeState_CEOff_TE() {
#ifdef DEBUG
  Serial.println(F("changeState_CEOff_TE"));
#endif
  changeState_C_T_Impl();
  stateTE();
}

void changeState_CEOff_AE() {
#ifdef DEBUG
  Serial.println(F("changeState_CEOff_AE"));
#endif
  changeState_C_A_Impl();
  stateAE();
}

void changeState_CEOff_SE() {
#ifdef DEBUG
  Serial.println(F("changeState_CEOff_SE"));
#endif
  changeState_C_S_Impl();
  stateSE();
}

void changeState_CEOff_CD() {
#ifdef DEBUG
  Serial.println(F("changeState_CEOff_CD"));
#endif
  alarmMasterSwitchEnabled = false;
  updateClockDisplayNow();
  waitForSliderToSettleToAlarmDisabled();
  stateCD();
}


} // namespace SunriseAlarm

