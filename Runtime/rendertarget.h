#ifndef RENDERTARGET_H
#define RENDERTARGET_H
#include "gfxinterface.h"

class RenderTarget {
	public:
		RenderTarget();
		virtual ~RenderTarget();
		bool activate();
		virtual bool deactivate() { return true; }
		virtual bool isValid() const = 0;
		virtual void lock(int flags) = 0;
		virtual void unlock() = 0;
		bool paintingActive() const { return sCurrentTarget == this; }
		const gfx::Blender &blender() const { return mBlender; }
		void setBlender(const gfx::Blender &blender);

		int width() const {return mWidth; }
		int height() const { return mHeight; }
		void setSize(int width, int height);

		static RenderTarget *activated();
		static void setFallbackRenderTarget(RenderTarget *t);
	protected:
		virtual bool activateRenderContext() = 0;

		void setupDrawingState();
		int mWidth;
		int mHeight;
		gfx::Blender mBlender;

		static RenderTarget *sCurrentTarget;
		static RenderTarget *sFallbackRenderTarget;
	private:
};

#endif // RENDERTARGET_H
