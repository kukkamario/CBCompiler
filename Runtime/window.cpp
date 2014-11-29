#include "window.h"
#include <assert.h>
#include "error.h"
#include "systeminterface.h"
#include "inputinterface.h"

static Window *sInstance = 0;

Window::Window():
	RenderTarget(),
	mDisplay(0),
	mEventQueue(0),
	mFPS(0),
	mFPSCounter(0),
	mLastFPSUpdate (0.0) {
	assert(sInstance == 0);
	sInstance = this;
}

Window::~Window() {
	sInstance = 0;
}

Window *Window::instance() {
	assert(sInstance);
	return sInstance;
}

bool Window::create(int width, int height, Window::WindowMode windowMode) {
	assert(mDisplay == 0 && mEventQueue == 0);

	switch (windowMode) {
		case Windowed:
			al_set_new_display_flags(ALLEGRO_WINDOWED | ALLEGRO_OPENGL); break;
		case Resizable:
			al_set_new_display_flags(ALLEGRO_RESIZABLE | ALLEGRO_OPENGL); break;
		case FullScreen:
			al_set_new_display_flags(ALLEGRO_FULLSCREEN | ALLEGRO_OPENGL); break;
	}
	mWindowMode = windowMode;
	al_set_new_display_option(ALLEGRO_DEPTH_SIZE,0,ALLEGRO_SUGGEST);
	al_set_new_display_option(ALLEGRO_SUPPORT_NPOT_BITMAP,1,ALLEGRO_SUGGEST);
	al_set_new_display_option(ALLEGRO_CAN_DRAW_INTO_BITMAP,1,ALLEGRO_REQUIRE);
	al_set_new_display_option(ALLEGRO_COMPATIBLE_DISPLAY,1,ALLEGRO_REQUIRE);
	al_set_new_display_option(ALLEGRO_VSYNC, 2, ALLEGRO_SUGGEST);
	mDisplay = al_create_display(width, height);
	if (mDisplay == 0) {
		error(U"Creating a window failed");

		//If fullscreen, try windowed
		if (windowMode == FullScreen) {
			return create(width, height);
		}
		else {
			return false;
		}
	}

	mEventQueue = al_create_event_queue();
	al_register_event_source(mEventQueue, al_get_display_event_source(mDisplay));

	mBackgroundColor = al_map_rgb(0,0,0);

	mBitmap = al_get_backbuffer(mDisplay);
	activate();

	return true;

}

void Window::resize(int width, int height, Window::WindowMode windowMode) {
	assert(mDisplay);

	if (windowMode == mWindowMode) {
		al_resize_display(mDisplay, width, height);
		return;
	}


	al_unregister_event_source(mEventQueue, al_get_display_event_source(mDisplay));
	al_destroy_display(mDisplay);
	switch (windowMode) {
		case Windowed:
			al_set_new_display_flags(ALLEGRO_WINDOWED | ALLEGRO_OPENGL); break;
		case Resizable:
			al_set_new_display_flags(ALLEGRO_RESIZABLE | ALLEGRO_OPENGL); break;
		case FullScreen:
			al_set_new_display_flags(ALLEGRO_FULLSCREEN | ALLEGRO_OPENGL); break;
	}
	mDisplay = al_create_display(width, height);
	if (mDisplay == 0) {
		error(U"Creating a window failed");
		sys::closeProgram();
	}

	al_register_event_source(mEventQueue, al_get_display_event_source(mDisplay));

	activate();
}

void Window::close() {
	al_destroy_display(mDisplay);
	al_destroy_event_queue(mEventQueue);
	mDisplay = 0;
	mEventQueue = 0;
}

bool Window::isValid() const {
	return mDisplay != 0;
}

bool Window::activateRenderContext() {
	al_set_target_backbuffer(mDisplay);
	return true;
}


void Window::drawscreen() {
	ALLEGRO_EVENT e;
	input::eventLoopBegin();
	while (al_get_next_event(mEventQueue, &e)) {
		handleEvent(e);
	}
	input::eventLoopEnd();
	al_flip_display();

	mFPSCounter++;
	double sysTime = sys::timeInSec();
	if (sysTime >= mLastFPSUpdate + 1.0) {
		mLastFPSUpdate = mLastFPSUpdate + 1.0;
		mFPS = mFPSCounter;
		mFPSCounter = 0;
	}

	cls();
}

void Window::cls() {
	activate();
	al_clear_to_color(mBackgroundColor);
}

ALLEGRO_DISPLAY *Window::display() {
	return mDisplay;
}

void Window::setBackgroundColor(const ALLEGRO_COLOR &color) {
	mBackgroundColor = color;
}


void Window::handleEvent(const ALLEGRO_EVENT &event) {
	switch (event.type) {
		case ALLEGRO_EVENT_MOUSE_BUTTON_DOWN:
		case ALLEGRO_EVENT_MOUSE_BUTTON_UP:
		case ALLEGRO_EVENT_MOUSE_AXES:
			input::handleMouseEvent(event);
		break;
		case ALLEGRO_EVENT_KEY_DOWN:
		case ALLEGRO_EVENT_KEY_UP:
			input::handleKeyEvent(event); break;
		case ALLEGRO_EVENT_DISPLAY_CLOSE:
			sys::closeProgram(); break;
	}
}
