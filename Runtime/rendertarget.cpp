#include "rendertarget.h"
RenderTarget *RenderTarget::sCurrentTarget = 0;
RenderTarget *RenderTarget::sFallbackRenderTarget = 0;

RenderTarget::RenderTarget() :
	mBlender(gfx::defaultBlender()) {
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

RenderTarget *RenderTarget::activated() {
	return sCurrentTarget;
}

void RenderTarget::setFallbackRenderTarget(RenderTarget *t) {
	sFallbackRenderTarget = t;
}

void RenderTarget::setupDrawingState() {
	gfx::setBlender(mBlender);
}
