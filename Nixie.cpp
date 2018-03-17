#include "Nixie.h"

NixieClass Nixie;

unsigned long long NixieClass::data = 0;
unsigned long long NixieClass::lastData = 0;
unsigned long NixieClass::lastUpdated = 0;

NixieACP NixieClass::acpMode = NONE;
bool NixieClass::acpInProgress = 0;
unsigned long NixieClass::acpCycleInterval = 0;
unsigned long NixieClass::acpDigitInterval = 0;
unsigned long NixieClass::nextAcpCycle = 0;
unsigned long NixieClass::nextAcpDigit = 0;
unsigned long NixieClass::acpCounter = 0;


void NixieClass::begin(){
  pinMode(PIN_HV5122_OE, OUTPUT);
  digitalWrite(PIN_HV5122_OE, LOW); 

  SPI.begin();
}
void NixieClass::update(){
  if(acpInProgress) {
    acp();
    return;
  }else if(acpMode != NONE && nextAcpCycle <= millis()){
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

void NixieClass::acp(){
  unsigned long long oldData = data;
  switch(acpMode) {
    case SINGLE1:
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
    case SINGLE2:
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
    case ALL: 
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

void NixieClass::sendData() {
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
void NixieClass::setDigit(byte digit, byte val){
  switch(digit) {
    case 0:
      data &= ~(0x3FFull);
      if(val<10)
        data |= ( 1ULL << (val+0) );
      break;
    case 1:
      data &= ~(0x3FFull<<10);
      if(val<10)
        data |= ( 1ULL << (val+10) );
      break;
    case 2:
      data &= ~(0x3FFull<<20);
      if(val<10)
        data |= ( 1ULL << (val+20) );
      break;
    case 3:
      data &= ~(0x3FFull<<32);
      if(val<10)
        data |= ( 1ULL << (val+32) );
      break;
    case 4:
      data &= ~(0x3FFull<<42);
      if(val<10)
        data |= ( 1ULL << (val+42) );
      break;
    case 5:
      data &= ~(0x3FFull<<52);
      if(val<10)
        data |= ( 1ULL << (val+52) );
      break;
  }
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
void NixieClass::setDots(bool leftLower, bool leftUpper, bool rightLower, bool rightUpper){
  data &= ~( (1ull << 30) | (1ull << 31) | (1ull<<62) | (1ull<<63) );
  data |= ((unsigned long long)leftLower << 30) | ((unsigned long long)leftUpper << 31) | ((unsigned long long)rightUpper << 62) | ((unsigned long long)rightLower << 63);
}
void NixieClass::setDots(bool left, bool right){
  setDots(left, left, right, right);
}
void NixieClass::setDots(bool on){
  setDots(on,on,on,on);
}

void NixieClass::setACP( NixieACP mode, unsigned long cycleInterval = 60000, unsigned long digitInterval = 100 ) {
  acpMode = mode;
  acpCycleInterval = cycleInterval;
  acpDigitInterval = digitInterval;
}