#include "rendertarget.h"
RenderTarget *RenderTarget::sCurrentTarget = 0;

RenderTarget::RenderTarget()
{
}

RenderTarget::~RenderTarget() {
	if (sCurrentTarget == this) {
		resetTarget();
	}
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
