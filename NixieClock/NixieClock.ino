#include "PreConfig.h"

//////////////////////////////////////////////////////////////////////////////////////
// START OF CONFIG ///////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////

#define _CONFIG_RTC_SYNC_INTERVAL 30*ONE_SECOND

#define _CONFIG_GPS_ENABLED 1 // Enables GPS synchronization on Mega boards
#define _CONFIG_GPS_SYNC_INTERVAL 10*ONE_SECOND

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
        
        next_check += _CONFIG_GPS_SYNC_INTERVAL;
        
        TimeElements timeinfo_utc = {};
        TimeElements timeinfo_local = {};
    
        GPSTime.getUtcTime( &timeinfo_utc );
        
        time_t time_utc = makeTime(timeinfo_utc);
        if(GPSTime.millisSinceUpdate()>=500)
          ++time_utc;
          
        breakTime(time_utc,timeinfo_local);
    
        Serial.write("UTC:   ");
        Serial.write(timeStr(timeinfo_utc));
        Serial.write("\r\n");
        Serial.write("LOCAL: ");
        Serial.write(timeStr(timeinfo_local));
        Serial.write("\r\n");
        
        setTime(time_utc);
        adjustTime((-8 + 1) * 3600);
      };break;
    }
  }
}


void rtc_sync_check(){
  static unsigned long next_check = millis();
  
  if(next_check<=millis()){
    //next_check += _CONFIG_RTC_SYNC_INTERVAL;
    next_check += 100;
    static unsigned long test = 0;
    test += 1;
    test %= 10;
    Nixie.setNumber(test*111111);
  }
}

void setup(){
  Wire.begin();
  SPI.begin(); 
  Nixie.begin();
  GPSTime.begin(false,&Serial1); 
  
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
