#include "error.h"
#include <iostream>
void error(const String &string) {
	std::cerr << string.toUtf8() << '\n';
}


void info(const String &string) {
	std::cout << string.toUtf8() << '\n';
}
