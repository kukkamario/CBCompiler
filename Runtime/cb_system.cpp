#include <stdio.h>
#include <time.h>
#include <iostream>
#include <locale>
#include <stdint.h>
#include "window.h"
#include "error.h"
#include "systeminterface.h"

CBEXPORT char *CB_Allocate(int size) {
	info(U"Allocated");
	//info(U"Allocated " + LString::number(size));
	return new char[size];
}

CBEXPORT void CB_Free(char *mem) {
	info(U"Free");
	delete [] mem;
}

int CBF_int(float f) {
	return int(f + 0.5f);
}

int CBF_int(LString s) {
	return s.toInt();
}


void CBF_print(int i) {
	printf("%i\n", i);
}

void CBF_print(float f) {
	printf("%f\n", f);
}

void CBF_print(LString s) {
	sys::writeToOutput(s + L'\n');
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

void CBF_makeError(LString errorMsg) {
	error(errorMsg);
	sys::closeProgram();
}

void CBF_wait(int t) {
	al_rest((double)t / 1000.0);
}

