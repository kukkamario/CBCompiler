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

void CBF_write(int i) {
	printf("%i", i);
}

void CBF_write(float f) {
	printf("%f", f);
}

void CBF_write(LString s) {
	sys::writeToOutput(s);
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

void CBF_setWindow(LString cbstr) {
	al_set_window_title(Window::instance()->display(), cbstr.toUtf8().c_str());
}


void CBF_setWindow(int val) {
	al_set_window_title(Window::instance()->display(), LString::number(val).toUtf8().c_str());
}

void CBF_setWindow(float val) {
	al_set_window_title(Window::instance()->display(), LString::number(val).toUtf8().c_str());
}

int CBF_addWindowDrawCallback(void (*callback)()) {
	return Window::instance()->addDrawCallback(callback);
}
