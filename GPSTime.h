#ifndef _GPS_TIME_H
#define _GPS_TIME_H

#include <TimeLib.h>
#include <HardwareSerial.h>

typedef unsigned char uint8_t;
typedef unsigned char byte;

class HardwareSerial;

enum GPSTimeStatus { IDLE, WAITING, FINISHED };

class GPSTimeClass
{
  private:
    static bool autoUpdate;

    static tmElements_t utcTime;

    static unsigned long timeUpdated;

    static GPSTimeStatus status;

    static HardwareSerial * gpsSerial;
    static unsigned long serialBaud;
    static byte serialConfig;
  
    static char msgBuf[256];
    static int msgLen;
    static unsigned long msgTime;

    static byte scanCurSetting;
    static bool scanNextSetting;
    static unsigned long scanTime;
    
    static bool isMessageValid( const char * msg );
    static void getMessageArg( char * dst, const char * msg, uint8_t arg );
    static int getDayOfWeek( int day, int month, int year, int cent );
    static void processMessage( const char * msg, unsigned long msgTime );
  public:
    // 
    static void begin( bool autoUpdate, HardwareSerial * serialPort, unsigned long serialBaud = 0, byte serialConfig = SERIAL_8N1);

    // Updates the library, processes serial data, etc.
    static void update();
    
    // Will parse next RMC message and updates its values
    static void request();

    // Returns true when request is fulfilled
    static GPSTimeStatus getStatus();

    // Returns state to IDLE
    static void reset();

    // Tells the library to retrieve time from next RMC message
    static unsigned long millisSinceUpdate();

    static unsigned short int getYear();
    static const tmElements_t* getUtcTime();
    static void getUtcTime( tmElements_t* tm );

};

extern GPSTimeClass GPSTime;

#endif