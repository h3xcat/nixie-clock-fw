#include "GPSTime.h"
#include "Arduino.h"

GPSTimeClass GPSTime;


bool GPSTimeClass::autoUpdate = false;
tmElements_t GPSTimeClass::utcTime = {};
uint32_t GPSTimeClass::timeUpdated = 0;
GPSTimeStatus GPSTimeClass::status = IDLE;
HardwareSerial * GPSTimeClass::gpsSerial = NULL;
uint32_t GPSTimeClass::serialBaud = 0;
byte GPSTimeClass::serialConfig = 0;
int8_t GPSTimeClass::msgBuf[256] = {};
uint32_t GPSTimeClass::msgLen = 0;
uint32_t GPSTimeClass::msgTime = 0;
byte GPSTimeClass::scanCurSetting = 0;
bool GPSTimeClass::scanNextSetting = false;
uint32_t GPSTimeClass::scanTime = 0;
bool GPSTimeClass::connected = false;



void GPSTimeClass::begin( bool autoUpdate, HardwareSerial * serialPort, uint32_t serialBaud, uint8_t serialConfig ) {
  utcTime.Second = 0;
  utcTime.Minute = 0;
  utcTime.Hour = 0;
  utcTime.Day = 0;
  utcTime.Month = 0;
  utcTime.Year = 0;

  msgLen = 0;
  msgBuf[0] = 0;

  GPSTimeClass::serialConfig = serialConfig;
  GPSTimeClass::serialBaud = serialBaud;
  GPSTimeClass::autoUpdate = autoUpdate;
  gpsSerial = serialPort;

  status = IDLE;

  scanCurSetting = 7;
  scanNextSetting = true;

  if(serialBaud != 0 && autoUpdate)
    gpsSerial->begin(serialBaud, serialConfig);
}

bool GPSTimeClass::isMessageValid( const int8_t * msg ){
  const int8_t * cur = msg;

  if(*cur != '$')
    return false;
  ++cur;

  uint8_t checksum = 0;
  while(*cur != '*'){
    checksum ^= *cur;
    ++cur;
  }

  return *cur && *(cur+1) && *(cur+2) && *(cur+3) == '\0' && (strtol((char *)cur+1,NULL,16) == checksum);
}

// Extracts nth argument from NMEA message, null byte terminated
void GPSTimeClass::getMessageArg( int8_t * dst, const int8_t * msg, uint8_t arg ){
  char * arg_start = strstr((const char *)msg,",")+1;
  char * arg_end;
  
  if(arg_start == NULL){
    *dst = 0;
    return;
  }
  
  while(arg>0){
    --arg;
    arg_start = strstr(arg_start,",")+1;
    
    if(arg_start == NULL){
      *dst = 0;
      return;
    }
  }
  
  arg_end = strstr(arg_start,",");
  if(arg_end == NULL)
    arg_end = strstr(arg_start,"*");

  strncpy((char *)dst, arg_start, arg_end - arg_start);
  dst[arg_end-arg_start] = 0;
}

uint8_t GPSTimeClass::getDayOfWeek( uint32_t day, uint32_t month, uint32_t year, uint32_t cent ){
  // Zeller's congruence
  uint32_t q = day;
  uint32_t m = month;
  uint32_t k = year;
  uint32_t j = cent;

  if(m <= 2){
    m += 12;
    --k;
    if(k==-1){
      k=99;
      --j;
    }
  }

  return (q + 13*(m+1)/5 + k + k/4 + j/4 + 5*j - 1) % 7 + 1;
}

void GPSTimeClass::processMessage( const int8_t * msg, uint32_t msgTime ){
  if(*(msg+3)=='R' && *(msg+4)=='M' && *(msg+5)=='C'){ // We're mainly interested in time and date, which RMC contains
    connected = true;

    if(!isMessageValid( msg )){ // Making sure our message is not corrupt
      Serial.println("GPSTime: Corrupt RMC message!");
      return;
    }
    // Check message status
    int8_t rmc_status[2];
    getMessageArg(rmc_status, msg, 1);
    if(*rmc_status != 'A') {
      //Serial.println("GPSTime: Received void RMC message!"); // Means no signal
      return;
    }
      
    Serial.print("GPSTime: Updating GPS Time... ");
    Serial.println((char *)msg);
    // Get Time
    int8_t str_time[16] = {0};
    getMessageArg(str_time, msg, 0);
    if(*str_time == 0)
      return;

    int8_t * str_time_decimal = str_time;
    while(*str_time_decimal != 0 && *str_time_decimal != '.')
      ++str_time_decimal;
      
    if(*str_time_decimal == '.'){
      *str_time_decimal = 0;
      ++str_time_decimal;
    }

    for(size_t i = 0; i<3; ++i){
      if(str_time_decimal[i] == 0){
        str_time_decimal[i] = '0';
      }
    }
    str_time_decimal[3] = 0;
    
    // Get Date
    int8_t date_time[16];
    getMessageArg(date_time, msg, 8);
    if(*date_time == 0)
      return;
    
    int32_t time = atol((char *)str_time);
    int32_t date = atol((char *)date_time);

    int32_t mil = atoi((char *)str_time_decimal);
    
    
    utcTime.Second = (time%100);
    utcTime.Minute = ((time / 100)%100);
    utcTime.Hour = (time / 10000);
    utcTime.Day = (date / 10000);
    utcTime.Month = ((date / 100)%100) ;
    utcTime.Year = (date % 100)+30;
    utcTime.Wday = getDayOfWeek( utcTime.Day, utcTime.Month, utcTime.Year - 30, 20 );
    
    timeUpdated = msgTime-mil;


    status = FINISHED;

    if(!autoUpdate){
      gpsSerial->end();
    }
  }
}



// Updates the library, processes serial data, etc.
void GPSTimeClass::update() { // Scan portion --------------------------------------------
  if(serialBaud == 0){
    static const uint32_t serialBaudRates[] = {4800,38400,9600,19200};
    static const uint8_t serialConfigs[] = {SERIAL_8N1, SERIAL_8O1};

    if(scanNextSetting){
      
      // Serial.println("GPSTime: Scanning...");
      scanNextSetting = false;
      scanTime = millis();

      scanCurSetting = (scanCurSetting+1)%8;

      uint8_t curBaudRate =      scanCurSetting & 0b00000011;
      uint8_t curSerialConfig = (scanCurSetting & 0b00000100) >> 2;

      gpsSerial->end();
      gpsSerial->begin(serialBaudRates[curBaudRate], serialConfigs[curSerialConfig]);


      Serial.print("GPSTime: Scanning... ");
      Serial.print("baud("); Serial.print(serialBaudRates[curBaudRate],DEC); Serial.print(") ");
      Serial.print("config("); Serial.print(serialConfigs[curSerialConfig],DEC); Serial.print(")\r\n");

      msgBuf[0] = 0;
      msgLen = 0;
    }

    while(gpsSerial->available()) {
      int8_t c = gpsSerial->read();
      if(c == '$'){
        scanTime = millis();
        msgBuf[0] = c;
        msgLen = 1;
      }else if(msgLen == 0){ // Stop from processing garbage
      }else if (c == '\r' || c == '\n'){
        msgBuf[msgLen] = 0;

        if(isMessageValid( msgBuf )){
          uint8_t curBaudRate =      scanCurSetting & 0b00000111;
          uint8_t curSerialConfig = (scanCurSetting & 0b00001000) >> 3;

          Serial.println((char *)msgBuf);
          Serial.print("GPSTime: Found GPS device!");
          Serial.print(" baud("); Serial.print(serialBaudRates[curBaudRate],DEC); Serial.print(") ");
          Serial.print("config("); Serial.print(serialConfigs[curSerialConfig],DEC); Serial.print(")\r\n");
          Serial.println();

          
          msgBuf[0] = 0;
          msgLen = 0;

          serialBaud = serialBaudRates[curBaudRate];
          serialConfig = serialConfigs[curSerialConfig];
        }
      }else if(msgLen < 255){
        msgBuf[msgLen++] = c;
      }else{
        msgLen = 0;
        scanNextSetting = true;
      }
    }
    if(millis()-scanTime > 1000)
      scanNextSetting = true;

  } else { // Processing portion --------------------------------------------
    while(gpsSerial->available()) {
      int c = gpsSerial->read();
      
      if(c == '$') {
        msgBuf[0] = c;
        msgLen = 1;
        msgTime = millis();
      } else if (msgLen == 0){ // Stop from processing garbage
      } else if (c == '\r' || c == '\n'){
        msgBuf[msgLen] = 0;
        processMessage( msgBuf, msgTime );
        msgBuf[0] = 0;
        msgLen = 0;
      } else if (msgLen < 127){
        msgBuf[msgLen++] = c;
      } else {
        msgLen = 0;
      }
    }
  }
}

// Tells the library to retrieve time from next RMC message
void GPSTimeClass::request(){
  if(status != WAITING){
    if(!autoUpdate)
      gpsSerial->begin(serialBaud, serialConfig);
    status = WAITING;
  }
}

GPSTimeStatus GPSTimeClass::getStatus(){
  return status;
}

void GPSTimeClass::reset(){
  status = IDLE;
}

uint32_t GPSTimeClass::millisSinceUpdate(){
  return millis() - timeUpdated;
}

const tmElements_t* GPSTimeClass::getUtcTime(){
  return &utcTime;
}

void GPSTimeClass::getUtcTime( tmElements_t* tm ){
  tm->Second = utcTime.Second;
  tm->Minute = utcTime.Minute;
  tm->Hour = utcTime.Hour;
  tm->Day = utcTime.Day;
  tm->Month = utcTime.Month;
  tm->Year = utcTime.Year;
  tm->Wday = utcTime.Wday;
}

bool GPSTimeClass::isConnected(){
  return connected;
}
