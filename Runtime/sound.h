#ifndef SOUND_H
#define SOUND_H
#include "lstring.h"
#include "memblock.h"
#include <allegro5/allegro_audio.h>

class Sound {

		ALLEGRO_SAMPLE* mSample;
		int mFrequency;
		LString mSourceFile;
		
		
	public:

		
		Sound();
		Sound(const LString& str);
		Sound(Memblock* mem, int samples, int frequency, int depth, int conf);
		~Sound();
		void loadSound(const LString& str);
		void createSound(Memblock* mem, int samples, int frequency, int depth, int conf);
		bool saveSound(const LString& str);

		int getFrequency() const;
		float getSoundLength() const;

		ALLEGRO_SAMPLE* getSample();

		const ALLEGRO_SAMPLE* getSample() const;

};

#endif // SOUND_H
