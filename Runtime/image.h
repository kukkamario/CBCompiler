#ifndef IMAGE_H
#define IMAGE_H
#include "common.h"
#include "rendertarget.h"
#include <allegro5/allegro.h>

class Image : public RenderTarget {
	public:
		Image(int w, int h);
		~Image();
		static Image *load(const LString &path);

		bool deactivate();
		bool isValid() const;

		void resize(int w, int h);
		void mask(const ALLEGRO_COLOR &color);
		ALLEGRO_BITMAP *getBitmap() const { return mBitmap; }

		void draw(float x, float y);

	private:
		bool activateRenderContext();

		Image(ALLEGRO_BITMAP *bitmap);

		ALLEGRO_BITMAP *mBitmap;
};

#endif // IMAGE_H
