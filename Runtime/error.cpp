#include "error.h"
#include <iostream>
void error(const LString &string) {

#ifndef _WIN32
	std::cerr << string.toUtf8() << std::endl;
#else
	std::wcerr << string.toWString() << std::endl;
#endif
}


void info(const LString &string) {
#ifndef _WIN32
	std::cout << string.toUtf8() << std::endl;
#else
	std::wcout << string.toWString() << std::endl;
#endif
}


void error(const std::string &txt) {
	std::cout << txt << std::endl;
}
