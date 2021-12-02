#include "Menu.h"
#include "Pins.h"
using NixieClock::MenuClass;

MenuClass Menu;

int16_t MenuClass::debounceCounter[16] = {};
bool MenuClass::_buttonState[16] = {};

void MenuClass::begin() {
  memset(debounceCounter, 0, 32);
  memset(_buttonState, 0, 16);
  

	pinMode(PIN_BTN_UP, INPUT_PULLUP);
	pinMode(PIN_BTN_DOWN, INPUT_PULLUP);
	pinMode(PIN_BTN_MODE, INPUT_PULLUP);

}
void MenuClass::update() {
  int current_states[3];
	current_states[BUTTON_UP] = digitalRead(PIN_BTN_UP);
	current_states[BUTTON_DOWN] = digitalRead(PIN_BTN_DOWN);
	current_states[BUTTON_MODE] = digitalRead(PIN_BTN_MODE);

  for(size_t i=0;i<3;++i){
    debounceCounter[i]+= current_states[i] ? ((debounceCounter[i] < debounceCounterMax) ? 1 : 0) : ((debounceCounter[i] > -debounceCounterMax) ? -1 : 0);
    
    if(_buttonState[i] && debounceCounter[i]>debounceThreshold){
      _buttonState[i]=false;
    }else if(!_buttonState[i] && debounceCounter[i]<-debounceThreshold){
      _buttonState[i]=true;
    }
  }
  
}


bool MenuClass::buttonState(const int button) {
  return (button >= 0 && button <= 2) ? _buttonState[button] : false;
}
