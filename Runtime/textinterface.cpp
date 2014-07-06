#include "textinterface.h"
#include <allegro5/allegro_ttf.h>
#include "error.h"

static ALLEGRO_FONT *mDefaultFont = 0;
static ALLEGRO_FONT *mCurrentFont = 0;
void text::initText() {
	al_init_font_addon();
	al_init_ttf_addon();
#ifdef WIN32
	mDefaultFont = al_load_font((getenv("WINDIR") + std::string("\\Fonts\\cour.ttf")).c_str(), 12, ALLEGRO_TTF_MONOCHROME);
#elif __MACH__
	mDefaultFont = al_load_font("/Library/Fonts/Courier New.ttf", 12, ALLEGRO_TTF_MONOCHROME);
#else
	mDefaultFont = al_load_font("/usr/share/fonts/truetype/freefont/FreeSans.ttf", 12, ALLEGRO_TTF_MONOCHROME);
#endif
	mCurrentFont = mDefaultFont;

	if (!mDefaultFont) {
		error(U"Failed to default font");
	}
}


ALLEGRO_FONT *text::currentFont() {
	assert(mCurrentFont);
	return mCurrentFont;
}
