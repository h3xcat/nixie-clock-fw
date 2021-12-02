#include <Time.h>
#include <TimeLib.h>

#include "TimeKeeper.h"
#include <Arduino.h>

TimeKeeperClass TimeKeeper;

DST TimeKeeperClass::dst = DST::NONE;
int8_t TimeKeeperClass::offset = 0;


uint8_t TimeKeeperClass::Int2Bcd( uint8_t val ) {
	return ((val/10)<<4) + (val%10);
}
uint8_t TimeKeeperClass::Bcd2Int( uint8_t val ) {
	return ((val>>4)*10) + (val&0x0F);
}

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

bool TimeKeeperClass::isDst( time_t utc, int8_t offset, DST dst ) {
	TimeElements tm;
	switch(dst) {
		case DST::USA: 
			breakTime(utc+3600*(long)offset, tm);
			if (tm.Month < 3 || tm.Month > 11) return false;
			if (tm.Month > 3 && tm.Month < 11) return true;
			if (tm.Month == 3) {
				if (tm.Day < 8 ) return false;
				if (tm.Day > 14) return true;

				int secondSunday = 15-getDayOfWeek( 7, 3, (tm.Year+1970)%100, (tm.Year+1970)/100);
        
				if (tm.Day < secondSunday) return false;
				if (tm.Day > secondSunday) return true;
				
				return (tm.Hour >= 2);

			} else if (tm.Month == 11) {
				if (tm.Day > 7) return false;

				int firstSunday = 8-getDayOfWeek( 31, 10, (tm.Year+1970)%100, (tm.Year+1970)/100);

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
				
				int lastSunday = 32-getDayOfWeek(24, 3, (tm.Year+1970)%100, (tm.Year+1970)/100);
				
				if (tm.Day < lastSunday) return false;
				if (tm.Day > lastSunday) return true;
				return (tm.Hour >= 1);
			}
			if (tm.Month == 10) {
				if (tm.Day < 25) return false;
				
				int lastSunday = 32-getDayOfWeek(24, 10, (tm.Year+1970)%100, (tm.Year+1970)/100);
				
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

void TimeKeeperClass::begin( ) {
	Wire.begin();
}

bool TimeKeeperClass::isDst( ) {
	return isDst(getEpoch(), offset, dst);
}

void TimeKeeperClass::getLocalTime( TimeElements &tm ) {
	time_t epoch = getEpoch( );
	breakTime(epoch + offset*3600 + (isDst(epoch, offset, dst) ? 3600 : 0), tm);
}
void TimeKeeperClass::setOffset( int8_t offset ) {
	TimeKeeperClass::offset = offset;
}
int8_t TimeKeeperClass::getOffset( ) {
	return offset;
}
void TimeKeeperClass::setDst( DST dst ) {
	TimeKeeperClass::dst = dst;
}


void TimeKeeperClass::setEpoch( time_t epoch ) {
	TimeElements tm;
	breakTime(epoch, tm);
	setUtc(tm);
}

time_t TimeKeeperClass::getEpoch( ) {
	TimeElements tm;
	getUtc(tm);
	return makeTime(tm);
}



void TimeKeeperClass::setUtc( const TimeElements &tm ) {
	Wire.beginTransmission(DS3231S_ADDRESS);
	Wire.write(0);

	Wire.write(Int2Bcd(tm.Second));
	Wire.write(Int2Bcd(tm.Minute));
	Wire.write(Int2Bcd(tm.Hour));
	Wire.write(Int2Bcd(tm.Wday-1));
	Wire.write(Int2Bcd(tm.Day));
	Wire.write(Int2Bcd(tm.Month));
	Wire.write(Int2Bcd(tm.Year%100));
	Wire.endTransmission();
}


void TimeKeeperClass::getUtc( TimeElements &tm ) {
	Wire.beginTransmission(DS3231S_ADDRESS);
	Wire.write(0);
	Wire.endTransmission();

	Wire.requestFrom(DS3231S_ADDRESS, 7);

	tm.Second = Bcd2Int(Wire.read() & 0x7f);
	tm.Minute = Bcd2Int(Wire.read() & 0x7f);
	tm.Hour = 	Bcd2Int(Wire.read() & 0x3f);
	tm.Wday = 	Bcd2Int(Wire.read() & 0x07) + 1;
	tm.Day = 	Bcd2Int(Wire.read() & 0x3f);
	tm.Month = 	Bcd2Int(Wire.read() & 0x1f);
	tm.Year = 	Bcd2Int(Wire.read() & 0xff);
}
