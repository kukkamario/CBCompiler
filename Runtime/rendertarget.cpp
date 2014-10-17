#include "rendertarget.h"
#include "error.h"
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
			error("Warning: Active RenderTarget deleted");
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
	assert(mLockedRegion && "RenderTarget isn't locked");
	*reinterpret_cast<int*>(reinterpret_cast<char*>(mLockedRegion->data) + mLockedRegion->pitch * y + x * 4) = pixel;
}

int RenderTarget::getPixel(int x, int y) const {
	assert(mLockedRegion && "RenderTarget isn't locked");
	return *reinterpret_cast<int*>(reinterpret_cast<char*>(mLockedRegion->data) + mLockedRegion->pitch * y + x * 4);
}

void RenderTarget::lock(int flags) {
	mLockedRegion = al_lock_bitmap(getBitmap(), ALLEGRO_PIXEL_FORMAT_ABGR_8888, flags);
}

void RenderTarget::unlock() {
	al_unlock_bitmap(getBitmap());
	mLockedRegion = 0;
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
