#ifndef IMAGE_H
#define IMAGE_H
#include "common.h"
#include "rendertarget.h"
#include "idmap.h"
#include <allegro5/allegro.h>

class Image : public RenderTarget {
	public:
		Image(int w, int h);
		~Image();
		static Image *load(const LString &path);

		bool activate();
		bool deactivate();
		bool isValid() const;
		void lock(int flags);
		void unlock();

		void resize(int w, int h);
		void mask(const ALLEGRO_COLOR &color);

		static Image *get(int id);

		int id() const { return mId; }
	private:
		Image(ALLEGRO_BITMAP *bitmap);

		ALLEGRO_BITMAP *mBitmap;
		int mId;

		static IdMap<Image> sIdMap;
};

#endif // IMAGE_H
