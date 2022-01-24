#include "NixieClockFW.h"

#include <TimeLib.h>
#include <Tone.h>

#include "Menu.h"
#include "DisplayDriver.h"
#include "TimeKeeper.h"
#include "RTTTL.h"

#if _CONFIG_GPS_ENABLED
#include "GPSTime.h"
#endif

//////////////////////////////////////////////////////////////////////////////////////

using NixieClock::DisplayDriver;
using NixieClock::DisplayACP;
using NixieClock::Menu;

RTTTL alarmMusic;


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
enum class ClockStates {
  ST_CLOCK,
  ST_DATE,
  ST_ACP,
};
void display_update() {
  static uint32_t last_updated = _CONFIG_NIXIE_UPDATE_INTERVAL;
  if (millis() - last_updated <= _CONFIG_NIXIE_UPDATE_INTERVAL)
    return;
  last_updated = millis();

  TimeElements timeinfo_local = {};
  TimeKeeper.getLocalTime(timeinfo_local);
  


  static uint32_t state_timer = 10000;

  static uint32_t clock_number = 000000;
  static uint32_t clock_number_live = 000000;
  static uint32_t date_number = 000000;
  static uint32_t date_number_live = 000000;
  
  static uint32_t acpCounter[6] = {0};
  
  clock_number_live = ((unsigned long)timeinfo_local.Hour) * 10000 + timeinfo_local.Minute * 100 + timeinfo_local.Second;
  date_number_live = ((unsigned long)timeinfo_local.Month) * 10000 + timeinfo_local.Day * 100 + (((unsigned long)timeinfo_local.Year+1970)%100);
  static uint32_t transition_updated = 0;
  
  static bool acpReady[6] = {0};
  uint32_t tnumber = 0;
  bool okTransition = false;
  int offset = 0;
  static ClockStates acp_next_state = ClockStates::ST_CLOCK;
  static ClockStates current_state = ClockStates::ST_CLOCK;
  switch (current_state){
    case ClockStates::ST_CLOCK:
      clock_number = clock_number_live;

      DisplayDriver.setNumber( clock_number );
      // DisplayDriver.setLed(0,0,0);

      if ( timeinfo_local.Second & 0x01 ) {
        DisplayDriver.setDots(true);
      } else {
        DisplayDriver.setDots(false);
      }
      DisplayDriver.setLed(0,0,0);

      if (millis() - state_timer > 60000){
        state_timer = millis();
        current_state = ClockStates::ST_ACP;
        acp_next_state = ClockStates::ST_DATE;
        memset(acpCounter, 0, sizeof(acpCounter));
        memset(acpReady, 0, sizeof(acpReady));
      }
      break;
    case ClockStates::ST_DATE:
      date_number = date_number_live;
      
      DisplayDriver.setNumber( date_number );
      DisplayDriver.setDots(true, false, true, false);
      // DisplayDriver.setLed(250,1,0);
      if (millis() - state_timer > 5000){
        state_timer = millis();
        current_state = ClockStates::ST_ACP;
        acp_next_state = ClockStates::ST_CLOCK;
        memset(acpCounter, 0, sizeof(acpCounter));
        memset(acpReady, 0, sizeof(acpReady));
      }
      break;
    case ClockStates::ST_ACP:
      static uint32_t pow10[10] = {1, 10, 100, 1000, 10000, 100000};
      DisplayDriver.setDots(true, true, true, true);
      // DisplayDriver.setLed(0,0,0);
      okTransition = true;
      if (millis() - transition_updated > 200){
        transition_updated = millis();

        // Generate number for Display
        for(size_t i = 0; i<6; ++i){
          acpCounter[i] += 1;
          
          uint32_t digit_live;
          uint32_t digit;
          
          if(acp_next_state == ClockStates::ST_DATE){
            digit_live = (date_number_live/pow10[5-i]) % 10;
            digit = (acpCounter[i]%10 + (clock_number/pow10[5-i])%10) % 10;
          }else if(acp_next_state == ClockStates::ST_CLOCK){
            digit_live = (clock_number_live/pow10[5-i]) % 10;
            digit = (acpCounter[i]%10 + (date_number/pow10[5-i])%10) % 10;
          }

          // Check if digits are ready
          if(!acpReady[i] && acpCounter[i] > 10 && digit == digit_live){
            acpReady[i] = true;
          }

          if(acpReady[i]){
            tnumber += digit_live * pow10[5-i];
          }else{
            tnumber += digit * pow10[5-i];
            okTransition = false;
          }
        }

        Serial.println(tnumber);
        DisplayDriver.setNumber( tnumber );

      }else{
        okTransition = false;
      }

      if (okTransition){
        state_timer = millis();
        current_state = acp_next_state;
      }
      break;
  }
  
  if (Menu.buttonState(BUTTON_UP) || Menu.buttonState(BUTTON_DOWN) || Menu.buttonState(BUTTON_MODE)) {
    // Display date when any of the buttons are pressed down
      
  } else {
    // Display the time


  }
}

//// SETUP ///////////////////////////////////////////////////////////////////////////
void setup() {
  Serial.begin(230400);
  Serial.write("H3xCat's NixieClock Firmware (" NCS_STR ")\r\n");
  
  DisplayDriver.begin();
  TimeKeeper.begin();
  
  #if _CONFIG_GPS_ENABLED
    GPSTime.begin(false, &Serial1);
  #endif
  
  Menu.begin();
  
  // // Enable anti-cathode-poisoning
  // DisplayDriver.setACP(DisplayACP::ALL, 60000, 500);

  TimeKeeper.setDst(DST::USA);
  TimeKeeper.setOffset(-8);
  
  alarmMusic.begin(PIN_BUZZER);
  alarmMusic.load((char *)"Auld L S:d=4,o=6,b=101:g5,c,8c,c,e,d,8c,d,8e,8d,c,8c,e,g,2a,a,g,8e,e,c,d,8c,d,8e,8d,c,8a5,a5,g5,2c");
  //  alarmMusic.play();
}

//// LOOP ////////////////////////////////////////////////////////////////////////////
void loop() {
  DisplayDriver.update();
  Menu.update();
  alarmMusic.update();

 if(Menu.buttonState(BUTTON_UP)){
  TimeKeeper.setDst(DST::USA);
  TimeKeeper.setOffset(-8);
 }
 if(Menu.buttonState(BUTTON_DOWN)){
  TimeKeeper.setDst(DST::NONE);
  TimeKeeper.setOffset(0);
 }
  
#if _CONFIG_GPS_ENABLED
  GPSTime.update();
  gps_sync();
#endif

  display_update();
}

//////////////////////////////////////////////////////////////////////////////////////
