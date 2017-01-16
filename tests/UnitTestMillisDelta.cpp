#include "gtest/gtest.h"
#include "../MillisDelta.hpp"

TEST(MillisWrap, trivial) {
  uint32_t past = 1000;
  uint32_t current = 1100;
  uint32_t delta = SunriseAlarm::millis_delta(past, current);
  EXPECT_EQ(100, delta);
}

TEST(MillisWrap, acrossZero) {
  uint32_t past = static_cast<uint32_t>(-1000);
  std::cout << "past = " << past << std::endl;
  uint32_t current = 1000;
  uint32_t delta = SunriseAlarm::millis_delta(past, current);
  EXPECT_EQ(2000, delta);
}

TEST(MillisWrap, acrossZero2) {
  uint32_t past = 1000;
  past = past - 2000;
  std::cout << "past = " << past << std::endl;
  uint32_t current = 1000;
  uint32_t delta = SunriseAlarm::millis_delta(past, current);
  EXPECT_EQ(2000, delta);
}

