#include "RTTTL.h"
#include "Arduino.h"

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
  playing = false;
  buzzer.begin( buzzerPin );
}

void RTTTL::play( ) {
  playing = true;
  nextNoteStr = commandsStart;
  notePlayed = millis();
  noteDuration = 0;
}

void RTTTL::update( ) {
  if(!playing)
    return;
  if(millis() - notePlayed < noteDuration )
    return;
  
  if(nextNoteStr == 0) {
    playing = false;
    return;
  }

  nextNoteStr = parseCommand(nextNoteStr);
}

const char * RTTTL::parseControl( const char * strControls ) {
  const char * cursor = strControls;

  char controlName;
  int controlValue;

  while(isspace(*cursor)) ++cursor;

  controlName = *(cursor++);

  while(isspace(*cursor)) ++cursor;
  
  if(*(cursor++) != '=')
    return 0;

  while(isspace(*cursor)) ++cursor;

  switch(controlName) {
    case 'd':
      defaultDuration = atoi(cursor);
    break;
    case 'o':
      defaultOctave = atoi(cursor);
    break;
    case 'b':
      beatsPerMinute = atoi(cursor);
    break;
  }

  while(*cursor != ',' && *cursor != ':' && *cursor != '\0') ++cursor;

  return cursor;
}


void RTTTL::parseControls( const char * strControls ) {
  const char * cursor;
  cursor = parseControl( strControls );
  if(*cursor == ',') 
    parseControls(cursor+1);
}

const char * RTTTL::parseNote( const char * strNote ) {
  int noteOctave = defaultOctave;
  
  const char * cursor = strNote;

  while(isspace(*cursor)) ++cursor;

  // Get note duration
  noteDuration = ((60L * 1000L / beatsPerMinute) * 4L)/(isdigit(*cursor) ? atol(cursor) : defaultDuration);
  
  // Skip the numbers and spaces
  while(isdigit(*cursor) || isspace(*cursor)) ++cursor; 
  
  // Check for special duration identifier
  if(*cursor == '.') {
    noteDuration += noteDuration/2;
    ++cursor;
  }
  
  while(isspace(*cursor)) ++cursor;

  // Next should go a letter note, followed by sharp symbol if present.
  //                                  A  B  C  D  E  F  G  H
  const int8_t noteLetterToKey[8] = { 1, 3,-8,-6,-4,-3,-1, 3};

  unsigned char noteLetter = toupper( *(cursor++) );

  int16_t noteId = (noteLetter >= 'A' && noteLetter <= 'H') ? noteLetterToKey[noteLetter-'A'] : 0;

  while(isspace(*cursor)) ++cursor;

  if(*cursor == '#'){
    noteId += 1;
    ++cursor;

    while(isspace(*cursor)) ++cursor;
  }

  // Again, check for special duration identifier
  if(*cursor == '.') {
    noteDuration += noteDuration/2;
    ++cursor;

    while(isspace(*cursor)) ++cursor;
  }
  
  // Get note scale
  if(isdigit(*cursor)) {
    noteId += 12*atoi(cursor);
    while(isdigit(*cursor)) ++cursor;
  }else{
    noteId += 12*defaultOctave;
  }
  
  // Skip scale numbers and spaces
  while(isspace(*cursor)) ++cursor;
  
  // Last check for special duration identifier
  if(*cursor == '.') {
    noteDuration += noteDuration/2;
    ++cursor;

    while(isspace(*cursor)) ++cursor;
  }

  notePlayed = millis();

  if(noteLetter != 'P')
    buzzer.play( powf(1.05946309, noteId-49)*440 + 0.5,noteDuration );
  else
    buzzer.stop();
  // Serial.write(strNote);
  // Serial.write("\r\n");
  // Serial.print((uint16_t)(powf(1.05946309, noteId-49)*440 + 0.5), DEC);
  // Serial.print(' ');
  // Serial.write(noteLetter);
  // Serial.print(' ');
  // Serial.print(noteId, DEC);
  // Serial.write("\r\n");
  // Skip to delimiter or null byte
  while(*cursor != ',' && *cursor != '\0')
    ++cursor;
  
  // If string contains more notes, parse them next
  return (*cursor) ? cursor+1 : 0;  
}

const char * RTTTL::parseCommand(const char * strCommands) {

  const char * cursor = strCommands;

  while(*cursor != '=' && *cursor != ',' && *cursor != '\0')
    ++cursor;

  if(*cursor == '=') {
    cursor = parseControl( strCommands );

    return (*cursor == ',') ? parseCommand( cursor+1 ) : 0;
  } else {
    return parseNote( strCommands );
  }
}

void RTTTL::load(char * songStr) {
  defaultDuration = 4;
  defaultOctave = 6;
  beatsPerMinute = 63;

  // Strip spaces
  stripSpaces(songStr);

  const char * controlsStart = strchr(songStr, ':')+1;
  commandsStart = strchr(controlsStart, ':')+1;
  
  // Parse the controls and then notes
  parseControls(controlsStart);
}