#ifndef _GPS_TIME_H
#define _GPS_TIME_H

#include <TimeLib.h>
#include <HardwareSerial.h>

#include "NixieClockFW.h"

class HardwareSerial;

enum GPSTimeStatus { IDLE, WAITING, FINISHED };

class GPSTimeClass
{
  private:
    static bool autoUpdate;

    static bool connected;

    static tmElements_t utcTime;

    static uint32_t timeUpdated;

    static GPSTimeStatus status;

    static HardwareSerial * gpsSerial;
    static uint32_t serialBaud;
    static uint8_t serialConfig;
  
    static int8_t msgBuf[256];
    static uint32_t msgLen;
    static uint32_t msgTime;

    static uint8_t scanCurSetting;
    static bool scanNextSetting;
    static uint32_t scanTime;
    
    static bool isMessageValid( const int8_t * msg );
    static void getMessageArg( int8_t * dst, const int8_t * msg, uint8_t arg );
    static uint8_t getDayOfWeek( uint32_t day, uint32_t month, uint32_t year, uint32_t cent );
    static void processMessage( const int8_t * msg, uint32_t msgTime );
  public:
    // 
    static void begin( bool autoUpdate, HardwareSerial * serialPort, uint32_t serialBaud = 0, uint8_t serialConfig = SERIAL_8N1);

    // Updates the library, processes serial data, etc.
    static void update();
    
    // Will parse next RMC message and updates its values
    static void request();

    // Returns true when request is fulfilled
    static GPSTimeStatus getStatus();

    // Returns state to IDLE
    static void reset();

    // Tells the library to retrieve time from next RMC message
    static uint32_t millisSinceUpdate();

    static uint16_t getYear();
    static const tmElements_t* getUtcTime();
    static void getUtcTime( tmElements_t* tm );
    static bool isConnected();

};

extern GPSTimeClass GPSTime;

#endif
