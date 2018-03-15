#include "TimeKeeper.h"
/*
// sun(1), mon(2), tue(3), wed(4), thu(5), fri(6), sat(7)
uint8_t TimeKeeper::getDayOfWeek( uint8_t day, uint8_t month, uint8_t year, uint8_t cent )
{
  // Zeller's congruence
  int q,m,k,j;

  q = utc_day;
  m = utc_month;
  k = utc_year;
  j = cent;

  if(m <= 2)
  {
    m += 12;
    --k;
  }

  return (q + 13*(m+1)/5 + k + k/4 + j/4 + 5*j - 1) % 7 + 1;
}

bool TimeKeeper::isDst()
{
	if(dst == US)
	{
		if(utc_month < 3 || utc_month > 11)
			return false;
		if(utc_month > 3 && utc_month < 11)
			return true;

		if(utc_month == 3)
		{
			uint8_t dst_start_day = 15-getDayOfWeek( 7, 3, utc_year, 20 )

			if(utc_day < dst_start_day)
				return false;
			if(utc_day == dst_start_day && utc_hour < 2)
				return false;
		}
		else if(utc_month == 11)
		{
			uint8_t dst_end_day = 8-getDayOfWeek( 30, 11, utc_year, 20 )

			if(utc_day > dst_end_day)
				return false;
			if(utc_day == dst_end_day && utc_hour >= 2)
				return false;
		}
		
		return true;
	}
	else if(dst == EU)
	{

	}
	else
	{
		return false;
	}
}

time_t TimeKeeper::getLocalTime()
{

}
*/