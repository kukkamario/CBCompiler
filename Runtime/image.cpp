#include "image.h"

IdMap<Image> Image::sIdMap;

Image::Image(int w, int h) {
	mId = sIdMap.add(this);
	mBitmap = al_create_bitmap(w, h);
	mWidth = w;
	mHeight = h;
}

Image::Image(ALLEGRO_BITMAP *bitmap) :
	mBitmap(bitmap) {
	mWidth = al_get_bitmap_width(bitmap);
	mHeight = al_get_bitmap_height(bitmap);
	mId = sIdMap.add(this);
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

bool Image::activate() {
	al_set_target_bitmap(mBitmap);
	setupDrawingState();
	sCurrentTarget = this;
	return true;
}

bool Image::deactivate() {
	return true;
}

Image *Image::get(int id) {
	return sIdMap.get(id);
}



bool Image::isValid() const {
	return mBitmap != 0;
}

void Image::lock(int flags) {
	al_lock_bitmap(mBitmap, 0, flags);
}

void Image::unlock() {
	al_unlock_bitmap(mBitmap);
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
