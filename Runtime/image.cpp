#include "image.h"
#include "mathinterface.h"

Image::Image(int w, int h) {
	mBitmap = al_create_bitmap(w, h);
	mWidth = w;
	mHeight = h;
}

Image::Image(ALLEGRO_BITMAP *bitmap) {
	mWidth = al_get_bitmap_width(bitmap);
	mHeight = al_get_bitmap_height(bitmap);
	mBitmap = bitmap;
}


Image::~Image() {
	al_destroy_bitmap(mBitmap);
}

Image *Image::load(const LString &path) {
	ALLEGRO_BITMAP *bitmap = al_load_bitmap(path.toUtf8().c_str());
	if (!bitmap) return 0;
	Image *img = new Image(bitmap);
	return img;
}

Image *Image::make(int w, int h) {
	ALLEGRO_BITMAP* bitmap = al_create_bitmap(w, h);
	if(!bitmap)
		return 0;
	return new Image(bitmap);
}

bool Image::activateRenderContext() {
	al_set_target_bitmap(mBitmap);
	return true;
}

bool Image::deactivate() {
	return true;
}



bool Image::isValid() const {
	return mBitmap != 0;
}


void Image::resize(int w, int h) {
	sCurrentTarget->deactivate();

	ALLEGRO_BITMAP *newBitmap = al_create_bitmap(w, h);
	al_set_target_bitmap(newBitmap);
	al_set_blender(ALLEGRO_ADD, ALLEGRO_ONE, ALLEGRO_ZERO);
	al_draw_scaled_bitmap(mBitmap, 0, 0, width(), height(), 0, 0, w, h, 0);
	al_destroy_bitmap(mBitmap);
	mBitmap = newBitmap;

	sCurrentTarget->activate();
}

void Image::mask(const ALLEGRO_COLOR &color) {
	al_convert_mask_to_alpha(mBitmap, color);
}

void Image::draw(float x, float y) {
	al_draw_bitmap(mBitmap, x, y, 0);
}

void Image::draw(float x, float y, float alpha) {
	al_draw_tinted_bitmap(mBitmap, al_map_rgba(255, 255, 255, (unsigned char)((alpha/100.0f) * 255.0f)), x, y, 0);
}

void Image::draw(float x, float y, float w, float h) {
	al_draw_scaled_bitmap(mBitmap, 0.0f, 0.0f, (float)width(), (float)height(), x, y, w, h, 0);
}

void Image::draw(float x, float y, float ang, float cx, float cy) {
	al_draw_rotated_bitmap(mBitmap, cx, cy, x, y, -math::toRad(ang), 0);
}

void Image::draw(float x, float y, float srcx, float srcy, float srcw, float srch) {
	al_draw_bitmap_region(mBitmap, srcx, srcy, srcw, srch, x, y, 0);
}

void Image::draw(float x, float y, float srcx, float srcy, float srcw, float srch, float w, float h) {
	al_draw_scaled_bitmap(mBitmap, srcx, srcy, srcw, srch, x, y, w, h, 0);
}


void Image::draw(float x, float y, float srcx, float srcy, float srcw, float srch, float cx, float cy, float sx, float sy, float ang) {
	al_draw_tinted_scaled_rotated_bitmap_region(mBitmap,
						srcx, srcy,
						srcw, srch,
						gfx::drawColor(),
						cx, cy,
						x, y,
						sx, sy,
						-math::toRad(ang),
						0);
}

void Image::drawTinted(float x, float y) {
	al_draw_tinted_bitmap(mBitmap, gfx::drawColor(), x, y, 0);
}

void Image::drawTinted(float x, float y, float w, float h) {
	al_draw_tinted_scaled_bitmap(mBitmap, gfx::drawColor(), 0.0f, 0.0f, (float)width(), (float)height(), x, y, w, h, 0);
}

void Image::drawTinted(float x, float y, float ang, float cx, float cy) {
	al_draw_tinted_rotated_bitmap(mBitmap, gfx::drawColor(), cx, cy, x, y, -math::toRad(ang), 0);
}

void Image::drawTinted(float x, float y, float srcx, float srcy, float srcw, float srch) {
	al_draw_tinted_scaled_bitmap(mBitmap, gfx::drawColor(), srcx, srcy, srcw, srch, x, y, srcw, srch, 0);
}

void Image::drawTinted(float x, float y, float srcx, float srcy, float srcw, float srch, float w, float h) {
	al_draw_tinted_scaled_bitmap(mBitmap, gfx::drawColor(), srcx, srcy, srcw, srch, x, y, w, h, 0);
}

