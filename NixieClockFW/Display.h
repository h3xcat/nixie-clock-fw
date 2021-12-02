#ifndef _NIXIE_CLOCK_DISPLAY_H
#define _NIXIE_CLOCK_DISPLAY_H

#define NIXIE_UPDATE_INTERVAL 20

#include <SPI.h>
#include "NixieClockFW.h"

#ifdef NCS318
  #include <Adafruit_NeoPixel.h>
#endif

namespace NixieClock {
    enum class DisplayACP {NONE,SINGLE1,SINGLE2,ALL};

    class DisplayClass {
      private:
        static uint64_t data;
        static uint64_t lastData;
        static uint32_t lastUpdated;

        static DisplayACP acpMode;
        static bool acpInProgress;
        static uint32_t acpCycleInterval;
        static uint32_t acpDigitInterval;
        static uint32_t nextAcpCycle;
        static uint32_t nextAcpDigit;
        static uint32_t acpCounter;

        static void acp();
        static void sendData();
      public:
      	static void begin();
        static void update();
        static void setDigitsStr( const uint8_t * digits );
        static void setDigits( uint8_t * digits );
        static void setDigit( uint8_t digit, uint8_t val );
        static void setNumber( uint32_t num, bool leadingZeros = true );
        static void setDots( bool leftLower, bool leftUpper, bool rightLower, bool rightUpper );
        static void setDots( bool left, bool right );
        static void setDots( bool on );
        static void setACP( DisplayACP mode, uint32_t cycleInterval = 60000, uint32_t digitInterval = 100 );
        static void setLed( uint8_t red, uint8_t green, uint8_t blue );
    };

    extern DisplayClass Display;
}

#endif
