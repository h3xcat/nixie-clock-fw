#include "GPSTime.h"
#include "Arduino.h"

GPSTimeClass GPSTime;


bool GPSTimeClass::autoUpdate = false;
tmElements_t GPSTimeClass::utcTime = {};
unsigned long GPSTimeClass::timeUpdated = 0;
GPSTimeStatus GPSTimeClass::status = IDLE;
HardwareSerial * GPSTimeClass::gpsSerial = NULL;
unsigned long GPSTimeClass::serialBaud = 0;
byte GPSTimeClass::serialConfig = 0;
char GPSTimeClass::msgBuf[256] = {};
int GPSTimeClass::msgLen = 0;
unsigned long GPSTimeClass::msgTime = 0;
byte GPSTimeClass::scanCurSetting = 0;
bool GPSTimeClass::scanNextSetting = false;
unsigned long GPSTimeClass::scanTime = 0;



void GPSTimeClass::begin( bool autoUpdate, HardwareSerial * serialPort, unsigned long serialBaud = 0, byte serialConfig = SERIAL_8N1 ) {
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

bool GPSTimeClass::isMessageValid( const char * msg ){
  const char * cur = msg;

  if(*cur != '$')
    return false;
  ++cur;

  unsigned char checksum = 0;
  while(*cur != '*'){
    checksum ^= *cur;
    ++cur;
  }

  return *cur && *(cur+1) && *(cur+2) && *(cur+3) == '\0' && (strtol(cur+1,NULL,16) == checksum);
}

// Extracts nth argument from NMEA message, null byte terminated
void GPSTimeClass::getMessageArg( char * dst, const char * msg, uint8_t arg ){
  char * arg_start = strstr(msg,",")+1;
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

  strncpy(dst, arg_start, arg_end - arg_start);
  dst[arg_end-arg_start] = 0;
}

int GPSTimeClass::getDayOfWeek( int day, int month, int year, int cent ){
  // Zeller's congruence
  int q = day;
  int m = month;
  int k = year;
  int j = cent;

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

void GPSTimeClass::processMessage( const char * msg, unsigned long msgTime ){
  if(*(msg+3)=='R' && *(msg+4)=='M' && *(msg+5)=='C'){ // We're mainly interested in time and date, which RMC contains
    if(!isMessageValid( msg )){ // Making sure our message is not corrupt
      Serial.println("GPSTime: Corrupt RMC message!");
      return;
    }

    // Check message status
    char rmc_status[2];
    getMessageArg(rmc_status, msg, 1 );
    if(*rmc_status != 'A') {
      //Serial.println("GPSTime: Received void RMC message!"); // Means no signal (can be caused by weather and such)
      return;
    }
      
    // Get Time
    char str_time[16];
    getMessageArg(str_time, msg, 0 );
    if(*str_time == 0)
      return;
    
    // Get Date
    char date_time[16];
    getMessageArg(date_time, msg, 8 );
    if(*date_time == 0)
      return;
    
    double timef = atof(str_time);
    long time = timef;
    long date = atol(date_time);

    int mil = (timef-time)*1000;

    utcTime.Second = (time%100);
    utcTime.Minute = ((time / 100)%100);
    utcTime.Hour = (time / 10000);
    utcTime.Day = (date / 10000);
    utcTime.Month = ((date / 100)%100) ;
    utcTime.Year = (date % 100)+30;
    utcTime.Wday = getDayOfWeek( utcTime.Day, utcTime.Month, utcTime.Year - 30, 20 );
    
    timeUpdated = msgTime-mil; // Making more precise

    status = FINISHED;

    if(!autoUpdate){
      gpsSerial->end();
    }
  }
}



// Updates the library, processes serial data, etc.
void GPSTimeClass::update() { // Scan portion --------------------------------------------
  if(serialBaud == 0){
    static const long serialBaudRates[] = {4800,38400,9600,19200};
    static const byte serialConfigs[] = {SERIAL_8N1, SERIAL_8O1};

    if(scanNextSetting){

      scanNextSetting = false;
      scanTime = millis();

      scanCurSetting = (scanCurSetting+1)%8;

      byte curBaudRate =      scanCurSetting & 0b00000011;
      byte curSerialConfig = (scanCurSetting & 0b00000100) >> 2;

      gpsSerial->end();
      gpsSerial->begin(serialBaudRates[curBaudRate], serialConfigs[curSerialConfig]);


      Serial.print("GPSTime: Scanning... ");
      Serial.print("baud("); Serial.print(serialBaudRates[curBaudRate],DEC); Serial.print(") ");
      Serial.print("config("); Serial.print(serialConfigs[curSerialConfig],DEC); Serial.print(")\r\n");

      msgBuf[0] = 0;
      msgLen = 0;
    }

    while(gpsSerial->available()) {
      unsigned char c = gpsSerial->read();
      if(c == '$'){
        scanTime = millis();
        msgBuf[0] = c;
        msgLen = 1;
      }else if(msgLen == 0){ // Stop from processing garbage
      }else if (c == '\r' || c == '\n'){
        msgBuf[msgLen] = 0;

        if(isMessageValid( msgBuf )){
          Serial.println("GPSTime: Found GPS device!");

          byte curBaudRate =      scanCurSetting & 0b00000111;
          byte curSerialConfig = (scanCurSetting & 0b00001000) >> 3;
          msgLen = 0;
          serialBaud = serialBaudRates[curBaudRate];
          serialConfig = serialConfigs[curSerialConfig];
        }
      }else if(msgLen < 200){
        msgBuf[msgLen++] = c;
      }else{
        msgLen = 0;
        scanNextSetting = true;
      }
    }
    if(millis()-scanTime > 500)
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

unsigned long GPSTimeClass::millisSinceUpdate(){
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