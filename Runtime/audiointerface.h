#ifndef AUDIOINTERFACE_H
#define AUDIOINTERFACE_H
#include "LString.h"
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>

class Sound;
class AudioChannel;
namespace audio {

	//static ALLEGRO_AUDIO_DEPTH sDepthMode[3];
	//static ALLEGRO_CHANNEL_CONF sChannelConfigs[7];
	
	
	bool isAudioInstalled();
	bool isCodecsInstalled();
	bool initAudio();

	ALLEGRO_AUDIO_DEPTH getDepthMode(int mode);
	ALLEGRO_CHANNEL_CONF getChannelConf(int conf);

	void setCurrentMixer(ALLEGRO_MIXER* mixer);
	void resetToDefaultMixer();
	ALLEGRO_MIXER *getMixer();
}

#endif // AUDIOINTERFACE_H
