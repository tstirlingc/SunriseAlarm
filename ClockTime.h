#ifndef CLOCKTIME_H
#define CLOCKTIME_H

namespace SunriseAlarm {
struct ClockTime {
  ClockTime() : hour(0), minute(0), second(0) {}
  ClockTime(int h, int m) : hour(h), minute(m), second(0) {fixTime();}
  ClockTime(int h, int m, int s) : hour(h), minute(m), second(s) {fixTime();}
  ClockTime(const ClockTime & ct) : hour(ct.hour), minute(ct.minute), second(ct.second) {fixTime();}

  ClockTime operator++(int) {
    ClockTime result(*this);
    ++this->minute;
    fixTime();
    return result;
  }

  ClockTime operator+=(int16_t delta)
  {
    ClockTime result(*this);
    this->minute += delta;
    fixTime();
    return result;
  }

  ClockTime operator-=(int16_t delta)
  {
    ClockTime result(*this);
    this->minute -= delta;
    fixTime();
    return result;
  }

  ClockTime operator--(int) {
    ClockTime result(*this);
    --this->minute;
    fixTime();
    return result;
  }

  ClockTime & operator++() {
    ++this->minute;
    fixTime();
    return *this;
  }

  ClockTime & operator--() {
    --this->minute;
    fixTime();
    return *this;
  }

  void fixTime() {
    while (this->second > 59) {
      ++this->minute;
      this->second -= 60;
    }
    while (this->second < 0) {
      --this->minute;
      this->second += 60;
    }

    while (this->minute > 59) {
      ++this->hour;
      this->minute -= 60;
    }
    while (this->minute < 0) {
      --this->hour;
      this->minute += 60;
    }

    while (this->hour > 23) {
      this->hour -= 24;
    }
    while (this->hour < 0) {
      this->hour += 24;
    }
  }

  int hour;
  int minute;
  int second;
};


ClockTime operator+(ClockTime lhs, int minutes) {
  lhs.minute += minutes;
  lhs.fixTime();
  return lhs;
}


bool operator<(const ClockTime& lhs, const ClockTime& rhs) {
  if (lhs.hour < rhs.hour) {
    return true;
  }
  if (lhs.hour > rhs.hour) {
    return false;
  }
  // lhs.hour == rhs.hour
  if (lhs.minute < rhs.minute) {
    return true;
  }
  if (lhs.minute > rhs.minute) {
    return false;
  }
  // lhs.minute == rhs.minute
  if (lhs.second < rhs.second) {
    return true;
  }
  return false;
}

bool operator<=(const ClockTime& lhs, const ClockTime& rhs) {
  if (lhs.hour < rhs.hour) {
    return true;
  }
  if (lhs.hour > rhs.hour) {
    return false;
  }
  // lhs.hour == rhs.hour
  if (lhs.minute < rhs.minute) {
    return true;
  }
  if (lhs.minute > rhs.minute) {
    return false;
  }
  // lhs.minute == rhs.minute
  if (lhs.second <= rhs.second) {
    return true;
  }
  return false;
}

bool operator>(const ClockTime& lhs, const ClockTime& rhs) {
  return !(lhs <= rhs);
}

bool operator>=(const ClockTime& lhs, const ClockTime& rhs) {
  return !(lhs < rhs);
}

bool operator==(const ClockTime& lhs, const ClockTime& rhs) {
  if ((lhs.hour == rhs.hour) && (lhs.minute == rhs.minute) && (lhs.second == rhs.second)) {
    return true;
  }
  return false;
}

bool operator!=(const ClockTime& lhs, const ClockTime& rhs) {
  return !(lhs == rhs);
}

//int32_t secondsBtwDates(const ClockTime& alarm, const ClockTime& currentTime) {
//  int32_t s = (currentTime.hour - alarm.hour);
//  s *= 3600;
//  s += (currentTime.minute - alarm.minute) * 60;
//  s += (currentTime.second - alarm.second);
//  return ( s );
//}

void fixCurrentTimeAndWindowEnd(
    ClockTime& currentTime, 
    const ClockTime& windowStart, 
    ClockTime& windowEnd) {
  if (windowEnd < windowStart && currentTime < windowEnd) {
    windowEnd.hour += 24;
    currentTime.hour += 24;
  } else if (windowEnd < windowStart) {
    windowEnd.hour += 24;
  }
}

bool isTimeInWindow(
    ClockTime currentTime, 
    const ClockTime& windowStart, 
    ClockTime windowEnd) {
  fixCurrentTimeAndWindowEnd(currentTime, windowStart, windowEnd);
  if (currentTime >= windowStart && currentTime < windowEnd) return true;
  return false;
}

int32_t numSecondsFromWindowStartForTimeInWindow(
    ClockTime currentTime, 
    const ClockTime& windowStart, 
    ClockTime windowEnd) {
  if (!isTimeInWindow(currentTime, windowStart, windowEnd)) return -1;
  fixCurrentTimeAndWindowEnd(currentTime, windowStart, windowEnd);
  int32_t s = (currentTime.hour - windowStart.hour);
  s *= 3600;
  s += (currentTime.minute - windowStart.minute) * 60;
  s += (currentTime.second - windowStart.second);
  return ( s );
}

uint32_t millis_delta(uint32_t start, uint32_t finish) {
  return static_cast<uint32_t>(finish-start);
}

} // namespace SunriseAlarm 

#endif // CLOCKTIME_H
