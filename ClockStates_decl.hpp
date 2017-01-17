#ifndef CLOCKSTATES_HPP
#define CLOCKSTATES_HPP

namespace SunriseAlarm {
  
// States of the alarm clock:
//*CE:  clockEState with alarm enabled through slider switch
//      Slider -> CD
//      AlarmButton -> AE
//      TimeButton -> TE
//      RotaryButton -> SE
//      time -> CEA
//*CD:  clockDState with alarm disabled
//      Slider -> CE
//      AlarmButton -> AD
//      TimeButton -> TD
//      RotaryButton -> SD
//*AE:  setAlarmEState with alarm enabled
//      AlarmButton -> CE
//      time -> CE
//*AD:  setAlarmDState with alarm disabled
//      AlarmButton -> CD
//      time -> CD
//*TE:  setTimeEState with alarm enabled
//      TimeButton -> CE
//      time -> CE
//*TD:  setTimeDState with alarm disabled
//      TimeButton -> CD
//      time -> CD
//*SE:  setSundownEState with alarm enabled
//      RotaryButton with brightness=0 -> CE
//      RotaryButton with brightness>0 -> CES
//*SD:  setSundownDState with alarm disabled
//      RotaryButton with brightness=0 -> CD
//      RotaryButton with brightness>0 -> CDS
//*CEA:  clockAlarmEState with alarm enabled
//      TouchSensor -> CEOff
//      time -> CE
//      Slider -> CD
//      AlarmButton -> AE
//      TimeButton -> TE
//      RotaryButton -> SE
//      time -> CEAM
//*CEAM:  clockAlarmMusicEState with alarm enabled
//      TouchSensor -> CEAMS
//      time -> CE
//      Slider -> CD
//      AlarmButton -> AE
//      TimeButton -> TE
//      RotaryButton -> SE
//*CES:  clockSunsetEState with alarm enabled
//      TouchSensor -> CE
//      time -> CE
//      Slider -> CDS
//      AlarmButton -> AE
//      TimeButton -> TE
//      RotaryButton -> SE
//*CDS:  clockSunsetDState with alarm disabled
//      TouchSensor -> CD
//      time -> CD
//      Slider -> CES
//      AlarmButton -> AD
//      TimeButton -> TD
//      RotaryButton -> SD
//*CEAMS: Snooze: Light on, music turned off
//      time -> CE
//      time -> CEAM
//      touchSensor -> CEOff
//      Slider -> CD
//      AlarmButton -> AE
//      TimeButton -> TE
//      RotaryButton -> SE
//*CEOff:  Turn alarm off.
//      time -> CE
//      Slider -> CD
//      AlarmButton -> AE
//      TimeButton -> TE
//      RotaryButton -> SE

void changeState_Init_CE();

void stateCE();           // done
void changeState_CE_CD(); // done
void changeState_CE_AE(); // done
void changeState_CE_TE(); // done
void changeState_CE_SE(); // done
void changeState_CE_CEA(); // done
void changeState_CE_EE();

void stateCD();           // done
void changeState_CD_CE(); // done
void changeState_CD_AD(); // done
void changeState_CD_TD(); // done
void changeState_CD_SD(); // done

void stateAE();           // done
void changeState_AE_CE(); // done

void stateAD();           // done
void changeState_AD_CD(); // done

void stateTE();           // done
void changeState_TE_CE(); // done

void stateTD();           // done
void changeState_TD_CD(); // done

void stateSE();           // done
void changeState_SE_CE(); // done
void changeState_SE_CES(); // done

void stateSD();           // done
void changeState_SD_CD(); // done
void changeState_SD_CDS(); // done

void stateCEA();         // done
void changeState_CEA_CE(); // done
void changeState_CEA_CEOff(); // done
void changeState_CEA_CD(); // done
void changeState_CEA_AE(); // done
void changeState_CEA_TE(); // done
void changeState_CEA_SE(); // done
void changeState_CEA_CEAM(); // done


void stateCEAM();          // done
void changeState_CEAM_CEAMS(); // done
void changeState_CEAM_CE(); // done
void changeState_CEAM_CD(); // done
void changeState_CEAM_AE(); // done
void changeState_CEAM_TE(); // done
void changeState_CEAM_SE(); // done


void stateCES();         // done
void changeState_CES_CE(); // done
void changeState_CES_TE(); // done
void changeState_CES_AE(); // done
void changeState_CES_SE(); // done
void changeState_CES_CDS(); // done

void stateCDS();         // done 
void changeState_CDS_CD(); // done
void changeState_CDS_TD(); // done
void changeState_CDS_AD(); // done
void changeState_CDS_SD(); // done
void changeState_CDS_CES(); // done

void stateCEAMS();       // done
void changeState_CEAMS_CD(); // done
void changeState_CEAMS_TE(); // done
void changeState_CEAMS_AE(); // done
void changeState_CEAMS_SE(); // done
void changeState_CEAMS_CE(); // done
void changeState_CEAMS_CEOff(); // done
void changeState_CEAMS_CEAM(); // done

void stateCEOff();          // done
void changeState_CEOff_CE(); // done
void changeState_CEOff_TE(); // done
void changeState_CEOff_AE(); // done
void changeState_CEOff_SE(); // done
void changeState_CEOff_CD(); // done

void stateEE();
void changeState_EE_CE();

} // namespace SunriseAlarm


#endif // CLOCKSTATES_HPP
