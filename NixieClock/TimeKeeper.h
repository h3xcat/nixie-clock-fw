#ifndef TIME_KEEPER_H
#define TIME_KEEPER_H

//#include <DS1307RTC.h>  // a basic DS1307 library that returns time as a time_t
#include <time.h>

enum DST {NONE, USA, EU};

typedef unsigned char uint8_t;
typedef signed char int8_t;

/*
struct TimeElements
{

}*/

class TimeKeeper
{
	private:
		DST dst;
	public:
		bool isDst();
		void setTimeZone( int8_t timezone );
		int8_t getTimeZone( );

		time_t getLocalTime();
		time_t getTime();


};

#endif
