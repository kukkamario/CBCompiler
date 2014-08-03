#include "rendertarget.h"
RenderTarget *RenderTarget::sCurrentTarget = 0;
RenderTarget *RenderTarget::sFallbackRenderTarget = 0;

RenderTarget::RenderTarget() :
	mBlender(gfx::defaultBlender()),
	mBitmap(0),
	mLockedRegion(0) {
}

RenderTarget::~RenderTarget() {
	if (sCurrentTarget == this) {
		if (sCurrentTarget != sFallbackRenderTarget && sFallbackRenderTarget) {
			sCurrentTarget = sFallbackRenderTarget;
			sCurrentTarget->activate();
		}
		else {
			sFallbackRenderTarget = 0;
			sCurrentTarget = 0;
		}
	}
}

bool RenderTarget::activate() {
	if (sCurrentTarget == this) return true;
	if (sCurrentTarget && !sCurrentTarget->deactivate()) return false;
	sCurrentTarget = this;
	if (!sCurrentTarget->activateRenderContext()) return false;
	sCurrentTarget->setupDrawingState();
	return true;
}

void RenderTarget::setBlender(const gfx::Blender &blender) {
	mBlender = blender;
}


void RenderTarget::setSize(int width, int height) {
	mWidth = width;
	mHeight = height;
}

void RenderTarget::putPixel(int x, int y, int pixel) {
	if (mLockedRegion) {
		*reinterpret_cast<int*>(reinterpret_cast<char*>(mLockedRegion->data) + mLockedRegion->pitch * y + x * 4) = pixel;
	}
	else {
		al_put_pixel(x, y, al_map_rgba((pixel >> 16) & 0xFF, (pixel >> 8) & 0xFF, pixel & 0xFF, (pixel >> 24) & 0xFF));
	}
}

int RenderTarget::getPixel(int x, int y) const {
	if (mLockedRegion) {
		return *reinterpret_cast<int*>(reinterpret_cast<char*>(mLockedRegion->data) + mLockedRegion->pitch * y + x * 4);
	}
	else {
		ALLEGRO_COLOR color =  al_get_pixel(getBitmap(), x, y);
		uint8_t r, g, b, a;
		al_unmap_rgba(color, &r, &g, &b, &a);
		return ((int)a << 24) + ((int)r << 16) + ((int)g << 8) + (int)b;
	}
}

void RenderTarget::lock(int flags) {
	mLockedRegion = al_lock_bitmap(getBitmap(), ALLEGRO_PIXEL_FORMAT_ANY, flags);
}

void RenderTarget::unlock() {
	al_unlock_bitmap(getBitmap());
}

RenderTarget *RenderTarget::activated() {
	return sCurrentTarget;
}

void RenderTarget::setFallbackRenderTarget(RenderTarget *t) {
	sFallbackRenderTarget = t;
}

void RenderTarget::setupDrawingState() {
	gfx::setBlender(mBlender);
}
