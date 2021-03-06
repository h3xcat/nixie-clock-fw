#ifndef _NIXIE_CLOCK_DISPLAY_H
#define _NIXIE_CLOCK_DISPLAY_H

#define PIN_HV5122_OE 10
#define PIN_LED_RED 9
#define PIN_LED_GREEN 6
#define PIN_LED_BLUE 3

#define NIXIE_UPDATE_INTERVAL 20


#include <SPI.h>

namespace NixieClock {
    enum class DisplayACP {NONE,SINGLE1,SINGLE2,ALL};

    class DisplayClass {
      private:
        static unsigned long long data;
        static unsigned long long lastData;
        static unsigned long lastUpdated;

        static DisplayACP acpMode;
        static bool acpInProgress;
        static unsigned long acpCycleInterval;
        static unsigned long acpDigitInterval;
        static unsigned long nextAcpCycle;
        static unsigned long nextAcpDigit;
        static unsigned long acpCounter;

        static void acp();
        static void sendData();
      public:
      	static void begin();
        static void update();
        static void setDigitsStr( const char * digits );
        static void setDigits( byte * digits );
        static void setDigit( byte digit, byte val );
        static void setNumber( unsigned long num, bool leadingZeros = true );
        static void setDots( bool leftLower, bool leftUpper, bool rightLower, bool rightUpper );
        static void setDots( bool left, bool right );
        static void setDots( bool on );
        static void setACP( DisplayACP mode, unsigned long cycleInterval = 60000, unsigned long digitInterval = 100 );
        static void setLed( uint8_t red, uint8_t green, uint8_t blue );
    };

    extern DisplayClass Display;
}

#endif
