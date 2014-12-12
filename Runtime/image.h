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
		static Image* make(int w, int h);
		bool deactivate();
		bool isValid() const;

		void resize(int w, int h);
		void mask(const ALLEGRO_COLOR &color);


		void draw(float x, float y);
		void draw(float x, float y, float alpha);
		void draw(float x, float y, float w, float h);
		void draw(float x, float y, float ang, float cx, float cy);
		void draw(float x, float y, float srcx, float srcy, float srcw, float srch);
		void draw(float x, float y, float srcx, float srcy, float srcw, float srch, float w, float h);
		void draw(float x, float y, float srcx, float srcy, float srcw, float srch, float cx, float cy, float sx, float sy, float ang);

		void drawTinted(float x, float y);
		void drawTinted(float x, float y, float w, float h);
		void drawTinted(float x, float y, float ang, float cx, float cy);
		void drawTinted(float x, float y, float srcx, float srcy, float srcw, float srch);
		void drawTinted(float x, float y, float srcx, float srcy, float srcw, float srch, float w, float h);
	private:
		bool activateRenderContext();
		Image(ALLEGRO_BITMAP *bitmap);
};

#endif // IMAGE_H
