#include "gfxinterface.h"
#include "window.h"
#include "error.h"
#include <stdio.h>
#include "common.h"
#include "image.h"
#include <allegro5/allegro_primitives.h>

using namespace gfx;
void CBF_drawscreen() {
	//printf("DrawScreen\n");
	Window::instance()->drawscreen();
}

void CBF_screen(int w, int h, int mode) {
	//printf("Screen\n");
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

void CBF_screen(int w, int h) {
	Window::instance()->resize(w, h);
}

void CBF_color(int r, int g, int b) {
	//printf("Color %i, %i, %i\n", r, g, b);
	setDrawColor(al_map_rgb(r, g, b));
}

void CBF_color(int r, int g, int b, int a) {
	setDrawColor(al_map_rgba(r, g, b, a));
}

void CBF_cls() {
	assert(RenderTarget::activated());
	al_clear_to_color(Window::instance()->backgroundColor());
}

void CBF_clsColor(int r, int g, int b) {
	//printf("ClsColor %i, %i, %i\n", r, g, b);
	Window::instance()->setBackgroundColor(al_map_rgb(r, g, b));
}

void CBF_clsColor(int r, int g, int b, int a) {
	Window::instance()->setBackgroundColor(al_map_rgba(r, g, b, a));
}

void CBF_line(float x1, float y1, float x2, float y2) {
	drawLine(x1, y1, x2, y2);
}

void CBF_circle(float x, float y, float d) {
	//printf("Circle %f, %f, %f\n", x, y, d);
	drawCircle(x, y, d);
}

void CBF_circle(float x, float y, float d, int fill) {
	drawCircle(x, y, d, fill);
}

void CBF_box(float x, float y, float w, float h) {
	drawBox(x, y, w, h);
}

void CBF_box(float x, float y, float w, float h, int fill) {
	drawBox(x, y, w, h, fill);
}

void CBF_text(float x, float y, CBString str) {
	drawText(str, x, y);
}

void CBF_dot(float x, float y) {
	drawDot(x, y);
}

int CBF_screen() {
	return 0;
}

void CBF_lock() {
	RenderTarget::activated()->lock(ALLEGRO_LOCK_READWRITE);
}

void CBF_lock2(int state) {
	int flags = 0;
	switch (state) {
		case 1:
			flags = ALLEGRO_LOCK_READONLY; break;
		case 2:
			flags = ALLEGRO_LOCK_WRITEONLY; break;
		default:
			flags = ALLEGRO_LOCK_READWRITE; break;
	}

	RenderTarget::activated()->lock(flags);
}

void CBF_lock(Image *img) {
	img->lock(ALLEGRO_LOCK_READWRITE);
}

void CBF_lock2(Image *img, int state) {
	int flags = 0;
	switch (state) {
		case 1:
			flags = ALLEGRO_LOCK_READONLY; break;
		case 2:
			flags = ALLEGRO_LOCK_WRITEONLY; break;
		default:
			flags = ALLEGRO_LOCK_READWRITE; break;
	}

	img->lock(flags);
}

void CBF_unlock() {
	RenderTarget::activated()->unlock();
}


void CBF_unlock(Image *img) {
	img->unlock();
}


void CBF_putpixel(int x, int y, unsigned char r, unsigned char g, unsigned char b) {
	int pixel = (255 << 24) + ((int)r << 16) + ((int)g << 8) + (int)b;
	RenderTarget::activated()->putPixel(x, y, pixel);
}

void CBF_putpixel(int x, int y, unsigned char r, unsigned char g, unsigned char b, unsigned char a) {
	int pixel = ((int)a << 24) + ((int)r << 16) + ((int)g << 8) + (int)b;
	RenderTarget::activated()->putPixel(x, y, pixel);
}

void CBF_putpixel(int x, int y, unsigned int pixel) {
	RenderTarget::activated()->putPixel(x, y, pixel);
}

void CBF_putpixel(Image *img, int x, int y, unsigned int pixel) {
	img->putPixel(x, y, pixel);
}

void CBF_putpixel(int x, int y, float r, float g, float b) {
	al_put_pixel(x, y, al_map_rgb_f(r, g, b));
}

void CBF_putpixel(int x, int y, float r, float g, float b, float a) {
	al_put_pixel(x, y, al_map_rgba_f(r, g, b, a));
}


int CBF_getPixel(int x, int y) {
	return RenderTarget::activated()->getPixel(x, y);
}

int CBF_getPixel(Image *img, int x, int y) {
	return img->getPixel(x, y);
}



void CBF_drawToScreen() {
	Window::instance()->activate();
}


