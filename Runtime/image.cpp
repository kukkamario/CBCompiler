#include "image.h"

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

