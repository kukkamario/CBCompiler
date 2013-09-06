#include "inputinterface.h"

int CBF_keyDown(int scancode) {
	return input::scancodeKeyState(scancode) & input::Down;
}

int CBF_keyUp(int scancode) {
	return input::scancodeKeyState(scancode) == input::Released;
}


int CBF_keyHit(int scancode) {
	return input::scancodeKeyState(scancode) == input::Pressed;
}

int CBF_escapeKey() {
	return input::keyState(ALLEGRO_KEY_ESCAPE) & input::Down;
}
