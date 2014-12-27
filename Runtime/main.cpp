#include "lstring.h"
#include <allegro5/allegro5.h>
#include <allegro5/allegro_primitives.h>
#include "error.h"
#include "window.h"
#include "systeminterface.h"
#include "gfxinterface.h"
#include "textinterface.h"
#include "inputinterface.h"
#include "transforminterface.h"
#include <cstdio>
#include <iostream>

extern "C" void CB_main();
extern "C" void CB_initialize();

#ifdef _WIN32
#include <windows.h>
int CALLBACK WinMain(
  HINSTANCE,
  HINSTANCE,
  LPSTR,
  int
) {
#else
int main(int, char **) {
#endif
	std::cout << "asd\n";
	sys::init();
	std::cout << "asd\n";
	info(U"Initializing types and strings");
	std::cout << "asd\n";
	CB_initialize();

	info(U"Initializing allegro");
	if (!al_init()) {
		error(U"Allegro initialization failed");
		return 1;
	}

	info(U"Creating a window");
	Window *window = new Window();
	if (!window->create()) return 1;
	RenderTarget::setFallbackRenderTarget(window);


	info(U"Initializing interfaces");
	gfx::initGfx();
	text::initText();
	input::initInput();
	transforms::initTransforms();

	//Begin execution
	info(U"Beginning execution");
	CB_main();

	sys::closeProgram();

	return 0;
}
