#include "error.h"
#include <iostream>
void error(const LString &string) {

#ifndef _WIN32
	std::cerr << string.toUtf8() << '\n';
#else
	std::wcerr << string.toWString() << '\n';
#endif
}


void info(const LString &string) {
#ifndef _WIN32
	std::cout << string.toUtf8() << '\n';
#else
	std::wcout << string.toWString() << '\n';
#endif
}


void error(const std::string &txt) {
	std::cout << txt << '\n';
}
