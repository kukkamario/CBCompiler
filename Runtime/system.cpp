#include <stdio.h>
#include <time.h>
#include "cbstring.h"

extern "C" void CBF_printI(int i) {
	printf("%i\n", i);
}

extern "C" void CBF_printF(float f) {
	printf("%f\n", f);
}

extern "C" void CBF_printS(CBString s) {
	printf("%s\n", String(s).getRef().c_str());
}

extern "C" void CBF_print() {
	printf("\n");
}

extern "C" int CBF_timer() {
	return clock() * 1000 / CLOCKS_PER_SEC;
}
