#include "gtest/gtest.h"
#include "arduino-mock/Arduino.h"
#include "../ClockTime.h"

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
