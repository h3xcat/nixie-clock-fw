#ifndef RTTTL_H
#define RTTTL_H

#include <Tone.h>

struct RTTLTone {
	unsigned char duration;
	unsigned char note[3];
	unsigned char scale;
	unsigned char special_duration;
};

class RTTTL {
  private:
    Tone buzzer;

  	char name[16];
    int defaultDuration;
  	int defaultOctave;
  	int bpm;


    void parseControls( const char * controls );
    void parseTones( const char * tones );
    void stripSpaces( char * str );

  public:
    void begin( uint8_t buzzerPin );
    void load( const char * ringtone );

    void update();
    void play();
    void stop();
};

#endif