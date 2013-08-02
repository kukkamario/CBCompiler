#include "image.h"

CBEXPORT int makeImage(int w, int h) {
	Image *img = new Image(w, h);
	return img->id();
}
