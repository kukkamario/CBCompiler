#include "system.h"
#include "error.h"
#include "window.h"

double systemTimeInSec() {
	 return (double)clock() / CLOCKS_PER_SEC;
}

clock_t systemTimeInMSec() {
	return clock() * clock_t(1000) / CLOCKS_PER_SEC;
}

void closeProgram() {
	info(U"Closing program");
	Window::instance()->close();
	exit(0);
}
