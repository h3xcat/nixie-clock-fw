#ifndef NOKRING_H
#define NOKRING_H



struct NokringNote {
	unsigned char duration;
	unsigned char note[3];
	unsigned char scale;
	unsigned char special_duration;
}

class Nokring {
  private:
  	char name[16];
  	unsigned char duration;
  	unsigned char octave;
  	int bpm;

    bool loop;

  public:
    Nokring( const char * ringtone, bool loop = false );
    void update();
    void play();
    void stop();
};

#endif