#include "inputinterface.h"
#include "window.h"

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


int CBF_waitKey() {
	ALLEGRO_EVENT e;
	while(true) {
		al_get_next_event(Window::instance()->eventQueue(), &e);
		if(e.type == ALLEGRO_EVENT_KEY_DOWN)
			return input::allegroKeyToScancode(e.keyboard.keycode);
	}
}

/*Hack aler!*/
void CBF_WaitKey() {
	ALLEGRO_EVENT e;
	while(true) {
		al_get_next_event(Window::instance()->eventQueue(), &e);
		if(e.type == ALLEGRO_EVENT_KEY_DOWN)
			return;
	}
}
