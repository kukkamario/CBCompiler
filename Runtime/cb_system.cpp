#include <stdio.h>
#include <time.h>
#include "cbstring.h"
#include <iostream>
#include <locale>
#include <stdint.h>
#include "window.h"
#include "error.h"
#include "system.h"

extern "C" char *CBF_CB_Allocate(int size) {
	return new char[size];
}

extern "C" void CBF_CB_Free(char *mem) {
	delete [] mem;
}

extern "C" void CBF_printI(int i) {
	printf("%i\n", i);
}

extern "C" void CBF_printF(float f) {
	printf("%f\n", f);
}

extern "C" void CBF_printS(CBString s) {
#ifdef _WIN //sizeof(wchar_t) == 2
	std::cout << "Fix printS\n";
#else //wchar_t == char32_t
	std::cout << String(s).toUtf8() << "\n";
#endif
}

extern "C" void CBF_print() {
	printf("\n");
}

extern "C" int CBF_timer() {
	return systemTimeInMSec();
}

extern "C" void CBF_end() {
	closeProgram();
}

extern "C" int CBF_fps() {
	return Window::instance()->fps();
}
