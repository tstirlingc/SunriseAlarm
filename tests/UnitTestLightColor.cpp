#include "gtest/gtest.h"
#include "arduino-mock/Arduino.h"

#define NUM_LED 32
int sunsetDimLevel = 100; // 100 = max bright, 0 = off
#define min std::min
#include "../LightColor.h"


TEST(LightColor, computeMinRGBLevel) {
  uint8_t RGB[3] = {1, 2, 3};
  uint8_t minRGB = SunriseAlarm::computeMinRGBLevel(RGB);
  EXPECT_EQ(1, minRGB);
}

TEST(LightColor, computeMinRGBLevel_zero) {
  uint8_t RGB[3] = {0, 2, 3};
  uint8_t minRGB = SunriseAlarm::computeMinRGBLevel(RGB);
  EXPECT_EQ(2, minRGB);
}

TEST(LightColor, setColorForSunset) {
  SunriseAlarm::setColorForSunset();
  EXPECT_EQ(SunriseAlarm::candleLight[0], SunriseAlarm::targetColor[0]);
  EXPECT_EQ(SunriseAlarm::candleLight[1], SunriseAlarm::targetColor[1]);
  EXPECT_EQ(SunriseAlarm::candleLight[2], SunriseAlarm::targetColor[2]);
  uint8_t minRGB = SunriseAlarm::computeMinRGBLevel(SunriseAlarm::candleLight);
  EXPECT_EQ(minRGB, SunriseAlarm::minRGBLevel);
  EXPECT_EQ(minRGB*NUM_LED, SunriseAlarm::totalDimmerSteps);
}

TEST(LightColor, setColorForSunRise) {
  SunriseAlarm::setColorForSunRise();
  uint8_t dimLevel = SunriseAlarm::sunriseDimLevel;
  uint8_t color[3];
  color[0] = (1.0*dimLevel/100.0)*SunriseAlarm::kitchen[0];
  color[1] = (1.0*dimLevel/100.0)*SunriseAlarm::kitchen[1];
  color[2] = (1.0*dimLevel/100.0)*SunriseAlarm::kitchen[2];
  EXPECT_EQ(color[0], SunriseAlarm::targetColor[0]);
  EXPECT_EQ(color[1], SunriseAlarm::targetColor[1]);
  EXPECT_EQ(color[2], SunriseAlarm::targetColor[2]);
  uint8_t minRGB = SunriseAlarm::computeMinRGBLevel(color);
  EXPECT_EQ(minRGB, SunriseAlarm::minRGBLevel);
  EXPECT_EQ(minRGB*NUM_LED, SunriseAlarm::totalDimmerSteps);
}
