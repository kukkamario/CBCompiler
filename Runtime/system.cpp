#include <stdio.h>
#include <time.h>
#include "cbstring.h"
#include <iostream>
#include <locale>
#include <stdint.h>



extern "C" void CBF_printI(int i) {
	printf("%i\n", i);
}

extern "C" void CBF_printF(float f) {
	printf("%f\n", f);
}

extern "C" void CBF_printS(CBString s) {
#ifdef _WIN //sizeof(wchar_t) == 2
	std::cout << "Fix printS\n";
#else //wchar_t == char32_t
	if (s == 0) {
		printf("\n");
		return;
	}
	std::cout << String(s).toUtf8() << "\n";
#endif
}

extern "C" void CBF_print() {
	printf("\n");
}

extern "C" int CBF_timer() {
	return clock() * 1000 / CLOCKS_PER_SEC;
}
