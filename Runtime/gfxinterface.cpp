#include "gfxinterface.h"
#include "error.h"
#include "rendertarget.h"
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_font.h>
#include "textinterface.h"
#include <vector>

//static ALLEGRO_COLOR sCurrentColor;
static gfx::Blender sDefaultBlender(ALLEGRO_ADD, ALLEGRO_ALPHA, ALLEGRO_INVERSE_ALPHA);
static std::vector<ALLEGRO_COLOR> sColorStack;


void gfx::initGfx() {
	al_init_primitives_addon();
	al_init_image_addon();
	sColorStack.push_back(al_map_rgb(255, 255, 255));
	al_set_new_bitmap_format(ALLEGRO_PIXEL_FORMAT_ARGB_8888);
}


void gfx::setDrawColor(const ALLEGRO_COLOR &color) {
	sColorStack.back() = color;
}


const ALLEGRO_COLOR &gfx::drawColor() {
	return sColorStack.back();
}

void gfx::pushColor() {
	ALLEGRO_COLOR back = sColorStack.back();
	sColorStack.push_back(back);
}

void gfx::popColor() {
	if(sColorStack.size() > 1)
		sColorStack.pop_back();
}


void gfx::drawLine(float x1, float y1, float x2, float y2) {
	assert(RenderTarget::activated());
	al_draw_line(x1, y1, x2, y2, sColorStack.back(), 1.0f);
}


void gfx::drawCircle(float x, float y, float d, bool fill) {
	assert(RenderTarget::activated());
	if (d <= 0) return;
	float r = d * 0.5f;
	if (fill) {
		al_draw_filled_circle(x + r, y + r, r, sColorStack.back());
	}
	else {
		al_draw_circle(x + r, y + r, r, sColorStack.back(), 1.0f);
	}
}


void gfx::drawText(const LString &str, float x, float y) {
	assert(RenderTarget::activated());
	ALLEGRO_USTR *ustr = str.toAllegroUStr();
	if (ustr) {
		al_draw_ustr(text::currentFont(), sColorStack.back(), x, y, 0, ustr);
		al_ustr_free(ustr);
	}
}


void gfx::drawBox(float x, float y, float w, float h, bool fill) {
	assert(RenderTarget::activated());
	if (fill) {
		al_draw_filled_rectangle(x, y, x + w, y + h, sColorStack.back());
	}
	else {
		al_draw_rectangle(x, y, x + w, y + h, sColorStack.back(), 1.0f);
	}
}


void gfx::drawDot(float x, float y) {
	assert(RenderTarget::activated());
	al_draw_pixel(x, y, sColorStack.back());
}


void gfx::setBlender(const gfx::Blender &blender) {
	al_set_blender(blender.mOp, blender.mSrc, blender.mDest);
}


const gfx::Blender &gfx::defaultBlender() {
	return sDefaultBlender;
}


