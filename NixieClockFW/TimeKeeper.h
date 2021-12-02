#ifndef TIME_KEEPER_H
#define TIME_KEEPER_H

#include "NixieClockFW.h"

#define DS3231S_ADDRESS 0x68

#include <TimeLib.h>
#include <Wire.h>

enum class DST {NONE, USA, EU};

typedef unsigned char uint8_t;
typedef signed char int8_t;

class TimeKeeperClass
{
	private:
		static DST dst;
		static int8_t offset;

		static uint8_t getDayOfWeek( uint8_t day, uint8_t month, uint8_t year, uint8_t cent );
		static bool isDst( time_t utc, int8_t offset, DST dst );
		static uint8_t Int2Bcd( uint8_t val );
		static uint8_t Bcd2Int( uint8_t val );
	public:
		static void begin();

		static void setOffset( int8_t offset );
		static void setDst( DST dst );
		static bool isDst();

		static void setEpoch( time_t utc );
		static time_t getEpoch();

		static void setUtc( const TimeElements &tm );
		static void getUtc( TimeElements &tm );

		static int8_t getOffset( );

		static void getLocalTime( TimeElements &tm );
};

extern TimeKeeperClass TimeKeeper;

#endif
