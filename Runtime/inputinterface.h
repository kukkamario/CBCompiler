#ifndef INPUTINTERFACE_H
#define INPUTINTERFACE_H
#include "common.h"
namespace input {
enum KeyState {
	Up = 0,			//00
	Down = 1,		//01
	Released = 2,	//10
	Pressed = 3		//11
};

bool initInput();
int scancodeToAllegroKey(int scan);
void eventLoopBegin();
void eventLoopEnd();
void handleKeyEvent(const ALLEGRO_EVENT &e);
KeyState keyState(int alKey);
KeyState scancodeKeyState(int scan);
void setSafeExit(bool t);

}
#endif // INPUTINTERFACE_H
