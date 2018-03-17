#include "PreConfig.h"

//////////////////////////////////////////////////////////////////////////////////////
// START OF CONFIG ///////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////

#define _CONFIG_RTC_SYNC_INTERVAL 30*ONE_SECOND

#define _CONFIG_GPS_ENABLED 1 // Enables GPS synchronization on Mega boards
#define _CONFIG_GPS_SYNC_INTERVAL 1*ONE_SECOND

#define _CONFIG_IR_ENABLED 1 // Enables IR remote functionality

//////////////////////////////////////////////////////////////////////////////////////
// END OF CONFIG /////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////

#if !defined(__AVR_ATmega1280__) && !defined(__AVR_ATmega2560__)
  // These are not supported on non-mega boards :(
  #define _CONFIG_GPS_ENABLED 0
  #define _CONFIG_IR_ENABLED 0
#endif

#include <TimeLib.h>
#include <SPI.h>
#include <Wire.h>
#include <ClickButton.h>
#include <Tone.h>
#include <EEPROM.h>
#include <OneWire.h>

#include "Nixie.h"
#include "TimeKeeper.h"

#if _CONFIG_GPS_ENABLED
  #include "GPSTime.h"
#endif



char * timeStr( const TimeElements &tm )
{
  char s_day[10];
  char s_mon[10];
  strcpy(s_day,dayShortStr(tm.Wday));
  strcpy(s_mon,monthShortStr(tm.Month));
  char str[100];
  sprintf(str,"%s %s %d %d:%02d:%02d %d", s_day, s_mon, tm.Day, tm.Hour, tm.Minute, tm.Second, tm.Year+1970);
  return str;
}
void gps_sync_check(){
  static unsigned long next_check = millis();
  
  if(next_check<=millis()){
    switch(GPSTime.getStatus()){
      case IDLE: GPSTime.request(); Serial.println("Initiating GPS sync."); break;
      case WAITING: break;
      case FINISHED:{
        GPSTime.reset();
        Serial.println("GPS Sync");
        
        next_check = millis() + _CONFIG_GPS_SYNC_INTERVAL;
        
        TimeElements timeinfo_utc = {};
        TimeElements timeinfo_local = {};
    
        GPSTime.getUtcTime( &timeinfo_utc );
        
        time_t time_utc = makeTime(timeinfo_utc);
        time_t latency = GPSTime.millisSinceUpdate();
        time_utc += GPSTime.millisSinceUpdate()/1000;
        if(latency>=500 && latency <1000)
          ++time_utc;

        TimeKeeper.setEpoch(time_utc);
        TimeKeeper.getLocalTime(timeinfo_local);
    
        Serial.write("UTC:   ");
        Serial.write(timeStr(timeinfo_utc));
        Serial.write("\r\n");
        Serial.write("LOCAL: ");
        Serial.write(timeStr(timeinfo_local));
        Serial.write("\r\n");

        
        Nixie.setNumber( ((unsigned long)timeinfo_local.Hour)*10000 + timeinfo_local.Minute*100 + timeinfo_local.Second );
        Nixie.setDots( time_utc % 2 );
        
        setTime(time_utc);
      };break;
    }
  }
}


void rtc_sync_check(){
  static unsigned long next_check = millis();
  
  if(next_check<=millis()){
    next_check += _CONFIG_RTC_SYNC_INTERVAL;
    static unsigned long test = 0;
    test += 1;
    test %= 10;
  }
}

void setup(){
  Wire.begin();
  SPI.begin(); 
  Nixie.begin();
  Nixie.setACP(ALL, 30000, 100);
  GPSTime.begin(false,&Serial1); 

  TimeKeeper.setDst(DST::USA);
  TimeKeeper.setTimeZone(-8);
  
  Serial.begin(230400);
  Serial.write("H3xCat's NixieClock Firmware\r\n");
}

//// LOOP ////////////////////////////////////////////////////////////////////////////

void loop(){
  GPSTime.update();
  Nixie.update();
  
  gps_sync_check();
  rtc_sync_check();
}

//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////
