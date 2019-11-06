
#include <Arduino.h>

#ifndef _NIXIE_CLOCK_MENU_H
#define _NIXIE_CLOCK_MENU_H

#define PIN_BTN_UP A2
#define PIN_BTN_DOWN A1
#define PIN_BTN_MODE A0

#define BUTTON_UP 0
#define BUTTON_DOWN 1
#define BUTTON_MODE 2

namespace NixieClock {
    class MenuClass {
    private:
        static const uint32_t debounceDelay = 100;
        static int buttonLastValue[16];
    public:
        static void begin();
        static void update();

        static void buttonUp( );
        static void buttonDown();
    };

    extern MenuClass Menu;
}

#endif