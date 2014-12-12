#include "image.h"
#include "systeminterface.h"
#include "error.h"
Image *CBF_makeImage(int w, int h) {
	Image *img = new Image(w, h);
	return img;
}

Image *CBF_loadImage(LString str) {
	info(str);
	Image *img = 0;
	img = Image::load(str);

	if (!img && sys::errorMessagesEnabled()) {
		error(LString(U"LoadImage failed!  \"%1\"").arg(str));
	}
	return img;
}

void CBF_drawImage(Image *img, float x, float y) {
	img->draw(x, y);
}

void CBF_drawImageTinted(Image *img, float x, float y) {
	img->drawTinted(x, y);
}


void CBF_drawImageBox(Image* img, float x, float y, float sx, float sy, float sw, float sh) {
	img->draw(x, y, sx, sy, sw, sh);
}

void CBF_drawScaledImageBox(Image* img, float x, float y, float sx, float sy, float sw, float sh, float w, float h) {
	img->draw(x, y, sx, sy, sw, sh, w, h);
}

void CBF_drawGhostImage(Image* img, float x, float y, float alpha) {
	img->draw(x, y, alpha);
}

void CBF_drawRotatedImage(Image* img, float x, float y, float ang, float cx, float cy) {
	img->draw(x, y, ang, cx, cy);
}

void CBF_drawImageScaled(Image* img, float x, float y, float w, float h) {
	img->draw(x, y, w, h);
}

void CBF_drawImageAdvanced(Image* img, float x, float y, float srcx, float srcy, float srcw, float srch, float cx, float cy, float sx, float sy, float ang) {
	img->draw(x, y, srcx, srcy, srcw, srch, cx, cy, sx, sy, ang);
}

void CBF_maskImage(Image *img, int r, int g, int b) {
	img->mask(al_map_rgb(r, g, b));
}

void CBF_maskImage(Image *img, int r, int g, int b, int a) {
	img->mask(al_map_rgba(r, g, b, a));
}

void CBF_drawToImage(Image *img) {
	img->activate();
}

int CBF_imageWidth(Image *img) {
	return img->width();
}

int CBF_imageHeight(Image *img) {
	return img->height();
}

