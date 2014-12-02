#include "inputinterface.h"
#include <allegro5/allegro.h>
#include "systeminterface.h"
#include "window.h"

static int sCBKeyMap[ALLEGRO_KEY_MAX] = {0};
static int sAllegroKeyMap[ALLEGRO_KEY_MAX] = {0};
static char sKeyStates[222] = {0};
static char sMouseStates[5] = {0};
static bool sSafeExit = true;
static int sMouseX = 0;
static int sMouseY = 0;
static int sMouseZ = 0;
static int sMouseMoveX = 0;
static int sMouseMoveY = 0;
static int sMouseMoveZ = 0;

static int sLastScanCode = 0;
static int sLastChar = 0;
static float sMousePressure = 0.0f;

bool input::initInput() {
	sCBKeyMap[0] = 0;
	sCBKeyMap[1] = ALLEGRO_KEY_ESCAPE;
	sCBKeyMap[2] = ALLEGRO_KEY_1;
	sCBKeyMap[3] = ALLEGRO_KEY_2;
	sCBKeyMap[4] = ALLEGRO_KEY_3;
	sCBKeyMap[5] = ALLEGRO_KEY_4;
	sCBKeyMap[6] = ALLEGRO_KEY_5;
	sCBKeyMap[7] = ALLEGRO_KEY_6;
	sCBKeyMap[8] = ALLEGRO_KEY_7;
	sCBKeyMap[9] = ALLEGRO_KEY_8;
	sCBKeyMap[10] = ALLEGRO_KEY_9;
	sCBKeyMap[11] = ALLEGRO_KEY_0;
	sCBKeyMap[12] = ALLEGRO_KEY_EQUALS;
	sCBKeyMap[13] = ALLEGRO_KEY_OPENBRACE;
	sCBKeyMap[14] = ALLEGRO_KEY_BACKSPACE;
	sCBKeyMap[15] = ALLEGRO_KEY_TAB;
	sCBKeyMap[16] = ALLEGRO_KEY_Q;
	sCBKeyMap[17] = ALLEGRO_KEY_W;
	sCBKeyMap[18] = ALLEGRO_KEY_E;
	sCBKeyMap[19] = ALLEGRO_KEY_R;
	sCBKeyMap[20] = ALLEGRO_KEY_T;
	sCBKeyMap[21] = ALLEGRO_KEY_Y;
	sCBKeyMap[22] = ALLEGRO_KEY_U;
	sCBKeyMap[23] = ALLEGRO_KEY_I;
	sCBKeyMap[24] = ALLEGRO_KEY_O;
	sCBKeyMap[25] = ALLEGRO_KEY_P;
	sCBKeyMap[26] = ALLEGRO_KEY_CLOSEBRACE;
	sCBKeyMap[27] = ALLEGRO_KEY_SEMICOLON;
	sCBKeyMap[28] = ALLEGRO_KEY_ENTER;
	sCBKeyMap[29] = ALLEGRO_KEY_LCTRL;
	sCBKeyMap[30] = ALLEGRO_KEY_A;
	sCBKeyMap[31] = ALLEGRO_KEY_S;
	sCBKeyMap[32] = ALLEGRO_KEY_D;
	sCBKeyMap[33] = ALLEGRO_KEY_F;
	sCBKeyMap[34] = ALLEGRO_KEY_G;
	sCBKeyMap[35] = ALLEGRO_KEY_H;
	sCBKeyMap[36] = ALLEGRO_KEY_J;
	sCBKeyMap[37] = ALLEGRO_KEY_K;
	sCBKeyMap[38] = ALLEGRO_KEY_L;
	sCBKeyMap[39] = ALLEGRO_KEY_TILDE;
	sCBKeyMap[40] = ALLEGRO_KEY_QUOTE;
	sCBKeyMap[41] = ALLEGRO_KEY_BACKSLASH;
	sCBKeyMap[42] = ALLEGRO_KEY_LSHIFT;
	sCBKeyMap[43] = ALLEGRO_KEY_SLASH;
	sCBKeyMap[44] = ALLEGRO_KEY_Z;
	sCBKeyMap[45] = ALLEGRO_KEY_X;
	sCBKeyMap[46] = ALLEGRO_KEY_C;
	sCBKeyMap[47] = ALLEGRO_KEY_V;
	sCBKeyMap[48] = ALLEGRO_KEY_B;
	sCBKeyMap[49] = ALLEGRO_KEY_N;
	sCBKeyMap[50] = ALLEGRO_KEY_M;
	sCBKeyMap[51] = ALLEGRO_KEY_COMMA;
	sCBKeyMap[52] = ALLEGRO_KEY_FULLSTOP;
	sCBKeyMap[53] = ALLEGRO_KEY_MINUS;
	sCBKeyMap[54] = ALLEGRO_KEY_RSHIFT;
	sCBKeyMap[55] = ALLEGRO_KEY_PAD_ASTERISK;
	sCBKeyMap[56] = ALLEGRO_KEY_ALT;
	sCBKeyMap[57] = ALLEGRO_KEY_SPACE;
	sCBKeyMap[58] = ALLEGRO_KEY_CAPSLOCK;
	sCBKeyMap[59] = ALLEGRO_KEY_F1;
	sCBKeyMap[60] = ALLEGRO_KEY_F2;
	sCBKeyMap[61] = ALLEGRO_KEY_F3;
	sCBKeyMap[62] = ALLEGRO_KEY_F4;
	sCBKeyMap[63] = ALLEGRO_KEY_F5;
	sCBKeyMap[64] = ALLEGRO_KEY_F6;
	sCBKeyMap[65] = ALLEGRO_KEY_F7;
	sCBKeyMap[66] = ALLEGRO_KEY_F8;
	sCBKeyMap[67] = ALLEGRO_KEY_F9;
	sCBKeyMap[68] = ALLEGRO_KEY_F10;
	sCBKeyMap[69] = ALLEGRO_KEY_PAUSE;
	sCBKeyMap[70] = ALLEGRO_KEY_SCROLLLOCK;
	sCBKeyMap[71] = ALLEGRO_KEY_PAD_7;
	sCBKeyMap[72] = ALLEGRO_KEY_PAD_8;
	sCBKeyMap[73] = ALLEGRO_KEY_PAD_9;
	sCBKeyMap[74] = ALLEGRO_KEY_PAD_MINUS;
	sCBKeyMap[75] = ALLEGRO_KEY_PAD_4;
	sCBKeyMap[76] = ALLEGRO_KEY_PAD_5;
	sCBKeyMap[77] = ALLEGRO_KEY_PAD_6;
	sCBKeyMap[78] = ALLEGRO_KEY_PAD_PLUS;
	sCBKeyMap[79] = ALLEGRO_KEY_PAD_1;
	sCBKeyMap[80] = ALLEGRO_KEY_PAD_2;
	sCBKeyMap[81] = ALLEGRO_KEY_PAD_3;
	sCBKeyMap[82] = ALLEGRO_KEY_PAD_0;
	sCBKeyMap[83] = ALLEGRO_KEY_PAD_DELETE;
	sCBKeyMap[86] = ALLEGRO_KEY_BACKSLASH2;
	sCBKeyMap[87] = ALLEGRO_KEY_F11;
	sCBKeyMap[88] = ALLEGRO_KEY_F12;
	sCBKeyMap[156] = ALLEGRO_KEY_PAD_ENTER;
	sCBKeyMap[157] = ALLEGRO_KEY_RCTRL;
	sCBKeyMap[181] = ALLEGRO_KEY_PAD_SLASH;
	sCBKeyMap[183] = ALLEGRO_KEY_PRINTSCREEN;
	sCBKeyMap[184] = ALLEGRO_KEY_ALTGR;
	sCBKeyMap[197] = ALLEGRO_KEY_NUMLOCK;
	sCBKeyMap[199] = ALLEGRO_KEY_HOME;
	sCBKeyMap[200] = ALLEGRO_KEY_UP;
	sCBKeyMap[201] = ALLEGRO_KEY_PGUP;
	sCBKeyMap[203] = ALLEGRO_KEY_LEFT;
	sCBKeyMap[205] = ALLEGRO_KEY_RIGHT;
	sCBKeyMap[207] = ALLEGRO_KEY_END;
	sCBKeyMap[208] = ALLEGRO_KEY_DOWN;
	sCBKeyMap[209] = ALLEGRO_KEY_PGDN;
	sCBKeyMap[210] = ALLEGRO_KEY_INSERT;
	sCBKeyMap[211] = ALLEGRO_KEY_DELETE;
	sCBKeyMap[219] = ALLEGRO_KEY_LWIN;
	sCBKeyMap[220] = ALLEGRO_KEY_RWIN;
	sCBKeyMap[221] = ALLEGRO_KEY_MENU;

	sAllegroKeyMap[0] = 0;
	sAllegroKeyMap[ALLEGRO_KEY_ESCAPE] = 1;
	sAllegroKeyMap[ALLEGRO_KEY_1] = 2;
	sAllegroKeyMap[ALLEGRO_KEY_2] = 3;
	sAllegroKeyMap[ALLEGRO_KEY_3] = 4;
	sAllegroKeyMap[ALLEGRO_KEY_4] = 5;
	sAllegroKeyMap[ALLEGRO_KEY_5] = 6;
	sAllegroKeyMap[ALLEGRO_KEY_6] = 7;
	sAllegroKeyMap[ALLEGRO_KEY_7] = 8;
	sAllegroKeyMap[ALLEGRO_KEY_8] = 9;
	sAllegroKeyMap[ALLEGRO_KEY_9] = 10;
	sAllegroKeyMap[ALLEGRO_KEY_0] = 11;
	sAllegroKeyMap[ALLEGRO_KEY_EQUALS] = 12;
	sAllegroKeyMap[ALLEGRO_KEY_OPENBRACE] = 13;
	sAllegroKeyMap[ALLEGRO_KEY_BACKSPACE] = 14;
	sAllegroKeyMap[ALLEGRO_KEY_TAB] = 15;
	sAllegroKeyMap[ALLEGRO_KEY_Q] = 16;
	sAllegroKeyMap[ALLEGRO_KEY_W] = 17;
	sAllegroKeyMap[ALLEGRO_KEY_E] = 18;
	sAllegroKeyMap[ALLEGRO_KEY_R] = 19;
	sAllegroKeyMap[ALLEGRO_KEY_T] = 20;
	sAllegroKeyMap[ALLEGRO_KEY_Y] = 21;
	sAllegroKeyMap[ALLEGRO_KEY_U] = 22;
	sAllegroKeyMap[ALLEGRO_KEY_I] = 23;
	sAllegroKeyMap[ALLEGRO_KEY_O] = 24;
	sAllegroKeyMap[ALLEGRO_KEY_P] = 25;
	sAllegroKeyMap[ALLEGRO_KEY_CLOSEBRACE] = 26;
	sAllegroKeyMap[ALLEGRO_KEY_SEMICOLON] = 27;
	sAllegroKeyMap[ALLEGRO_KEY_ENTER] = 28;
	sAllegroKeyMap[ALLEGRO_KEY_LCTRL] = 29;
	sAllegroKeyMap[ALLEGRO_KEY_A] = 30;
	sAllegroKeyMap[ALLEGRO_KEY_S] = 31;
	sAllegroKeyMap[ALLEGRO_KEY_D] = 32;
	sAllegroKeyMap[ALLEGRO_KEY_F] = 33;
	sAllegroKeyMap[ALLEGRO_KEY_G] = 34;
	sAllegroKeyMap[ALLEGRO_KEY_H] = 35;
	sAllegroKeyMap[ALLEGRO_KEY_J] = 36;
	sAllegroKeyMap[ALLEGRO_KEY_K] = 37;
	sAllegroKeyMap[ALLEGRO_KEY_L] = 38;
	sAllegroKeyMap[ALLEGRO_KEY_TILDE] = 39;
	sAllegroKeyMap[ALLEGRO_KEY_QUOTE] = 40;
	sAllegroKeyMap[ALLEGRO_KEY_BACKSLASH] = 41;
	sAllegroKeyMap[ALLEGRO_KEY_LSHIFT] = 42;
	sAllegroKeyMap[ALLEGRO_KEY_SLASH] = 43;
	sAllegroKeyMap[ALLEGRO_KEY_Z] = 44;
	sAllegroKeyMap[ALLEGRO_KEY_X] = 45;
	sAllegroKeyMap[ALLEGRO_KEY_C] = 46;
	sAllegroKeyMap[ALLEGRO_KEY_V] = 47;
	sAllegroKeyMap[ALLEGRO_KEY_B] = 48;
	sAllegroKeyMap[ALLEGRO_KEY_N] = 49;
	sAllegroKeyMap[ALLEGRO_KEY_M] = 50;
	sAllegroKeyMap[ALLEGRO_KEY_COMMA] = 51;
	sAllegroKeyMap[ALLEGRO_KEY_FULLSTOP] = 52;
	sAllegroKeyMap[ALLEGRO_KEY_MINUS] = 53;
	sAllegroKeyMap[ALLEGRO_KEY_RSHIFT] = 54;
	sAllegroKeyMap[ALLEGRO_KEY_PAD_ASTERISK] = 55;
	sAllegroKeyMap[ALLEGRO_KEY_ALT] = 56;
	sAllegroKeyMap[ALLEGRO_KEY_SPACE] = 57;
	sAllegroKeyMap[ALLEGRO_KEY_CAPSLOCK] = 58;
	sAllegroKeyMap[ALLEGRO_KEY_F1] = 59;
	sAllegroKeyMap[ALLEGRO_KEY_F2] = 60;
	sAllegroKeyMap[ALLEGRO_KEY_F3] = 61;
	sAllegroKeyMap[ALLEGRO_KEY_F4] = 62;
	sAllegroKeyMap[ALLEGRO_KEY_F5] = 63;
	sAllegroKeyMap[ALLEGRO_KEY_F6] = 64;
	sAllegroKeyMap[ALLEGRO_KEY_F7] = 65;
	sAllegroKeyMap[ALLEGRO_KEY_F8] = 66;
	sAllegroKeyMap[ALLEGRO_KEY_F9] = 67;
	sAllegroKeyMap[ALLEGRO_KEY_F10] = 68;
	sAllegroKeyMap[ALLEGRO_KEY_PAUSE] = 69;
	sAllegroKeyMap[ALLEGRO_KEY_SCROLLLOCK] = 70;
	sAllegroKeyMap[ALLEGRO_KEY_PAD_7] = 71;
	sAllegroKeyMap[ALLEGRO_KEY_PAD_8] = 72;
	sAllegroKeyMap[ALLEGRO_KEY_PAD_9] = 73;
	sAllegroKeyMap[ALLEGRO_KEY_PAD_MINUS] = 74;
	sAllegroKeyMap[ALLEGRO_KEY_PAD_4] = 75;
	sAllegroKeyMap[ALLEGRO_KEY_PAD_5] = 76;
	sAllegroKeyMap[ALLEGRO_KEY_PAD_6] = 77;
	sAllegroKeyMap[ALLEGRO_KEY_PAD_PLUS] = 78;
	sAllegroKeyMap[ALLEGRO_KEY_PAD_1] = 79;
	sAllegroKeyMap[ALLEGRO_KEY_PAD_2] = 80;
	sAllegroKeyMap[ALLEGRO_KEY_PAD_3] = 81;
	sAllegroKeyMap[ALLEGRO_KEY_PAD_0] = 82;
	sAllegroKeyMap[ALLEGRO_KEY_PAD_DELETE] = 83;
	sAllegroKeyMap[ALLEGRO_KEY_BACKSLASH2] = 86;
	sAllegroKeyMap[ALLEGRO_KEY_F11] = 87;
	sAllegroKeyMap[ALLEGRO_KEY_F12] = 88;
	sAllegroKeyMap[ALLEGRO_KEY_PAD_ENTER] = 156;
	sAllegroKeyMap[ALLEGRO_KEY_RCTRL] = 157;
	sAllegroKeyMap[ALLEGRO_KEY_PAD_SLASH] = 181;
	sAllegroKeyMap[ALLEGRO_KEY_PRINTSCREEN] = 183;
	sAllegroKeyMap[ALLEGRO_KEY_ALTGR] = 184;
	sAllegroKeyMap[ALLEGRO_KEY_NUMLOCK] = 197;
	sAllegroKeyMap[ALLEGRO_KEY_HOME] = 199;
	sAllegroKeyMap[ALLEGRO_KEY_UP] = 200;
	sAllegroKeyMap[ALLEGRO_KEY_PGUP] = 201;
	sAllegroKeyMap[ALLEGRO_KEY_LEFT] = 203;
	sAllegroKeyMap[ALLEGRO_KEY_RIGHT] = 205;
	sAllegroKeyMap[ALLEGRO_KEY_END] = 207;
	sAllegroKeyMap[ALLEGRO_KEY_DOWN] = 208;
	sAllegroKeyMap[ALLEGRO_KEY_PGDN] = 209;
	sAllegroKeyMap[ALLEGRO_KEY_INSERT] = 210;
	sAllegroKeyMap[ALLEGRO_KEY_DELETE] = 211;
	sAllegroKeyMap[ALLEGRO_KEY_LWIN] = 219;
	sAllegroKeyMap[ALLEGRO_KEY_RWIN] = 220;
	sAllegroKeyMap[ALLEGRO_KEY_MENU] = 221;

	if (!(al_install_keyboard() && al_install_mouse())) return false;
	al_register_event_source(Window::instance()->eventQueue(), al_get_keyboard_event_source());
	al_register_event_source(Window::instance()->eventQueue(), al_get_mouse_event_source());
	return true;
}


int input::scancodeToAllegroKey(int scan) {
	if (scan > 0 && scan < 222) return sCBKeyMap[scan];
	return 0;
}


int input::allegroKeyToScancode(int alkey) {
	return sAllegroKeyMap[alkey];
}


void input::handleKeyEvent(const ALLEGRO_EVENT &e) {
	int keyCode = e.keyboard.keycode;
	sLastScanCode = allegroKeyToScancode(keyCode);
	if (sSafeExit && e.type == ALLEGRO_EVENT_KEY_DOWN && keyCode == ALLEGRO_KEY_ESCAPE) {
		sys::closeProgram();
	}

	if ((sKeyStates[keyCode] & 1) != (e.type == ALLEGRO_EVENT_KEY_DOWN)) { // state changed
		sKeyStates[keyCode] ^= 1; //swap key state
		sKeyStates[keyCode] |= 2; //just modified bit
	}
}

void input::handleMouseEvent(const ALLEGRO_EVENT &e) {
	switch(e.type) {
		case ALLEGRO_EVENT_MOUSE_BUTTON_DOWN:
		case ALLEGRO_EVENT_MOUSE_BUTTON_UP:
			if(e.mouse.button < 5) {
				sMouseStates[e.mouse.button] ^= 1;
				sMouseStates[e.mouse.button] |= 2;
			}
		case ALLEGRO_EVENT_MOUSE_AXES:
			sMouseX = e.mouse.x;
			sMouseY = e.mouse.y;
			sMouseZ = e.mouse.z;
			sMouseMoveX = e.mouse.dx;
			sMouseMoveY = e.mouse.dy;
			sMouseMoveZ = e.mouse.dz;
			sMousePressure = e.mouse.pressure;
		break;
	}
}


void input::eventLoopEnd() {
}


void input::eventLoopBegin() {
	for (int i = 1; i < 222; i++) {
		sKeyStates[i] &= 1;
	}
	for(int i = 0; i < 5; i++) {
		sMouseStates[i] &= 1;
	}
}


input::KeyState input::scancodeKeyState(int scan) {
	return (input::KeyState) sKeyStates[scancodeToAllegroKey(scan)];
}


void input::setSafeExit(bool t) {
	sSafeExit = t;
}


input::KeyState input::keyState(int alKey) {
	return (input::KeyState) sKeyStates[alKey];
}



input::KeyState input::mouseState(int mousebutton) {
	return (input::KeyState) sMouseStates[mousebutton];
}


int input::Mousex() {
	return sMouseX;
}

int input::MouseY() {
	return sMouseY;
}

int input::MouseZ() {
	return sMouseZ;
}

int input::MouseMoveX() {
	return sMouseMoveX;
}

int input::MouseMoveY() {
	return sMouseMoveY;
}

int input::MouseMoveZ() {
	return sMouseMoveZ;
}


float input::MousePressure() {
	return sMousePressure;
}


void input::setMousePosition(int x, int y) {
	al_set_mouse_xy(Window::instance()->display(), x, y);
}

void input::showMouse(int cursor) {
	al_set_system_mouse_cursor(Window::instance()->display(),
				(cursor >= 0 && cursor < ALLEGRO_NUM_SYSTEM_MOUSE_CURSORS) == true ?
				(ALLEGRO_SYSTEM_MOUSE_CURSOR)cursor :
				ALLEGRO_SYSTEM_MOUSE_CURSOR_DEFAULT
				);
}




int input::getLastKey() {
	return sLastScanCode;
}


int input::getLastChar() {
	return sLastChar;
}


void input::handleCharEvent(const ALLEGRO_EVENT &e) {
	if(e.keyboard.unichar != 0)
		sLastChar = e.keyboard.unichar;
}
