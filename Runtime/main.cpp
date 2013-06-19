#include "cbstring.h"
#include <allegro5/allegro5.h>
#include <allegro5/allegro_primitives.h>
#include "error.h"
#include "window.h"
#include "system.h"
#include "gfx.h"

extern "C" void CBF_CB_main();

int main() {

	if (!al_init()) {
		error(U"Allegro initialization failed");
		return 1;
	}
	al_init_primitives_addon();

	Window *window = new Window();
	if (!window->create()) return 1;

	initGfx();


	//Begin execution
	CBF_CB_main();
	closeProgram();

	return 0;
}
