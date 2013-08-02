#include "inputinterface.h"
#include <allegro5/allegro.h>
#include "systeminterface.h"
#include "window.h"
static int sCBKeyMap[ALLEGRO_KEY_MAX] = {0};
static char sKeyStates[222] = {0};
static bool sSafeExit = true;

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
	if (!(al_install_keyboard() && al_install_mouse())) return false;
	al_register_event_source(Window::instance()->eventQueue(), al_get_keyboard_event_source());
	al_register_event_source(Window::instance()->eventQueue(), al_get_mouse_event_source());
	return true;
}


int input::scancodeToAllegroKey(int scan) {
	if (scan > 0 && scan < 222) return sCBKeyMap[scan];
	return 0;
}


void input::handleKeyEvent(const ALLEGRO_EVENT &e) {
	int keyCode = e.keyboard.keycode;
	if (sSafeExit && e.type == ALLEGRO_EVENT_KEY_DOWN && keyCode == ALLEGRO_KEY_ESCAPE) {
		sys::closeProgram();
	}

	if ((sKeyStates[keyCode] & 1) != (e.type == ALLEGRO_EVENT_KEY_DOWN)) { // state changed
		sKeyStates[keyCode] ^= 1; //swap key state
		sKeyStates[keyCode] |= 2; //just modified bit
	}
}

void input::eventLoopEnd() {
}


void input::eventLoopBegin() {
	for (int i = 1; i < 222; i++) {
		sKeyStates[i] &= 1;
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
