#line 1 "c:\\dev\\nixie-clock-fw\\NixieClockFW\\NixieClockFW.h"
#pragma once

#include "Arduino.h"

#define ONE_MILLISECOND 1ul
#define ONE_SECOND 1000ul
#define ONE_MINUTE (60ul*1000)
#define ONE_HOUR (60ul*60*1000)
#define ONE_DAY (24ul*60*60*1000)

#include "Config.h"


#define _NCS_COUNT (defined(NCS314) + defined(NCS318))

#if _NCS_COUNT > 1
    #error Both NCS314 and NCS318 are defined, only one of them must be defined
#elif _NCS_COUNT == 0
    #error NCS314 or NCS318 has to be defined
#endif


#if defined(NCS314)
    #define NCS_STR "NCS314"
#elif defined(NCS318)
    #define NCS_STR "NCS318"
#endif

#if !defined(ARDUINO_AVR_MEGA2560)
// These are not supported on non-mega boards
  #define _CONFIG_GPS_ENABLED 0
  #define _CONFIG_IR_ENABLED 0
#endif