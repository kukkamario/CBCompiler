#include "cbstring.h"
#include <allegro5/allegro5.h>
#include <allegro5/allegro_primitives.h>
#include "error.h"
#include "window.h"
#include "system.h"
#include "gfx.h"
#include "text.h"

extern "C" void CBF_CB_main();

int main() {

	info(U"Initializing allegro");
	if (!al_init()) {
		error(U"Allegro initialization failed");
		return 1;
	}
	initGfx();
	initText();

	info(U"Creating a window");
	Window *window = new Window();
	if (!window->create()) return 1;

	info(U"Initilizing gfx");



	//Begin execution
	info(U"Beginning execution");
	CBF_CB_main();

	closeProgram();

	return 0;
}
