#include "image.h"
#include "systeminterface.h"
#include "error.h"

Image *CBF_makeImage(int w, int h) {

	Image* img = Image::make(w, h);
	if(!img)
		error(LString(U"MakeImage failed!"));
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

Image *CBF_cloneImage(Image* img) {

	Image* out = Image::clone(img);
	if(!out)
		error(LString(U"CloneImage failed!"));
	return out;
}

void CBF_drawImage(Image *img, float x, float y) {
	img->draw(x, y);
}

void CBF_drawImageBox(Image* img, float x, float y, float sx, float sy, float sw, float sh) {
	img->draw(x, y, sx, sy, sw, sh);
}

void CBF_drawImageBoxScaled(Image* img, float x, float y, float sx, float sy, float sw, float sh, float w, float h) {
	img->draw(x, y, sx, sy, sw, sh, w, h);
}

void CBF_drawImageBoxResized(Image* img, float x, float y, float sx, float sy, float sw, float sh, float w, float h) {
	img->draw(x, y, sx, sy, sw, sh, sw*w, sh*h);
}

void CBF_drawGhostImage(Image* img, float x, float y, float alpha) {
	img->draw(x, y, alpha);
}

void CBF_drawRotatedImage(Image* img, float x, float y, float ang, float cx, float cy) {
	img->draw(x, y, ang, cx, cy);
}

void CBF_drawRotatedImage(Image* img, float x, float y, float ang) {
	img->draw(x, y, ang, 0.0f, 0.0f);
}

void CBF_drawImageScaledRotated(Image* img, float x, float y, float ang, float w, float h, float cx, float cy) {
	img->draw(x, y, w, h, cx, cy, ang);
}

void CBF_drawImageResizedRotated(Image* img, float x, float y, float ang, float w, float h, float cx, float cy) {
	img->draw(x, y,(float)img->width()*w, (float)img->height()*h, cx, cy, ang);
}

void CBF_drawImageScaledRotated(Image* img, float x, float y, float ang, float w, float h) {
	img->draw(x, y, w, h, 0.0f, 0.0f, ang);
}

void CBF_drawImageResizedRotated(Image* img, float x, float y, float ang, float w, float h) {
	img->draw(x, y,(float)img->width()*w, (float)img->height()*h, 0.0f, 0.0f, ang);
}


void CBF_drawImageScaled(Image* img, float x, float y, float w, float h) {
	img->draw(x, y, (float)img->width()*w, (float)img->height()*h);
}

void CBF_drawImageResized(Image* img, float x, float y, float w, float h) {
	img->draw(x, y, w, h);
}

void CBF_drawImageAdvanced(Image* img, float x, float y, float srcx, float srcy, float srcw, float srch, float w, float h, float ang, float cx, float cy) {
	img->draw(x, y, srcx, srcy, srcw, srch, cx, cy, w, h, ang);
}

void CBF_drawImageAdvanced2(Image* img, float x, float y, float srcx, float srcy, float srcw, float srch, float w, float h, float ang, float cx, float cy) {
	img->draw(x, y, srcx, srcy, srcw, srch, cx, cy, w*srcw, h*srch, ang);
}

void CBF_drawImageAdvanced(Image* img, float x, float y, float srcx, float srcy, float srcw, float srch, float w, float h, float ang) {
	img->draw(x, y, srcx, srcy, srcw, srch, 0.0f, 0.0f, w, h, ang);
}

void CBF_drawImageAdvanced2(Image* img, float x, float y, float srcx, float srcy, float srcw, float srch, float w, float h, float ang) {
	img->draw(x, y, srcx, srcy, srcw, srch, 0.0f, 0.0f, w*srcw, h*srch, ang);
}

void CBF_drawTintedImage(Image *img, float x, float y) {
	img->drawTinted(x, y);
}

void CBF_drawTintedImageBox(Image *img, float x, float y, float sx, float sy, float sw, float sh) {
	img->drawTinted(x, y, sx, sy, sw, sh);
}

void CBF_drawTintedImageBoxScaled(Image *img, float x, float y, float sx, float sy, float sw, float sh, float w, float h) {
	img->drawTinted(x, y, sx, sy, sw, sh, sw*w, sh*h);
}

void CBF_drawTintedImageBoxResized(Image *img, float x, float y, float sx, float sy, float sw, float sh, float w, float h) {
	img->drawTinted(x, y, sx, sy, sw, sh, w, h);
}

void CBF_drawTintedImageRotated(Image *img, float x, float y, float ang, float cx, float cy) {
	img->drawTinted(x, y, ang, cx, cy);
}

void CBF_drawTintedImageRotated(Image *img, float x, float y, float ang) {
	img->drawTinted(x, y, ang, 0.0f, 0.0f);
}

void CBF_drawTintedImageScaledRotated(Image* img, float x, float y, float ang, float w, float h, float cx, float cy) {
	img->drawTinted(x, y, w, h, cx, cy, ang);
}

void CBF_drawTintedImageResizedRotated(Image* img, float x, float y, float ang, float w, float h, float cx, float cy) {
	img->drawTinted(x, y,(float)img->width()*w, (float)img->height()*h, cx, cy, ang);
}

void CBF_drawTintedImageScaledRotated(Image* img, float x, float y, float ang, float w, float h) {
	img->drawTinted(x, y, w, h, 0.0f, 0.0f, ang);
}

void CBF_drawTintedImageResizedRotated(Image* img, float x, float y, float ang, float w, float h) {
	img->drawTinted(x, y,(float)img->width()*w, (float)img->height()*h, 0.0f, 0.0f, ang);
}

void CBF_drawTintedImageScaled(Image *img, float x, float y, float w, float h) {
	img->drawTinted(x, y, (float)img->width()*w, (float)img->height()*h);
}

void CBF_drawTintedImageResized(Image *img, float x, float y, float w, float h) {
	img->drawTinted(x, y, w, h);
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

