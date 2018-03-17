#include "TimeKeeper.h"
#include <Arduino.h>

TimeKeeperClass TimeKeeper;

DST TimeKeeperClass::dst = DST::NONE;
time_t TimeKeeperClass::epoch = 0;
int8_t TimeKeeperClass::timezone = 0;

// sun(1), mon(2), tue(3), wed(4), thu(5), fri(6), sat(7)
uint8_t TimeKeeperClass::getDayOfWeek( uint8_t day, uint8_t month, uint8_t year, uint8_t cent )
{
  // Zeller's congruence
  int q = day;
  int m = month;
  int k = year;
  int j = cent;

  if(m <= 2) {
    m += 12;
    --k;
  }

  return (q + 13*(m+1)/5 + k + k/4 + j/4 + 5*j - 1) % 7 + 1;
}

bool TimeKeeperClass::isDst( time_t utc, int8_t timezone, DST dst ) {
	TimeElements tm;
	switch(dst) {
		case DST::USA: 
			breakTime(utc+3600*timezone, tm);
			if (tm.Month < 3 || tm.Month > 11) return false;
			if (tm.Month > 3 && tm.Month < 11) return true;
			if (tm.Month == 3) {
				if (tm.Day < 8 ) return false;
				if (tm.Day > 14) return true;

				int secondSunday = 15-getDayOfWeek( 7, 3, tm.Year%100, tm.Year/100);

				if (tm.Day < secondSunday) return false;
				if (tm.Day > secondSunday) return true;
				
				return (tm.Hour >= 2);

			} else if (tm.Month == 11) {
				if (tm.Day > 7) return false;

				int firstSunday = 8-getDayOfWeek( 31, 10, tm.Year%100, tm.Year/100);

				if (tm.Day < firstSunday) return true;
				if (tm.Day > firstSunday) return false;
				
				return (tm.Hour < 2);
			}
			break;
		case DST::EU:
			breakTime(utc, tm);

			if (tm.Month < 3 || tm.Month > 10) return false;
			if (tm.Month > 3 && tm.Month < 10) return true;

			if (tm.Month == 3) {
				if (tm.Day < 25) return false;
				
				int lastSunday = 32-getDayOfWeek(24, 3, tm.Year%100, tm.Year/100);
				
				if (tm.Day < lastSunday) return false;
				if (tm.Day > lastSunday) return true;
				return (tm.Hour >= 1);
			}
			if (tm.Month == 10) {
				if (tm.Day < 25) return false;
				
				int lastSunday = 32-getDayOfWeek(24, 10, tm.Year%100, tm.Year/100);
				
				if (tm.Day < lastSunday) return true;
				if (tm.Day > lastSunday) return false;
				return (tm.Hour < 1);
			}
			break;
		case DST::NONE:
			return false;
			break;
	}
	return false;
}

bool TimeKeeperClass::isDst( ) {
	return isDst(epoch, timezone, dst);
}

void TimeKeeperClass::getLocalTime( TimeElements &tm ) {
	breakTime(epoch + timezone*3600 + (isDst(epoch, timezone, dst) ? 3600 : 0), tm);
}
void TimeKeeperClass::setTimeZone( int8_t timezone ) {
	TimeKeeperClass::timezone = timezone;
}
int8_t TimeKeeperClass::getTimeZone( ) {
	return timezone;
}
time_t TimeKeeperClass::getEpoch( ) {
	return epoch;
}
void TimeKeeperClass::setDst( DST dst ) {
	TimeKeeperClass::dst = dst;
}
void TimeKeeperClass::setEpoch( time_t epoch ) {
	TimeKeeperClass::epoch = epoch;
}