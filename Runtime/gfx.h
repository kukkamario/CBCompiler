#ifndef GFX_H
#define GFX_H
#include "common.h"
#include <allegro5/allegro5.h>

void initGfx();
const ALLEGRO_COLOR &drawColor();
void setDrawColor(const ALLEGRO_COLOR &color);
void drawLine(float x1, float y1, float x2, float y2);
void drawCircle(float x, float y, float d, bool fill = true);
void drawText(const String &str, float x, float y);
void drawBox(float x, float y, float w, float h, bool fill = true);
#endif // GFX_H
