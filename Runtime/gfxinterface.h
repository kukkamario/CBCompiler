#ifndef GFX_H
#define GFX_H
#include "common.h"
#include <allegro5/allegro5.h>
namespace gfx {
	//http://alleg.sourceforge.net/a5docs/5.0.10/graphics.html#al_set_blender
	struct Blender {
			Blender(int op, int src, int dest) : mOp(op), mSrc(src), mDest(dest) {}
			int mOp;
			int mSrc;
			int mDest;
	};

	void initGfx();
	const ALLEGRO_COLOR &drawColor();
	void pushColor();
	void popColor();
	void setDrawColor(const ALLEGRO_COLOR &color);
	void drawLine(float x1, float y1, float x2, float y2);
	void drawCircle(float x, float y, float d, bool fill = true);
	void drawText(const LString &str, float x, float y);
	void drawBox(float x, float y, float w, float h, bool fill = true);
	void drawDot(float x, float y);
	const Blender &defaultBlender();
	void setBlender(const Blender &blender);
}
#endif // GFX_H
