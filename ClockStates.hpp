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
//      AlarmButton -> SE
//      TimeButton -> TE
//      RotaryButton -> SE
//      time -> CEAM
// CEAM:  clockAlarmMusicEState with alarm enabled
//      TouchSensor -> CEA
//      time -> CE
//      Slider -> CD
//      AlarmButton -> SE
//      TimeButton -> TE
//      RotaryButton -> SE
//      
// CES:  clockSunsetEState with alarm enabled
// CDS:  clockSunsetDState with alarm disabled


void clockState(); 
void changeState_Clock_to_setAlarm();
void changeState_Clock_to_setTime();
void turnOffLightsDueToOffButton();

bool sunsetActive = false;
void changeState_Clock_to_setSunset();


} // namespace SunriseAlarm

#endif // CLOCKSTATES_HPP
