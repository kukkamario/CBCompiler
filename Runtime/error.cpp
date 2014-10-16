#include "error.h"
#include <iostream>
#include "systeminterface.h"
#ifdef _WIN32
	#include <windows.h>
#endif
void error(const LString &string) {
	sys::writeToError(string + U'\n');
}


void info(const LString &string) {
	sys::writeToOutput(string + U'\n');
}


void error(const std::string &txt) {
	std::cout << txt << std::endl;
}
