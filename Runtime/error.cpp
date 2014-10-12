#include "error.h"
#include <iostream>
#ifdef _WIN32
	#include <windows.h>
#endif
void error(const LString &string) {
	std::cout << "Error\n";
	return;
#ifndef _WIN32
	std::cerr << string.toUtf8() << std::endl;
#else
	HANDLE consoleHandle = GetStdHandle(STD_ERROR_HANDLE);
	std::wstring wstr = string.toWString();
	WriteConsoleW(consoleHandle, wstr.c_str(), wstr.size(), 0, 0);
#endif
}


void info(const LString &string) {
	std::cout << "Info\n";
	return;
#ifndef _WIN32
	std::cout << string.toUtf8() << std::endl;
#else
	HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
	std::wstring wstr = string.toWString();
	WriteConsoleW(consoleHandle, wstr.c_str(), wstr.size(), 0, 0);
#endif
}


void error(const std::string &txt) {
	std::cout << txt << std::endl;
}
