#include "PreConfig.h"

//////////////////////////////////////////////////////////////////////////////////////
// START OF CONFIG ///////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////

#define _CONFIG_NIXIE_UPDATE_INTERVAL 200*ONE_MILLISECOND

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

#if _CONFIG_GPS_ENABLED
void gps_sync() {
  static uint32_t last_updated = _CONFIG_GPS_SYNC_INTERVAL;
  
  if (millis() - last_updated <= _CONFIG_GPS_SYNC_INTERVAL)
    return;
  
    
  switch (GPSTime.getStatus()) {
    case IDLE: GPSTime.request(); Serial.println("Initiating GPS sync."); break;
    case WAITING: break;
    case FINISHED:
        last_updated = millis();

        GPSTime.reset();
        
        TimeElements timeinfo_utc = {};

        GPSTime.getUtcTime( &timeinfo_utc );

        time_t time_utc = makeTime(timeinfo_utc);
        time_t latency = GPSTime.millisSinceUpdate();

        
        
        
        delay(1000-(latency % 1000));
        time_utc += (latency / 1000)+1;

        TimeKeeper.setEpoch(time_utc);
        
        Serial.print("GPS Sync ");
        Serial.println(time_utc, DEC);
        Serial.write("\r\n");
        break;
  }

}
#endif

void display_update() {
  static uint32_t last_updated = _CONFIG_NIXIE_UPDATE_INTERVAL;
  if (millis() - last_updated <= _CONFIG_NIXIE_UPDATE_INTERVAL)
    return;
  last_updated = millis();

  TimeElements timeinfo_local = {};
  TimeKeeper.getLocalTime(timeinfo_local);
  
  if (Menu.buttonState(BUTTON_UP) || Menu.buttonState(BUTTON_DOWN) || Menu.buttonState(BUTTON_MODE)) {
    
    Display.setNumber( ((unsigned long)timeinfo_local.Month) * 10000 + timeinfo_local.Day * 100 + (((unsigned long)timeinfo_local.Year+1970)%100) );
    Display.setDots(true, false, true, false);
    
    Display.setLed(10,1,0);
  } else {
  
    Display.setNumber( ((unsigned long)timeinfo_local.Hour) * 10000 + timeinfo_local.Minute * 100 + timeinfo_local.Second );
    Display.setLed(0,0,0);
    if ( timeinfo_local.Second & 0x01 ) {
      Display.setDots(true);
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

  alarmMusic.begin(PIN_BUZZER);


  Menu.begin();
}

//// LOOP ////////////////////////////////////////////////////////////////////////////
void loop() {
  Display.update();
  Menu.update();
  alarmMusic.update();

//  if(Menu.buttonState(BUTTON_UP)){
//    alarmMusic.load("fifth:d=4,o=5,b=63:8P,8G5,8G5,8G5,2D#5");
//    alarmMusic.play();
//  }
  
#if _CONFIG_GPS_ENABLED
  GPSTime.update();
  gps_sync();
#endif

  display_update();
}

//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////
