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
		al_wait_for_event(Window::instance()->eventQueue(), &e);
		input::handleKeyEvent(e);
		if(e.type == ALLEGRO_EVENT_KEY_DOWN && (input::keyState(e.keyboard.keycode) & input::Pressed))
			return input::allegroKeyToScancode(e.keyboard.keycode);
	}
}

/*Hack aler!*/
void CBF_WaitKey() {
	CBF_waitKey();
	return;
}


int CBF_mouseX() {
	return input::Mousex();
}

int CBF_mouseY() {
	return input::MouseY();
}

int CBF_mouseZ() {
	return input::MouseZ();
}


int CBF_mouseMoveX() {
	return input::MouseMoveX();
}

int CBF_mouseMoveY() {
	return input::MouseMoveY();
}

int CBF_mouseMoveZ() {
	return input::MouseMoveZ();
}

float CBF_MousePressure() {
	return input::MousePressure();
}

int CBF_MouseDown(int button) {
	return input::mouseState(button) & input::Down;
}

int CBF_MouseHit(int button) {
	return input::mouseState(button) == input::Pressed;
}

int CBF_MouseUp(int button) {
	return input::mouseState(button) == input::Released;
}

