#include "lstring.h"
#include <allegro5/allegro5.h>
#include <allegro5/allegro_primitives.h>
#include "error.h"
#include "window.h"
#include "systeminterface.h"
#include "gfxinterface.h"
#include "textinterface.h"
#include "inputinterface.h"

extern "C" void CB_main();
extern "C" void CB_initialize();


int main(int argc, char **argv) {

	info(U"Initializing types and strings");
	CB_initialize();

	info(U"Initializing allegro");
	if (!al_init()) {
		error(U"Allegro initialization failed");
		return 1;
	}

	info(U"Creating a window");
	Window *window = new Window();
	if (!window->create()) return 1;


	info(U"Initializing interfaces");
	gfx::initGfx();
	text::initText();
	input::initInput();

	//Begin execution
	info(U"Beginning execution");
	CB_main();

	sys::closeProgram();

	return 0;
}
