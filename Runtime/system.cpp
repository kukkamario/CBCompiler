#include <stdio.h>
#include "cbstring.h"

extern "C" void CBF_printI(int i) {
	printf("%i\n", i);
}

extern "C" void CBF_printF(float f) {
	printf("%f\n", f);
}

extern "C" void CBF_printS(String s) {
	printf("%i\n", s.getRef().c_str());
}

extern "C" void CBF_print() {
	printf("\n");
}
