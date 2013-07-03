#include "gfx.h"
#include "error.h"
#include "rendertarget.h"
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_font.h>
#include "text.h"

static ALLEGRO_COLOR sCurrentColor;

void initGfx() {
	al_init_primitives_addon();
	sCurrentColor = al_map_rgb(255, 255, 255);
}


void setDrawColor(const ALLEGRO_COLOR &color) {
	sCurrentColor = color;
}


const ALLEGRO_COLOR &drawColor() {
	return sCurrentColor;
}


void drawLine(float x1, float y1, float x2, float y2) {
	assert(RenderTarget::activated());
	al_draw_line(x1, y1, x2, y2, sCurrentColor, 1.0f);
}


void drawCircle(float x, float y, float d, bool fill) {
	float r = d * 0.5f;
	if (fill) {
		al_draw_filled_circle(x - r, y - r, r, sCurrentColor);
	}
	else {
		al_draw_circle(x - r, y - r, r, sCurrentColor, 1.0f);
	}
}


void drawText(const String &str, float x, float y) {
	assert(RenderTarget::activated());
	ALLEGRO_USTR *ustr = str.toALLEGRO_USTR();
	if (ustr) {
		al_draw_ustr(currentFont(), sCurrentColor, x, y, 0, ustr);
		al_ustr_free(ustr);
	}
}


void drawBox(float x, float y, float w, float h, bool fill) {
	assert(RenderTarget::activated());
	if (fill) {
		al_draw_filled_rectangle(x, y, x + w, y + h, sCurrentColor);
	}
	else {
		al_draw_rectangle(x, y, x + w, y + h, sCurrentColor, 1.0f);
	}
}
