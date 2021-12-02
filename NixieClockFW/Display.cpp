#include "Display.h"


using NixieClock::DisplayClass;
using NixieClock::DisplayACP;

DisplayClass Display;

uint64_t DisplayClass::data = 0;
uint64_t DisplayClass::lastData = 0;
uint32_t DisplayClass::lastUpdated = 0;

DisplayACP DisplayClass::acpMode = DisplayACP::NONE;
bool DisplayClass::acpInProgress = 0;
uint32_t DisplayClass::acpCycleInterval = 0;
uint32_t DisplayClass::acpDigitInterval = 0;
uint32_t DisplayClass::nextAcpCycle = 0;
uint32_t DisplayClass::nextAcpDigit = 0;
uint32_t DisplayClass::acpCounter = 0;

#ifdef NCS318
  Adafruit_NeoPixel leds = Adafruit_NeoPixel(NUMLED, PIN_LED, NEO_GRB + NEO_KHZ800);
#endif

void DisplayClass::begin( ){
  pinMode(PIN_HV5122_OE, OUTPUT);

  #ifdef NCS314
    pinMode(PIN_LED_RED, OUTPUT);
    pinMode(PIN_LED_GREEN, OUTPUT);
    pinMode(PIN_LED_BLUE, OUTPUT);
  #endif

  #ifdef NCS318
    leds.begin();
    leds.setBrightness(50);
  #endif

  digitalWrite(PIN_HV5122_OE, LOW); 

  SPI.begin();
}
void DisplayClass::update( ){
  if(acpInProgress) {
    acp();
    return;
  }else if(acpMode != DisplayACP::NONE && nextAcpCycle <= millis()){
    lastData = ~data;
    nextAcpCycle = millis() + acpCycleInterval;
    acpInProgress = true;
    nextAcpDigit = millis(); 
    acp();
    return;
  }

  if(data == lastData)
    return;
  if(millis()-lastUpdated < NIXIE_UPDATE_INTERVAL)
    return;

  lastData = data;
  lastUpdated = millis();

  sendData();
}

void DisplayClass::acp( ){
  uint64_t oldData = data;
  switch(acpMode) {
    case DisplayACP::SINGLE1:
      if(nextAcpDigit <= millis()) {
        if(acpCounter>59) {
          acpCounter = 0;
          acpInProgress = false;
          break;
        }
        
        nextAcpDigit += acpDigitInterval;
        setDigit(acpCounter/10,acpCounter%10);
        sendData();
        ++acpCounter;
      }

      break;
    case DisplayACP::SINGLE2:
      if(nextAcpDigit <= millis()) {
        nextAcpDigit += acpDigitInterval;

        uint32_t digit = acpCounter/100;
        uint32_t val = acpCounter%100;

        if(val > 9) {
          acpCounter = (digit+1)*100;
          acpCounter %= 600;

          acpInProgress = false;
          break;
        }
        setDigit(digit,val%10);
        sendData();

        ++acpCounter;
      }

      break;
    case DisplayACP::ALL: 
      if(nextAcpDigit <= millis()) {
        if(acpCounter>9) {
          acpCounter = 0;
          acpInProgress = false;
          break;
        }

        nextAcpDigit += acpDigitInterval;
        
        uint32_t c = acpCounter;
        
        setNumber(
          ((c+0)%10) * 100000ull + 
          ((c+3)%10) * 10000ull + 
          ((c+6)%10) * 1000ull + 
          ((c+9)%10) * 100ull + 
          ((c+2)%10) * 10ull + 
          ((c+5)%10) * 1ull
        );
        
        sendData();
        ++acpCounter;
      }
      break;
  }

  data = oldData;
}

void DisplayClass::sendData( ){
  static SPISettings spiSettings = SPISettings(4e6, MSBFIRST, SPI_MODE2);

  uint8_t * dataArray = ((uint8_t *)(&data));

  digitalWrite(PIN_HV5122_OE, LOW); 

  SPI.beginTransaction(spiSettings);
  for(int i = 7;i>=0;--i)
  SPI.transfer( dataArray[i] );
  SPI.endTransaction();

  digitalWrite(PIN_HV5122_OE, HIGH);
}

void DisplayClass::setDigitsStr( const uint8_t * digitsStr ){
  static uint8_t digits[6];
  for(int i = 0; i<6;++i)
    digits[i] = (digitsStr[i]>=0x30 && digitsStr[i]<=0x39) ? (digitsStr[i] - 0x30) : 0xFF;
  setDigits(digits);
}
void DisplayClass::setDigit( uint8_t digit, uint8_t val ) {
  switch(digit) {
    case 0:
      data &= ~(0x3FFull);
      if(val<10)
        data |= ( 1ull << (val+0) );
      break;
    case 1:
      data &= ~(0x3FFull<<10);
      if(val<10)
        data |= ( 1ull << (val+10) );
      break;
    case 2:
      data &= ~(0x3FFull<<20);
      if(val<10)
        data |= ( 1ull << (val+20) );
      break;
    case 3:
      data &= ~(0x3FFull<<32);
      if(val<10)
        data |= ( 1ull << (val+32) );
      break;
    case 4:
      data &= ~(0x3FFull<<42);
      if(val<10)
        data |= ( 1ull << (val+42) );
      break;
    case 5:
      data &= ~(0x3FFull<<52);
      if(val<10)
        data |= ( 1ull << (val+52) );
      break;
  }
}
void DisplayClass::setDigits( uint8_t * digits ){
  data &= ~( (0x3FFull) | (0x3FFull<<10) | (0x3FFull<<20) | (0x3FFull<<32) | (0x3FFull<<42) | (0x3FFull<<52) );

  if(digits[0]<10)
    data |= ( 1ull << (digits[0]+0) );
  if(digits[1]<10)
    data |= ( 1ull << (digits[1]+10) );
  if(digits[2]<10)
    data |= ( 1ull << (digits[2]+20) );
  if(digits[3]<10)
    data |= ( 1ull << (digits[3]+32) );
  if(digits[4]<10)
    data |= ( 1ull << (digits[4]+42) );
  if(digits[5]<10)
    data |= ( 1ull << (digits[5]+52) );
}
void DisplayClass::setNumber( uint32_t num, bool leadingZeros ){
  static uint8_t digits[6] = {0xFF};

  for(int i = 5; i>=0;--i){
    if(num > 0){
      digits[i] = num%10;
      num = num / 10;
    }else if(leadingZeros){
      digits[i] = 0;
    }else{
      digits[i] = 0xFF;
    }
    setDigits(digits);
  }
}
void DisplayClass::setDots( bool leftLower, bool leftUpper, bool rightLower, bool rightUpper ){
  data &= ~( (1ull << 30) | (1ull << 31) | (1ull<<62) | (1ull<<63) );
  data |= ((uint64_t)leftLower << 30) 
    | ((uint64_t)leftUpper << 31) 
    | ((uint64_t)rightLower << 62) 
    | ((uint64_t)rightUpper << 63);
}
void DisplayClass::setDots( bool left, bool right ){
  setDots(left, left, right, right);
}
void DisplayClass::setDots( bool on ){
  setDots(on,on,on,on);
}

void DisplayClass::setACP( DisplayACP mode, uint32_t cycleInterval, uint32_t digitInterval ){
  acpMode = mode;
  acpCycleInterval = cycleInterval;
  acpDigitInterval = digitInterval;
}

void DisplayClass::setLed( uint8_t red, uint8_t green, uint8_t blue ) {
  #ifdef NCS314
    analogWrite(PIN_LED_RED, red);
    analogWrite(PIN_LED_GREEN, green);
    analogWrite(PIN_LED_BLUE, blue);
  #endif
  
  #ifdef NCS318
    for(int i=0;i<8;i++){
      leds.setPixelColor(i, leds.Color(red, green, blue)); 
    }
    leds.show();
  #endif
}
