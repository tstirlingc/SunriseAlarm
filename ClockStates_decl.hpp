#ifndef CLOCKSTATES_HPP
#define CLOCKSTATES_HPP

namespace SunriseAlarm {
  
// States of the alarm clock:
// CE:  clockEState with alarm enabled through slider switch
//      Slider -> CD
//      AlarmButton -> AE
//      TimeButton -> TE
//      RotaryButton -> SE
//      time -> CEA
// CD:  clockDState with alarm disabled
//      Slider -> CE
//      AlarmButton -> AD
//      TimeButton -> TD
//      RotaryButton -> SD
// AE:  setAlarmEState with alarm enabled
//      AlarmButton -> CE
//      time -> CE
// AD:  setAlarmDState with alarm disabled
//      AlarmButton -> CD
//      time -> CD
// TE:  setTimeEState with alarm enabled
//      TimeButton -> CE
//      time -> CE
// TD:  setTimeDState with alarm disabled
//      TimeButton -> CD
//      time -> CD
// SE:  setSundownEState with alarm enabled
//      RotaryButton with brightness=0 -> CE
//      RotaryButton with brightness>0 -> CES
// SD:  setSundownDState with alarm disabled
//      RotaryButton with brightness=0 -> CD
//      RotaryButton with brightness>0 -> CDS
// CEA:  clockAlarmEState with alarm enabled
//      TouchSensor -> CE
//      time -> CE
//      Slider -> CD
//      AlarmButton -> AE
//      TimeButton -> TE
//      RotaryButton -> SE
//      time -> CEAM
// CEAM:  clockAlarmMusicEState with alarm enabled
//      TouchSensor -> CEA
//      time -> CE
//      Slider -> CD
//      AlarmButton -> AE
//      TimeButton -> TE
//      RotaryButton -> SE
//      
// CES:  clockSunsetEState with alarm enabled
// CDS:  clockSunsetDState with alarm disabled

uint32_t millisAtStartOfSunset = 0;
bool sunsetActive = false;

void stateCE(); 
void changeState_CE_CD();
void changeState_CE_AE();
void changeState_CE_TE();
void changeState_CE_SE();
void changeState_CE_CEA(); // TODO

void stateCD(); // TODO
void changeState_CD_CE(); // TODO
void changeState_CD_AD(); // TODO
void changeState_CD_TD(); // TODO
void changeState_CD_SD(); // TODO

void stateAE();
void changeState_AE_CE();

void stateAD(); // TODO
void changeState_AD_CD(); // TODO

void stateTE();
void changeState_TE_CE();

void stateTD(); // TODO
void changeState_TD_CD(); // TODO

void stateSE();
void changeState_SE_CE();
void changeState_SE_CES(); // TODO

void stateSD(); // TODO
void changeState_SD_CD(); // TODO
void changeState_SD_CDS(); // TODO

void state_CEA(); // TODO
void changeState_CEA_CE(); // TODO
void changeState_CEA_CD(); // TODO
void changeState_CEA_AE(); // TODO
void changeState_CEA_TE(); // TODO
void changeState_CEA_SE(); // TODO
void changeState_CEA_CEAM(); // TODO

void state_CEAM(); // TODO
void changeState_CEAM_CEA(); // TODO
void changeState_CEAM_CE(); // TODO
void changeState_CEAM_CD(); // TODO
void changeState_CEAM_AE(); // TODO
void changeState_CEAM_TE(); // TODO
void changeState_CEAM_SE(); // TODO


void state_CES(); // TODO
void changeState_CES_CE(); // TODO
void changeState_CES_TE(); // TODO
void changeState_CES_AE(); // TODO
void changeState_CES_SE(); // TODO
void changeState_CES_CDS(); // TODO

void state_CDS(); // TODO
void changeState_CDS_CD(); // TODO
void changeState_CDS_TD(); // TODO
void changeState_CDS_AD(); // TODO
void changeState_CDS_SD(); // TODO
void changeState_CDS_CES(); // TODO

} // namespace SunriseAlarm


#endif // CLOCKSTATES_HPP
