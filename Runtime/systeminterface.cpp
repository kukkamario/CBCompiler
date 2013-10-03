#include "systeminterface.h"
#include "error.h"
#include "window.h"

static bool sErrorMessagesEnabled = true;

double sys::timeInSec() {
	 return (double)clock() / CLOCKS_PER_SEC;
}

clock_t sys::timeInMSec() {
	return clock() * clock_t(1000) / CLOCKS_PER_SEC;
}

void sys::closeProgram() {
	info(U"Closing program");
	Window::instance()->close();
	exit(0);
}


bool sys::errorMessagesEnabled() {
	return sErrorMessagesEnabled;
}
