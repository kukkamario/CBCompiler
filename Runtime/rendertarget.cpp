#include "rendertarget.h"
RenderTarget *RenderTarget::sCurrentTarget = 0;

RenderTarget::RenderTarget() :
	mBlender(gfx::defaultBlender()) {
}

RenderTarget::~RenderTarget() {
	if (sCurrentTarget == this) {
		resetTarget();
	}
}

void RenderTarget::setBlender(const gfx::Blender &blender) {
	mBlender = blender;
}


void RenderTarget::setSize(int width, int height) {
	mWidth = width;
	mHeight = height;
}

void RenderTarget::resetTarget() {
	if (sCurrentTarget && sCurrentTarget->paintingActive()) {
		sCurrentTarget->deactivate();
	}
	sCurrentTarget = 0;
}

RenderTarget *RenderTarget::activated() {
	return sCurrentTarget;
}

void RenderTarget::setupDrawingState() {
	gfx::setBlender(mBlender);
}
