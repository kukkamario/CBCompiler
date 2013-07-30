#include "error.h"
#include <iostream>
void error(const LString &string) {
	std::cerr << string.toUtf8() << '\n';
}


void info(const LString &string) {
	std::cout << string.toUtf8() << '\n';
}
