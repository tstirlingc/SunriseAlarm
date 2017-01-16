#ifndef MILLIS_DELTA_H
#define MILLIS_DELTA_H

namespace SunriseAlarm {

uint32_t millis_delta(uint32_t start, uint32_t finish) {
  return static_cast<uint32_t>(finish-start);
}

} // namespace SunriseAlarm 

#endif // MILLIS_DELTA_H
