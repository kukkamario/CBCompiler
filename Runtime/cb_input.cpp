#include "inputinterface.h"
#include "window.h"

int CBF_keyDown(int scancode) {
	return input::scancodeKeyState(scancode) & input::Down;
}

int CBF_keyUp(int scancode) {
	return input::scancodeKeyState(scancode) == input::Released;
}

int CBF_upKey() {
	return input::keyState(ALLEGRO_KEY_UP) & input::Down;
}

int CBF_downKey() {
	return input::keyState(ALLEGRO_KEY_DOWN) & input::Down;
}

int CBF_leftKey() {
	return input::keyState(ALLEGRO_KEY_LEFT) & input::Down;
}

int CBF_rightKey() {
	return input::keyState(ALLEGRO_KEY_RIGHT) & input::Down;
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

float CBF_mousePressure() {
	return input::MousePressure();
}

int CBF_mouseDown(int button) {
	return input::mouseState(button) & input::Down;
}

int CBF_mouseHit(int button) {
	return input::mouseState(button) == input::Pressed;
}

int CBF_mouseUp(int button) {
	return input::mouseState(button) == input::Released;
}

void CBF_positionMouse(int x, int y) {
	input::setMousePosition(x, y);
}

void CBF_showMouse(int cursor) {
	input::showMouse(cursor);
}

void CBF_setMouseSystemCursor(int cursor) {
	input::setMouseCursor(cursor);
}


int CBF_getChar() {
	return input::getLastChar();
}

int CBF_getKey() {
	return input::getLastKey();
}

