#include "Nixie.h"

NixieClass Nixie;

unsigned long long NixieClass::data = 0;
unsigned long NixieClass::lastUpdated = 0;

void NixieClass::begin(){
  pinMode(PIN_HV5122_OE, OUTPUT);
  digitalWrite(PIN_HV5122_OE, LOW); 

  SPI.begin();
}
void NixieClass::update(){
  if(micros()-lastUpdated < NIXIE_UPDATE_INTERVAL)
    return;
  lastUpdated = micros();

  static SPISettings spiSettings = SPISettings(4e6, MSBFIRST, SPI_MODE2);

  byte * dataArray = ((byte *)(&data));

  digitalWrite(PIN_HV5122_OE, LOW); 

  SPI.beginTransaction(spiSettings);
  for(int i = 7;i>=0;--i)
  SPI.transfer( dataArray[i] );
  SPI.endTransaction();

  digitalWrite(PIN_HV5122_OE, HIGH);
}

void NixieClass::setDigitsStr(const char * digitsStr){
  static byte digits[6];
  for(int i = 0; i<6;++i)
    digits[i] = (digitsStr[i]>=0x30 && digitsStr[i]<=0x39) ? digitsStr[i] - 0x30 : 0xFF;
  setDigits(digits);
}
void NixieClass::setDigits(byte * digits){
  data &= ~( (0x3FFull) | (0x3FFull<<10) | (0x3FFull<<20) | (0x3FFull<<32) | (0x3FFull<<42) | (0x3FFull<<52) );

  if(digits[0]<10)
    data |= ( 1ULL << (digits[0]+0) );
  if(digits[1]<10)
    data |= ( 1ULL << (digits[1]+10) );
  if(digits[2]<10)
    data |= ( 1ULL << (digits[2]+20) );
  if(digits[3]<10)
    data |= ( 1ULL << (digits[3]+32) );
  if(digits[4]<10)
    data |= ( 1ULL << (digits[4]+42) );
  if(digits[5]<10)
    data |= ( 1ULL << (digits[5]+52) );
}
void NixieClass::setNumber(unsigned long num, bool leadingZeros = true){
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
void NixieClass::setDots(bool leftUpper, bool leftLower, bool rightUpper, bool rightLower){
}