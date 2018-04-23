#include "PreConfig.h"

//////////////////////////////////////////////////////////////////////////////////////
// START OF CONFIG ///////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////

#define _CONFIG_NIXIE_UPDATE_INTERVAL 500*ONE_MILLISECOND

#define _CONFIG_GPS_ENABLED 1 // Enables GPS synchronization on Mega boards
#define _CONFIG_GPS_SYNC_INTERVAL 30*ONE_MINUTE

#define _CONFIG_IR_ENABLED 1 // Enables IR remote functionality

//////////////////////////////////////////////////////////////////////////////////////
// END OF CONFIG /////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////

#define PIN_BUZZER 2

#if !defined(__AVR_ATmega1280__) && !defined(__AVR_ATmega2560__)
  // These are not supported on non-mega boards :(
  #define _CONFIG_GPS_ENABLED 0
  #define _CONFIG_IR_ENABLED 0
#endif

#include <TimeLib.h>
#include <Tone.h>

#include "Display.h"
#include "TimeKeeper.h"
#include "RTTTL.h"

#if _CONFIG_GPS_ENABLED
  #include "GPSTime.h"
#endif

using NixieClock::Display;
using NixieClock::DisplayACP;

//////////////////////////////////////////////////////////////////////////////////////

char * timeStr( const TimeElements &tm ) {
  char s_day[10];
  char s_mon[10];
  strcpy(s_day,dayShortStr(tm.Wday));
  strcpy(s_mon,monthShortStr(tm.Month));
  char str[100];
  sprintf(str,"%s %s %d %d:%02d:%02d %d", s_day, s_mon, tm.Day, tm.Hour, tm.Minute, tm.Second, tm.Year+1970);
  return str;
}

#if _CONFIG_GPS_ENABLED
void gps_sync_check() {
  static unsigned long last_updated = 0;
  
  if(millis()-last_updated > _CONFIG_GPS_SYNC_INTERVAL) {
    switch(GPSTime.getStatus()){
      case IDLE: GPSTime.request(); Serial.println("Initiating GPS sync."); break;
      case WAITING: break;
      case FINISHED:{
        last_updated = millis();

        GPSTime.reset();
        Serial.println("GPS Sync");
        
        TimeElements timeinfo_utc = {};
    
        GPSTime.getUtcTime( &timeinfo_utc );
        
        time_t time_utc = makeTime(timeinfo_utc);
        time_t latency = GPSTime.millisSinceUpdate();
        if(latency%1000 >= 500)
          ++time_utc;
        time_utc += latency/1000;

        TimeKeeper.setEpoch(time_utc);
    
        Serial.write("UTC:   ");
        Serial.write(timeStr(timeinfo_utc));
        Serial.write("\r\n");
      };break;
    }
  }
}
#endif

void display_update_check() {
  static unsigned long last_updated = 0;
  
  if(millis()-last_updated > _CONFIG_NIXIE_UPDATE_INTERVAL){
    last_updated = millis();

    TimeElements timeinfo_local = {};
    TimeKeeper.getLocalTime(timeinfo_local);

    Display.setNumber( ((unsigned long)timeinfo_local.Hour)*10000 + timeinfo_local.Minute*100 + timeinfo_local.Second );
    if( timeinfo_local.Second & 0x01 ){
      Display.setDots(true);
      //Display.setLed(10,1,0);
    }else{
      Display.setDots(false);
      //Display.setLed(0,0,0);
    }
  }
}

//// SETUP ///////////////////////////////////////////////////////////////////////////
RTTTL alarmMusic;
void setup() {
  alarmMusic.begin(PIN_BUZZER);
  alarmMusic.load("HauntHouse: d=4,o=5,b=108: 2a4, 2e, 2d#, 2b4, 2a4, 2c, 2d, 2a#4, 2e., e, 1f4, 1a4, 1d#, 2e., d, 2c., b4, 1a4, 1p, 2a4, 2e, 2d#, 2b4, 2a4, 2c, 2d, 2a#4, 2e., e, 1f4, 1a4, 1d#, 2e., d, 2c., b4, 1a4");
  alarmMusic.play();

  Display.begin();
  TimeKeeper.begin();
  #if _CONFIG_GPS_ENABLED
    GPSTime.begin(false,&Serial1);
  #endif 

  Display.setACP(DisplayACP::ALL, 30000, 100);

  TimeKeeper.setDst(DST::USA);
  TimeKeeper.setTimeZone(-8);
  
  Serial.begin(230400);
  Serial.write("H3xCat's NixieClock Firmware\r\n");
}

//// LOOP ////////////////////////////////////////////////////////////////////////////
void loop() {
  Display.update();

  #if _CONFIG_GPS_ENABLED
    GPSTime.update();
    gps_sync_check();
  #endif
  
  display_update_check();
}

//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////