#ifndef _NIXIE_H
#define _NIXIE_H

#define PIN_HV5122_OE 10
#define NIXIE_UPDATE_INTERVAL 20000

#include <SPI.h>

class NixieClass {
  private:
    static unsigned long long data;
    static unsigned long lastUpdated;
  public:
  	static void begin();
    static void update();
    static void setDigitsStr(const char * digits);
    static void setDigits(byte * digits);
    static void setNumber(unsigned long num, bool leadingZeros = true);
    static void setDots(bool leftUpper, bool leftLower, bool rightUpper, bool rightLower);
};

extern NixieClass Nixie;

#endif