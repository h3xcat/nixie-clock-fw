#include "Display.h"
using NixieClock::DisplayClass;
using NixieClock::DisplayACP;

DisplayClass Display;

unsigned long long DisplayClass::data = 0;
unsigned long long DisplayClass::lastData = 0;
unsigned long DisplayClass::lastUpdated = 0;

DisplayACP DisplayClass::acpMode = DisplayACP::NONE;
bool DisplayClass::acpInProgress = 0;
unsigned long DisplayClass::acpCycleInterval = 0;
unsigned long DisplayClass::acpDigitInterval = 0;
unsigned long DisplayClass::nextAcpCycle = 0;
unsigned long DisplayClass::nextAcpDigit = 0;
unsigned long DisplayClass::acpCounter = 0;


void DisplayClass::begin( ){
  pinMode(PIN_HV5122_OE, OUTPUT);
  pinMode(PIN_LED_RED, OUTPUT);
  pinMode(PIN_LED_GREEN, OUTPUT);
  pinMode(PIN_LED_BLUE, OUTPUT);

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
  unsigned long long oldData = data;
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

        unsigned long digit = acpCounter/100;
        unsigned long val = acpCounter%100;

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
        setNumber(acpCounter*111111ull);
        sendData();
        ++acpCounter;
      }
      break;
  }

  data = oldData;
}

void DisplayClass::sendData( ){
  static SPISettings spiSettings = SPISettings(4e6, MSBFIRST, SPI_MODE2);

  byte * dataArray = ((byte *)(&data));

  digitalWrite(PIN_HV5122_OE, LOW); 

  SPI.beginTransaction(spiSettings);
  for(int i = 7;i>=0;--i)
  SPI.transfer( dataArray[i] );
  SPI.endTransaction();

  digitalWrite(PIN_HV5122_OE, HIGH);
}

void DisplayClass::setDigitsStr( const char * digitsStr ){
  static byte digits[6];
  for(int i = 0; i<6;++i)
    digits[i] = (digitsStr[i]>=0x30 && digitsStr[i]<=0x39) ? (digitsStr[i] - 0x30) : 0xFF;
  setDigits(digits);
}
void DisplayClass::setDigit( byte digit, byte val ) {
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
void DisplayClass::setDigits( byte * digits ){
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
void DisplayClass::setNumber( unsigned long num, bool leadingZeros = true ){
  static byte digits[6] = {0xFF};

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
  data |= ((unsigned long long)leftLower << 30) 
    | ((unsigned long long)leftUpper << 31) 
    | ((unsigned long long)rightUpper << 62) 
    | ((unsigned long long)rightLower << 63);
}
void DisplayClass::setDots( bool left, bool right ){
  setDots(left, left, right, right);
}
void DisplayClass::setDots( bool on ){
  setDots(on,on,on,on);
}

void DisplayClass::setACP( DisplayACP mode, unsigned long cycleInterval = 60000, unsigned long digitInterval = 100 ){
  acpMode = mode;
  acpCycleInterval = cycleInterval;
  acpDigitInterval = digitInterval;
}

void DisplayClass::setLed( uint8_t red, uint8_t green, uint8_t blue ) {
  analogWrite(PIN_LED_RED, red);
  analogWrite(PIN_LED_GREEN, green);
  analogWrite(PIN_LED_BLUE, blue);
}