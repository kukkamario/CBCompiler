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
int allegroKeyToScancode(int alkey);
void eventLoopBegin();
void eventLoopEnd();
void handleKeyEvent(const ALLEGRO_EVENT &e);
void handleMouseEvent(const ALLEGRO_EVENT &e);
KeyState keyState(int alKey);
KeyState mouseState(int mousebutton);
KeyState scancodeKeyState(int scan);

int getLastKey();
int getLastChar();

void setMousePosition(int x, int y);
void showMouse(int cursor);

int Mousex();
int MouseY();
int MouseZ();
int MouseMoveX();
int MouseMoveY();
int MouseMoveZ();
float MousePressure();

void setSafeExit(bool t);

}
#endif // INPUTINTERFACE_H
