#include "systeminterface.h"
#include "error.h"
#include "window.h"
#include <iostream>
#ifdef _WIN32
	#include <windows.h>
	HANDLE sOutputHandle = INVALID_HANDLE_VALUE;
	HANDLE sErrorHandle = INVALID_HANDLE_VALUE;
#endif
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


void sys::init() {
	sOutputHandle = GetStdHandle(STD_OUTPUT_HANDLE);
	sErrorHandle = GetStdHandle(STD_ERROR_HANDLE);
}


void sys::writeToOutput(const LString &s) {
#ifdef _WIN32 //sizeof(wchar_t) == 2
	//std::wcout << s.toWString();
	std::wstring wstr = s.toWString();
	WriteConsoleW(sOutputHandle, wstr.c_str(), wstr.size(), 0, 0);
#else //wchar_t == char32_t
	std::cout << s.toUtf8();
#endif
}


void sys::writeToError(const LString &s) {
#ifdef _WIN32 //sizeof(wchar_t) == 2
	std::wstring wstr = s.toWString();
	WriteConsoleW(sErrorHandle, wstr.c_str(), wstr.size(), 0, 0);
#else //wchar_t == char32_t
	std::cout << s.toUtf8();
#endif
}
