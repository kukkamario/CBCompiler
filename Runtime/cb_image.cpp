#include "image.h"

Image *CBF_makeImage(int w, int h) {
	Image *img = new Image(w, h);
	return img;
}
