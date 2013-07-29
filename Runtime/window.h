#ifndef WINDOW_H
#define WINDOW_H
#include <allegro5/allegro5.h>
#include "rendertarget.h"
#include "common.h"
class Window : RenderTarget{
	public:
		enum WindowMode {
			Windowed,
			Resizable,
			FullScreen
		};
		Window();
		~Window();
		static Window *instance();

		bool create(int width = 400, int height = 300, WindowMode windowMode = Windowed);
		void resize(int width = 400, int height = 300, WindowMode windowMode = Windowed);

		void close();

		bool isValid() const;
		bool activate();
		bool deactivate();
		void lock(int flags);
		void unlock();
		void drawscreen();
		void cls();

		int fps() const { return mFPS; }

		void handleEvent(const ALLEGRO_EVENT &event);

		void setBackgroundColor(const ALLEGRO_COLOR &color);
		const ALLEGRO_COLOR &backgroundColor() const { return mBackgroundColor; }
	private:
		ALLEGRO_DISPLAY *mDisplay;
		ALLEGRO_EVENT_QUEUE *mEventQueue;
		ALLEGRO_COLOR mBackgroundColor;
		WindowMode mWindowMode;

		int mFPS;
		int mFPSCounter;
		double mLastFPSUpdate;
};

#endif // WINDOW_H
