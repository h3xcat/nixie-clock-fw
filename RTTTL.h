#ifndef RTTTL_H
#define RTTTL_H

#include <Tone.h>


class RTTTL {
  private:
    Tone buzzer;

  	char name[16];
    uint16_t defaultDuration;
  	uint8_t defaultOctave;
  	uint16_t beatsPerMinute;

    uint16_t nextNoteStr;
    uint32_t notePlayed;
    uint32_t noteDuration;

    bool playing;


    const char * commandsStart;

    void stripSpaces( char * str );
    void parseControls( const char * strControls );
    const char * parseControl( const char * strControls );
    const char * parseCommand(const char * strCommands);
    const char * parseNote( const char * strNote );
  public:
    void begin( uint8_t buzzerPin );
    void load( char * ringtone );

    void update();
    void play();
    void stop();
};

#endif