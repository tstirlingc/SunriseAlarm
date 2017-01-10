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


inline ClockTime operator+(ClockTime lhs, int minutes) {
  lhs.minute += minutes;
  lhs.fixTime();
  return lhs;
}


inline bool operator<(ClockTime lhs, ClockTime rhs) {
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

inline bool operator<=(ClockTime lhs, ClockTime rhs) {
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

inline bool operator==(ClockTime lhs, ClockTime rhs) {
  if ((lhs.hour == rhs.hour) && (lhs.minute == rhs.minute) && (lhs.second == rhs.second)) {
    return true;
  }
  return false;
}

inline bool operator!=(ClockTime lhs, ClockTime rhs) {
  return !(lhs == rhs);
}


} // namespace SunriseAlarm 

#endif // CLOCKTIME_H
