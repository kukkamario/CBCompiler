//#include "audiointerface.h"

//static ALLEGRO_MIXER* sCurrentMixer = NULL;
//static bool sAudioInstalled = false;
//static bool sCodecsInstalled = false;


//bool audiointerface::initAudio() {
//	if(!(sAudioInstalled = al_install_audio()))
//		return false;
//	if(!(sCodecsInstalled = al_init_acodec_addon()))
//		return false;
//	if(!al_reserve_samples(512))
//		return false;

//	resetToDefaultMixer();
//}

//bool audiointerface::isAudioInstalled() {
//	return sAudioInstalled;
//}

//bool audiointerface::isCodecsInstalled() {
//	return sCodecsInstalled;
//}

//void audiointerface::setCurrentMixer(ALLEGRO_MIXER *mixer) {
//	sCurrentMixer = mixer;
//}

//void audiointerface::resetToDefaultMixer() {
//	sCurrentMixer = al_get_default_mixer();
//}
