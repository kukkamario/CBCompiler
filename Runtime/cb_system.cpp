#include <stdio.h>
#include <time.h>
#include <iostream>
#include <locale>
#include <stdint.h>
#include "window.h"
#include "error.h"
#include "systeminterface.h"

CBEXPORT char *CB_Allocate(int size) {
	return new char[size];
}

CBEXPORT void CB_Free(char *mem) {
	delete [] mem;
}

int CBF_int(float f) {
	return int(f + 0.5f);
}

int CBF_int(CBString s) {
	return LString(s).toInt();
}

//extern "C" float CBF_floatI(int i) {
//	return float(i);
//}

//extern "C" float CBF_floatS(CBString s) {
//	return LString(s).toFloat();
//}


void CBF_print(int i) {
	printf("%i\n", i);
}

void CBF_print(float f) {
	printf("%f\n", f);
}

void CBF_print(CBString s) {
#ifdef _WIN32 //sizeof(wchar_t) == 2
	std::wcout << LString(s).toWString() << std::endl;
#else //wchar_t == char32_t
	std::cout << LString(s).toUtf8() << std::endl;
#endif
}

void CBF_print() {
	printf("\n");
}

int CBF_timer() {
	return sys::timeInMSec();
}

void CBF_end() {
	sys::closeProgram();
}

int CBF_fps() {
	return Window::instance()->fps();
}

