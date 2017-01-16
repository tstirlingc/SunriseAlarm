#include "gtest/gtest.h"
#include "arduino-mock/Arduino.h"

using ::testing::Return; 

namespace SunriseAlarm {

uint32_t millis_delta(uint32_t start, uint32_t finish) {
  return finish-start;
}

} // namespace SunriseAlarm 

TEST(MillisWrap, trivial) {
  uint32_t past = 1000;
  uint32_t current = 1100;
  uint32_t delta = SunriseAlarm::millis_delta(past, current);
  EXPECT_EQ(100, delta);
}
