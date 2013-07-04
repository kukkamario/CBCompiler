#include "gfxinterface.h"
#include "window.h"
#include "error.h"
#include <stdio.h>
#include "common.h"
#include <allegro5/allegro_primitives.h>

using namespace gfx;
CBEXPORT void CBF_drawscreen() {
	Window::instance()->drawscreen();
}

CBEXPORT void CBF_screenIII(int w, int h, int mode) {
	Window::WindowMode windowMode = Window::Windowed;
	switch(mode) {
		case 0:
			windowMode = Window::FullScreen; break;
		case 1:
			windowMode = Window::Windowed; break;
		case 2:
			windowMode = Window::Resizable; break;
		default:
			error(U"Invalid window mode");
	}
	Window::instance()->resize(w, h, windowMode);
}

CBEXPORT void CBF_screenII(int w, int h) {
	Window::instance()->resize(w, h);
}

CBEXPORT void CBF_colorIII(int r, int g, int b) {
	setDrawColor(al_map_rgb(r, g, b));
}

CBEXPORT void CBF_colorIIII(int r, int g, int b, int a) {
	setDrawColor(al_map_rgba(r, g, b, a));
}

CBEXPORT void CBF_cls() {
	assert(RenderTarget::activated());
	al_clear_to_color(Window::instance()->backgroundColor());
}

CBEXPORT void CBF_clsColorIII(int r, int g, int b) {
	Window::instance()->setBackgroundColor(al_map_rgb(r, g, b));
}

CBEXPORT void CBF_clsColorIIII(int r, int g, int b, int a) {
	Window::instance()->setBackgroundColor(al_map_rgba(r, g, b, a));
}

CBEXPORT void CBF_lineFFFF(float x1, float y1, float x2, float y2) {
	drawLine(x1, y1, x2, y2);
}

CBEXPORT void CBF_circleFFF(float x, float y, float d) {
	drawCircle(x, y, d);
}

CBEXPORT void CBF_circleFFFI(float x, float y, float d, int fill) {
	drawCircle(x, y, d, fill);
}

CBEXPORT void CBF_boxFFFF(float x, float y, float w, float h) {
	drawBox(x, y, w, h);
}

CBEXPORT void CBF_boxFFFFI(float x, float y, float w, float h, int fill) {
	drawBox(x, y, w, h, fill);
}

CBEXPORT void CBF_textFFS(float x, float y, CBString str) {
	drawText(str, x, y);
}



