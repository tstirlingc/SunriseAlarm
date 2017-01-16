#include "gtest/gtest.h"
#include "arduino-mock/Arduino.h"
#include "../ClockTime.hpp"

using ::testing::Return; 

TEST(digitalRead, 2) {
    ArduinoMock* arduinoMock = arduinoMockInstance();
    EXPECT_CALL(*arduinoMock, digitalRead(2)).WillOnce(Return(1));
    digitalRead(2);
    releaseArduinoMock();
}

TEST(delay, normal) {
    ArduinoMock* arduinoMock = arduinoMockInstance();
    EXPECT_CALL(*arduinoMock, delay(1));
    delay(1);
    releaseArduinoMock();
}

TEST(ClockTime, trivial) {
  SunriseAlarm::ClockTime ct;
  EXPECT_EQ(0, ct.hour);
  EXPECT_EQ(0, ct.minute);
  EXPECT_EQ(0, ct.second);
}

TEST(ClockTime, plusPlus) {
  SunriseAlarm::ClockTime ct;
  ct++;
  EXPECT_EQ(0, ct.hour);
  EXPECT_EQ(1, ct.minute);
  EXPECT_EQ(0, ct.second);
}

TEST(ClockTime, minusMinus) {
  SunriseAlarm::ClockTime ct;
  ct--;
  EXPECT_EQ(23, ct.hour);
  EXPECT_EQ(59, ct.minute);
  EXPECT_EQ(0, ct.second);
}

TEST(ClockTime, plusEqual) {
  SunriseAlarm::ClockTime ct;
  ct+=65;
  EXPECT_EQ(1, ct.hour);
  EXPECT_EQ(5, ct.minute);
  EXPECT_EQ(0, ct.second);
}

TEST(ClockTime, minusEqual) {
  SunriseAlarm::ClockTime ct;
  ct-=65;
  EXPECT_EQ(22, ct.hour);
  EXPECT_EQ(55, ct.minute);
  EXPECT_EQ(0, ct.second);
}

TEST(ClockTime, fixTime) {
  SunriseAlarm::ClockTime ct(5,62,-5);
  EXPECT_EQ(6, ct.hour);
  EXPECT_EQ(1, ct.minute);
  EXPECT_EQ(55, ct.second);
}

TEST(ClockTime, plusInt) {
  SunriseAlarm::ClockTime ct;
  SunriseAlarm::ClockTime ct2 = ct + 5;
  EXPECT_EQ(0, ct2.hour);
  EXPECT_EQ(5, ct2.minute);
  EXPECT_EQ(0, ct2.second);
}

TEST(ClockTime, lessThan) {
  SunriseAlarm::ClockTime ct;
  SunriseAlarm::ClockTime ct2 = ct + 5;
  EXPECT_LT(ct, ct2);
}

TEST(ClockTime, lessThanEqual) {
  SunriseAlarm::ClockTime ct;
  SunriseAlarm::ClockTime ct2 = ct + 5;
  EXPECT_TRUE(ct <= ct2);
  EXPECT_FALSE(ct2 <= ct);
}

TEST(ClockTime, equal) {
  SunriseAlarm::ClockTime ct;
  SunriseAlarm::ClockTime ct2 = ct + 5;
  EXPECT_FALSE(ct == ct2);
  ct2 -= 5;
  EXPECT_TRUE(ct == ct2);
}

TEST(ClockTime, notEqual) {
  SunriseAlarm::ClockTime ct;
  SunriseAlarm::ClockTime ct2 = ct + 5;
  EXPECT_TRUE(ct != ct2);
}


bool checkIfTimeInWindow(
    uint8_t cH, uint8_t cM, uint8_t cS, 
    uint8_t wsH,uint8_t wsM,uint8_t wsS, 
    uint8_t weH,uint8_t weM,uint8_t weS) {
  SunriseAlarm::ClockTime currentTime(cH,cM,cS);
  SunriseAlarm::ClockTime windowStart(wsH,wsM,wsS);
  SunriseAlarm::ClockTime   windowEnd(weH,weM,weS);
  return SunriseAlarm::isTimeInWindow(currentTime, windowStart, windowEnd);
}
  
int32_t checkNumSecondsFromWindowStart(
    uint8_t cH, uint8_t cM, uint8_t cS, 
    uint8_t wsH,uint8_t wsM,uint8_t wsS, 
    uint8_t weH,uint8_t weM,uint8_t weS) {
  SunriseAlarm::ClockTime currentTime(cH,cM,cS);
  SunriseAlarm::ClockTime windowStart(wsH,wsM,wsS);
  SunriseAlarm::ClockTime   windowEnd(weH,weM,weS);
  return SunriseAlarm::numSecondsFromWindowStartForTimeInWindow(currentTime, windowStart, windowEnd);
}
  

TEST(isTimeInWindow, trivialTrue)       { EXPECT_TRUE (checkIfTimeInWindow(01,30,00, 01,00,00, 02,00,00)); }
TEST(isTimeInWindow, lowerBoundTrue)    { EXPECT_TRUE (checkIfTimeInWindow(01,00,00, 01,00,00, 02,00,00)); }
TEST(isTimeInWindow, lowerBoundFalse)   { EXPECT_FALSE(checkIfTimeInWindow(00,59,59, 01,00,00, 02,00,00)); }
TEST(isTimeInWindow, upperBoundTrue)    { EXPECT_TRUE (checkIfTimeInWindow(01,59,59, 01,00,00, 02,00,00)); }
TEST(isTimeInWindow, upperBoundFalse)   { EXPECT_FALSE(checkIfTimeInWindow(02,00,00, 01,00,00, 02,00,00)); }
TEST(isTimeInWindow, wellOutside)       { EXPECT_FALSE(checkIfTimeInWindow(13,00,00, 01,00,00, 02,00,00)); }
TEST(isTimeInWindow, midnightAtLowerT)  { EXPECT_TRUE (checkIfTimeInWindow(00,00,00, 00,00,00, 01,00,00)); }
TEST(isTimeInWindow, midnightAtLowerF)  { EXPECT_FALSE(checkIfTimeInWindow(23,59,59, 00,00,00, 01,00,00)); }
TEST(isTimeInWindow, midnightAtUpperF)  { EXPECT_FALSE(checkIfTimeInWindow(00,00,00, 23,00,00, 00,00,00)); }
TEST(isTimeInWindow, midnightAtUpperT)  { EXPECT_TRUE (checkIfTimeInWindow(23,59,59, 23,00,00, 00,00,00)); }
TEST(isTimeInWindow, midnightInLowerT)  { EXPECT_TRUE (checkIfTimeInWindow(23,40,00, 23,30,00, 00,30,00)); }
TEST(isTimeInWindow, midnightInUpperT)  { EXPECT_TRUE (checkIfTimeInWindow(00,10,00, 23,30,00, 00,30,00)); }
TEST(isTimeInWindow, midnightInLowerT2) { EXPECT_TRUE (checkIfTimeInWindow(23,30,00, 23,30,00, 00,30,00)); }
TEST(isTimeInWindow, midnightInUpperF2) { EXPECT_FALSE(checkIfTimeInWindow(00,30,00, 23,30,00, 00,30,00)); }
TEST(isTimeInWindow, empty1)            { EXPECT_FALSE(checkIfTimeInWindow(00,00,00, 00,00,00, 00,00,00)); }
TEST(isTimeInWindow, empty2)            { EXPECT_FALSE(checkIfTimeInWindow(00,00,01, 00,00,00, 00,00,00)); }
TEST(isTimeInWindow, empty3)            { EXPECT_FALSE(checkIfTimeInWindow(23,59,59, 00,00,00, 00,00,00)); }
TEST(isTimeInWindow, midnightAtLowerF2) { EXPECT_FALSE(checkIfTimeInWindow(23,59,59, 00,00,00, 01,00,00)); }

TEST(numSecondsFromWindowStart, trivialPositive)  { EXPECT_EQ(1800, checkNumSecondsFromWindowStart(01,30,00, 01,00,00, 02,00,00)); }
TEST(numSecondsFromWindowStart, trivialZero)      { EXPECT_EQ(   0, checkNumSecondsFromWindowStart(01,00,00, 01,00,00, 02,00,00)); }
TEST(numSecondsFromWindowStart, trivialNegative)  { EXPECT_EQ(  -1, checkNumSecondsFromWindowStart(00,59,59, 01,00,00, 02,00,00)); }
TEST(numSecondsFromWindowStart, midnightAtLower1) { EXPECT_EQ(   1, checkNumSecondsFromWindowStart(00,00,01, 00,00,00, 01,00,00)); }
TEST(numSecondsFromWindowStart, midnightAtLower2) { EXPECT_EQ(  -1, checkNumSecondsFromWindowStart(23,59,59, 00,00,00, 01,00,00)); }
TEST(numSecondsFromWindowStart, midnightAtUpper1) { EXPECT_EQ(   1, checkNumSecondsFromWindowStart(23,00,01, 23,00,00, 00,00,00)); }
TEST(numSecondsFromWindowStart, midnightInLower)  { EXPECT_EQ(   1, checkNumSecondsFromWindowStart(23,30,01, 23,30,00, 00,30,00)); }
TEST(numSecondsFromWindowStart, midnightInUpper)  { EXPECT_EQ(1801, checkNumSecondsFromWindowStart(00,00,01, 23,30,00, 00,30,00)); }

