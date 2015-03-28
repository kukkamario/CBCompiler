#include "audiointerface.h"
#include "sound.h"

static ALLEGRO_MIXER* sCurrentMixer = NULL;
static bool sAudioInstalled = false;
static bool sCodecsInstalled = false;

static ALLEGRO_AUDIO_DEPTH  sDepthMode[3] =
						{ALLEGRO_AUDIO_DEPTH_UINT8,
						ALLEGRO_AUDIO_DEPTH_UINT16,
						ALLEGRO_AUDIO_DEPTH_FLOAT32};

static ALLEGRO_CHANNEL_CONF sChannelConfigs[7] =
						{ALLEGRO_CHANNEL_CONF_1,
						 ALLEGRO_CHANNEL_CONF_2,
						 ALLEGRO_CHANNEL_CONF_3,
						 ALLEGRO_CHANNEL_CONF_4,
						 ALLEGRO_CHANNEL_CONF_5_1,
						 ALLEGRO_CHANNEL_CONF_6_1,
						 ALLEGRO_CHANNEL_CONF_7_1};


bool audio::initAudio() {
	if(!(sAudioInstalled = al_install_audio()))
		return false;
	if(!(sCodecsInstalled = al_init_acodec_addon()))
		return false;
	if(!al_reserve_samples(512))
		return false;

	resetToDefaultMixer();
	return true;
}

bool audio::isAudioInstalled() {
	return sAudioInstalled;
}

bool audio::isCodecsInstalled() {
	return sCodecsInstalled;
}

void audio::setCurrentMixer(ALLEGRO_MIXER *mixer) {
	sCurrentMixer = mixer;
}

void audio::resetToDefaultMixer() {
	sCurrentMixer = al_get_default_mixer();
}


ALLEGRO_MIXER *audio::getMixer() {
	return sCurrentMixer;
}



ALLEGRO_CHANNEL_CONF audio::getChannelConf(int conf) {
	return sChannelConfigs[conf];
}


ALLEGRO_AUDIO_DEPTH audio::getDepthMode(int mode)
{
	return sDepthMode[mode];
}
