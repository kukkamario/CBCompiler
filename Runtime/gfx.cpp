#include "window.h"
#include <stdio.h>
#include "common.h"
#include <allegro5/allegro_primitives.h>

static ALLEGRO_COLOR sCurrentColor;

void initGfx() {
	sCurrentColor = al_map_rgb(255, 255, 255);
}

CBEXPORT void CBF_lineFFFF(float x1, float y1, float x2, float y2) {
	assert(RenderTarget::activated());
	al_draw_line(x1, y1, x2, y2, sCurrentColor, 3.0);
}

CBEXPORT void CBF_drawscreen() {
	Window::instance()->drawscreen();
}

CBEXPORT void CBF_colorIII(int r, int g, int b) {
	sCurrentColor = al_map_rgb(r, g, b);
}

CBEXPORT void CBF_colorIIII(int r, int g, int b, int a) {
	sCurrentColor = al_map_rgba(r, g, b, a);
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


