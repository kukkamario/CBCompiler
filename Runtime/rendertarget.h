#ifndef RENDERTARGET_H
#define RENDERTARGET_H
class RenderTarget {
	public:
		RenderTarget();
		virtual ~RenderTarget();
		virtual bool activate() = 0;
		virtual bool deactivate() = 0;
		virtual bool isValid() const = 0;
		bool paintingActive() const { return sCurrentTarget == this; }

		int width() const {return mWidth; }
		int height() const { return mHeight; }
		void setSize(int width, int height);

		static void resetTarget();
		static RenderTarget *activated();
	protected:
		int mWidth;
		int mHeight;
		static RenderTarget *sCurrentTarget;
	private:
};

#endif // RENDERTARGET_H
