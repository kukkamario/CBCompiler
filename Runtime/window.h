#ifndef WINDOW_H
#define WINDOW_H
#include <allegro5/allegro5.h>
#include "rendertarget.h"
#include "common.h"
#include <map>

class Window : public RenderTarget{
	public:
		typedef void (*DrawCallback)(void);

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

		void lock(int flags);
		void unlock();
		void drawscreen();
		void cls();

		ALLEGRO_EVENT_QUEUE *eventQueue() const { return mEventQueue; }

		int fps() const { return mFPS; }

		ALLEGRO_DISPLAY* display();

		void handleEvent(const ALLEGRO_EVENT &event);

		void setBackgroundColor(const ALLEGRO_COLOR &color);
		const ALLEGRO_COLOR &backgroundColor() const { return mBackgroundColor; }

		void flipBuffers();
		int addDrawCallback(DrawCallback drawCallback);
		void removeDrawCallback(int handle);
	private:
		bool activateRenderContext();

		ALLEGRO_DISPLAY *mDisplay;
		ALLEGRO_EVENT_QUEUE *mEventQueue;
		ALLEGRO_COLOR mBackgroundColor;
		WindowMode mWindowMode;

		std::map<int, DrawCallback> mDrawCallbacks;

		int mFPS;
		int mFPSCounter;
		int mCallbackCounter;
		double mLastFPSUpdate;
};

#endif // WINDOW_H
