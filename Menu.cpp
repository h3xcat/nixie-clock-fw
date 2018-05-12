#include "Menu.h"
using NixieClock::MenuClass;

int MenuClass::buttonLastValue[16] = {};

MenuClass Menu;


void MenuClass::begin() {
	memset(buttonLastValue, 1, sizeof(buttonLastValue));
	pinMode(PIN_BTN_UP, INPUT_PULLUP);
	pinMode(PIN_BTN_DOWN, INPUT_PULLUP);
	pinMode(PIN_BTN_MODE, INPUT_PULLUP);
}
void MenuClass::update() {
	int buttonUp = digitalRead(PIN_BTN_UP);
	int buttonDown = digitalRead(PIN_BTN_DOWN);
	int buttonMode = digitalRead(PIN_BTN_MODE);

	if(buttonUp!=buttonLastValue[BUTTON_UP]){
		buttonLastValue[BUTTON_UP] = buttonUp;
		Serial.print(buttonUp,DEC);
		Serial.write("Up\r\n");
	}
	if(buttonDown!=buttonLastValue[BUTTON_DOWN]){
		buttonLastValue[BUTTON_DOWN] = buttonDown;
		Serial.print(buttonDown,DEC);
		Serial.write("Down\r\n");
	}
	if(buttonMode!=buttonLastValue[BUTTON_MODE]){
		buttonLastValue[BUTTON_MODE] = buttonMode;
		Serial.print(buttonMode,DEC);
		Serial.write("Mode\r\n");
	}
}