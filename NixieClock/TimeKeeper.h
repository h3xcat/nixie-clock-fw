#ifndef TIME_KEEPER_H
#define TIME_KEEPER_H

//#include <DS1307RTC.h>  // a basic DS1307 library that returns time as a time_t
#include <time.h>

enum class DST {NONE, USA, EU};

typedef unsigned char uint8_t;
typedef signed char int8_t;


class TimeKeeperClass
{
	private:
		static DST dst;
		static time_t epoch;
		static int8_t timezone;

		static uint8_t getDayOfWeek( uint8_t day, uint8_t month, uint8_t year, uint8_t cent );
		static bool isDst( time_t utc, int8_t timezone, DST dst );
	public:
		
		static void setTimeZone( int8_t timezone );
		static void setDst( DST dst );
		static bool isDst();

		static void setEpoch( time_t utc );

		static int8_t getTimeZone( );
		static time_t getEpoch();
		static void getLocalTime( TimeElements &tm );
};

extern TimeKeeperClass TimeKeeper;

#endif
