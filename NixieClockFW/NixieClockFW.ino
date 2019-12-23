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

#include "Menu.h"
#include "Display.h"
#include "TimeKeeper.h"
#include "RTTTL.h"

#if _CONFIG_GPS_ENABLED
#include "GPSTime.h"
#endif

using NixieClock::Display;
using NixieClock::DisplayACP;
using NixieClock::Menu;


//////////////////////////////////////////////////////////////////////////////////////

char * timeStr( const TimeElements &tm ) {
  char s_day[10];
  char s_mon[10];
  strcpy(s_day, dayShortStr(tm.Wday));
  strcpy(s_mon, monthShortStr(tm.Month));
  char str[100];
  sprintf(str, "%s %s %d %d:%02d:%02d %d", s_day, s_mon, tm.Day, tm.Hour, tm.Minute, tm.Second, tm.Year + 1970);
  return str;
}

#if _CONFIG_GPS_ENABLED
void gps_sync_check() {
  static unsigned long last_updated = 0;

  if (millis() - last_updated > _CONFIG_GPS_SYNC_INTERVAL) {
    switch (GPSTime.getStatus()) {
      case IDLE: GPSTime.request(); Serial.println("Initiating GPS sync."); break;
      case WAITING: break;
      case FINISHED: {
          last_updated = millis();

          GPSTime.reset();
          Serial.println("GPS Sync");

          TimeElements timeinfo_utc = {};

          GPSTime.getUtcTime( &timeinfo_utc );

          time_t time_utc = makeTime(timeinfo_utc);
          time_t latency = GPSTime.millisSinceUpdate();

          
          Serial.println(time_utc, DEC);
          Serial.println(latency, DEC);

          delay(1000-(latency % 1000));
          
          time_utc += (latency / 1000)+1;

          TimeKeeper.setEpoch(time_utc);
          
          Serial.write("\r\n");
        }; break;
    }
  }
}
#endif

void display_update_check() {
  static unsigned long last_updated = 0;

  if (millis() - last_updated > _CONFIG_NIXIE_UPDATE_INTERVAL) {
    last_updated = millis();

    TimeElements timeinfo_local = {};
    TimeKeeper.getLocalTime(timeinfo_local);

    Display.setNumber( ((unsigned long)timeinfo_local.Hour) * 10000 + timeinfo_local.Minute * 100 + timeinfo_local.Second );
    if ( timeinfo_local.Second & 0x01 ) {
      Display.setDots(true);
      //Display.setLed(10,1,0);
    } else {
      Display.setDots(false);
      //Display.setLed(0,0,0);
    }
  }
}

//// SETUP ///////////////////////////////////////////////////////////////////////////
RTTTL alarmMusic;
void setup() {
  Serial.begin(230400);
  Serial.write("H3xCat's NixieClock Firmware\r\n");

  Display.begin();
  TimeKeeper.begin();
#if _CONFIG_GPS_ENABLED
  GPSTime.begin(false, &Serial1);
#endif

  Display.setACP(DisplayACP::ALL, 60000, 500);

  TimeKeeper.setDst(DST::USA);
  TimeKeeper.setTimeZone(-8);

//
//
//  alarmMusic.begin(PIN_BUZZER);
//  alarmMusic.load("korobyeyniki:d=4,o=5,b=e6,8b,8c6, 8d6,16e6,16d6,8c6,8b,a,8a,8c6,e6,8d6,8c6,b,8b,8c6,d6,e6,c6,a,2a,8p,d6,8f6,a6");
//  alarmMusic.play();

  Menu.begin();
}

//// LOOP ////////////////////////////////////////////////////////////////////////////
void loop() {
  Display.update();
  Menu.update();
  alarmMusic.update();

#if _CONFIG_GPS_ENABLED
  GPSTime.update();
  gps_sync_check();
#endif

  display_update_check();
}

//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////
