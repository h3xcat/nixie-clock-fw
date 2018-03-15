#ifndef _NIXIE_H
#define _NIXIE_H

#define PIN_HV5122_OE 10
#define NIXIE_UPDATE_INTERVAL 20

enum NixieACP {NONE,SINGLE1,SINGLE2,ALL};

#include <SPI.h>

class NixieClass {
  private:
    static unsigned long long data;
    static unsigned long long lastData;
    static unsigned long lastUpdated;

    static NixieACP acpMode;
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
    static void setDigitsStr(const char * digits);
    static void setDigits(byte * digits);
    static void setDigit(byte digit, byte val);
    static void setNumber(unsigned long num, bool leadingZeros = true);
    static void setDots( bool leftUpper, bool leftLower, bool rightUpper, bool rightLower );
    static void setDots( bool left, bool right );
    static void setDots( bool on );
    static void setACP( NixieACP mode, unsigned long cycleInterval = 60000, unsigned long digitInterval = 100 );
};

extern NixieClass Nixie;

#endif