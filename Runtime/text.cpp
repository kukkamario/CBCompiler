#include "text.h"
#include <allegro5/allegro_ttf.h>
#include "error.h"

static ALLEGRO_FONT *mDefaultFont = 0;
static ALLEGRO_FONT *mCurrentFont = 0;
void initText() {
	al_init_font_addon();
	al_init_ttf_addon();
#ifdef WIN32
	mDefaultFont = al_load_font((getenv("WINDIR") + std::string("\\Fonts\\cour.ttf")).c_str(), 12, ALLEGRO_TTF_MONOCHROME);
#endif

	mCurrentFont = mDefaultFont;

	if (!mDefaultFont) {
		error(U"Failed to default font");
	}
}


ALLEGRO_FONT *currentFont() {
	assert(mCurrentFont);
	return mCurrentFont;
}
