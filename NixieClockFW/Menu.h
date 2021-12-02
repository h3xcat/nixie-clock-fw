#ifndef _NIXIE_CLOCK_MENU_H
#define _NIXIE_CLOCK_MENU_H
#include "NixieClockFW.h"

#define BUTTON_UP 0
#define BUTTON_DOWN 1
#define BUTTON_MODE 2

namespace NixieClock {
    class MenuClass {
    private:
        static const int16_t debounceThreshold  = 750;
        static const int16_t debounceCounterMax  = 1000;
        
        static int16_t debounceCounter[16];
        static bool _buttonState[16];
    public:
        static void begin();
        static void update();

        static void buttonUp();
        static void buttonDown();
        static void buttonMode();

        static bool buttonState(const int button);
    };

    extern MenuClass Menu;
}

#endif
