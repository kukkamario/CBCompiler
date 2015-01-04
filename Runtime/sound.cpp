#include "sound.h"
#include "audiointerface.h"
#include "error.h"


Sound::Sound():mSample(NULL) {}

Sound::Sound(const LString &str):Sound() {
	loadSound(str);
}

Sound::Sound(Memblock *mem, int samples, int frequency, int depth, int conf) {
	createSound(mem, samples, frequency, audio::getDepthMode(depth),  audio::getChannelConf(conf));
}

Sound::~Sound() {
	if(mSample != NULL)
		al_destroy_sample(mSample);
}

void Sound::loadSound(const LString& str) {
	if(!(audio::isAudioInstalled() && audio::isCodecsInstalled()))
		error("Audio is not initialized.");
	mSample = al_load_sample(str.toUtf8().c_str());
	mFrequency = al_get_sample_frequency(mSample);

}

void Sound::createSound(Memblock* mem, int samples, int frequency, int depth, int conf) {
	/*To allocate a buffer of the correct size, you can use something like this:
	
	sample_size = al_get_channel_count(chan_conf) * al_get_audio_depth_size(depth);
	bytes = samples * sample_size;
	buffer = al_malloc(bytes);*/
	mSample = al_create_sample((void*)mem, samples, frequency, audio::getDepthMode(depth), audio::getChannelConf(conf), false);
	mFrequency = al_get_sample_frequency(mSample);
}

bool Sound::saveSound(const LString& str) {
	al_save_sample(str.toUtf8().c_str(), mSample);
}

float Sound::getSoundLength() const {
	return  (float)al_get_sample_length(mSample) / (float)mFrequency;
}


int Sound::getFrequency() const {
	return mFrequency;
}

ALLEGRO_SAMPLE *Sound::getSample() {
	return mSample;
}

const ALLEGRO_SAMPLE *Sound::getSample() const {
	return mSample;
}
