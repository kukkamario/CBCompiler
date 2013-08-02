#include "inputinterface.h"

CBEXPORT int CBF_keyDownI(int scancode) {
	return input::scancodeKeyState(scancode) & input::Down;
}

CBEXPORT int CBF_keyUpI(int scancode) {
	return input::scancodeKeyState(scancode) == input::Released;
}


CBEXPORT int CBF_keyHitI(int scancode) {
	return input::scancodeKeyState(scancode) == input::Pressed;
}

CBEXPORT int CBF_escapeKey() {
	return input::keyState(ALLEGRO_KEY_ESCAPE) & input::Down;
}
