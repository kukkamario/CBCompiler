#ifndef TEXT_H
#define TEXT_H
#include "common.h"
#include <allegro5/allegro_font.h>
namespace text {
	void initText();
	ALLEGRO_FONT *currentFont();
}
#endif // TEXT_H
