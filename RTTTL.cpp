#include "RTTTL.h"
#include "Arduino.h"

int defaultToneDuration = 4;
int defaultToneScale = 6;
int defaultToneBPM = 63;

#define NOTE_SHARP 0x80

void RTTTL::stripSpaces( char * str) {
  char * c = str;
  char * p = str;

  while(*c != '\0'){
    while(isspace(*p))
      ++p;

    if(*p == '\0')
      *c = 0;
    else
      *c++ = *p++;
  }
}

void RTTTL::begin( uint8_t buzzerPin ) {
  buzzer.begin( buzzerPin );
}
void RTTTL::play( ) {

}

void RTTTL::parseControls( const char * strControls ) {
  if(strncmp(strControls, "d=", 2) == 0) {
    defaultToneDuration = atoi(strControls+2);
  }else if(strncmp(strControls, "o=", 2) == 0) {
    defaultToneScale = atoi(strControls+2);
  }else if(strncmp(strControls, "b=", 2) == 0) {
    defaultToneBPM = atoi(strControls+2);
  }
  
  const char * nextControl = strchr(strControls,',');
  
  if(nextControl != 0) 
    parseControls(nextControl+1);
}

void RTTTL::parseTones( const char * strTones ) {
  const char * cursor = strTones;
  
  int toneDuration = defaultToneDuration;
  int toneScale = defaultToneScale;
  bool toneSpecialDuration = false;
  
  
  // Check if tone duration specified
  if(isdigit(*cursor))
    toneDuration = atoi(cursor);
  
  // Skip the numbers
  while(isdigit(*cursor)) ++cursor; 
  
  // Check for special duration identifier
  if(*cursor == '.') {
    toneSpecialDuration = true;
    ++cursor;
  }
  
  // Next should go a note, followed by sharp symbol if present.
  unsigned char note = *(cursor++);
  if(*cursor == '#'){
    note |= NOTE_SHARP; // We signify sharp by using highest bit
    ++cursor;
  }
  
  switch(note) {
    case 'P':
    break;
    case 'C':
    break;
    case 'C'|NOTE_SHARP:
    break;
    case 'D':
    break;
    case 'D'|NOTE_SHARP:
    break;
    case 'E':
    break;
    case 'F':
    break;
    case 'F'|NOTE_SHARP:
    break;
    case 'G':
    break;
    case 'G'|NOTE_SHARP:
    break;
    case 'A':
    break;
    case 'A'|NOTE_SHARP:
    break;
    case 'B':
    break;
  }
  
  // Again, check for special duration identifier
  if(*cursor == '.') {
    toneSpecialDuration = true;
    ++cursor;
  }
  
  // Get tone scale
  if(isdigit(*cursor)) {
    toneScale = atoi(cursor);
  }
  
  // Skip scale numbers
  while(isdigit(*cursor))
    ++cursor;
  
  // Last check for special duration identifier
  if(*cursor == '.') {
    toneSpecialDuration = true;
    ++cursor;
  }
  
  // Just a sanity check, skip to delimiter or null byte
  while(*cursor != ',' && *cursor != '\0')
    ++cursor;
  
  // If string contains more tones, parse them next
  if(*cursor == ',')
    parseTones(cursor + 1);
}

void RTTTL::load(const char * songStr) {
  char strName[32];
  char strControls[64];
  char strTones[256];
  
  // Seperate the RTTTL into 3 parts: name, controls, and tones
  
  const char * nameEnd = strchr(songStr, ':');
  const char * controlsEnd = strchr(nameEnd+1, ':');
  const char * toneEnd = strchr(controlsEnd+1, '\0');
  
  int nameLen = nameEnd-songStr;
  int controlsLen = controlsEnd-nameEnd-1;
  int tonesLen = toneEnd-controlsEnd-1;
  
  strncpy( strName, songStr, nameLen );
  strncpy( strControls, nameEnd+1, controlsLen );
  strncpy( strTones, controlsEnd+1, tonesLen );
  
  strName[nameLen] = 0;
  strControls[controlsLen] = 0;
  strTones[tonesLen] = 0;
  // Strip spaces
  stripSpaces(strControls);
  stripSpaces(strTones);

  // Parse the controls and then notes
  parseControls(strControls);
  parseTones(strTones);
}
