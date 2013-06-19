#include "cbstring.h"
#include "allegro5/allegro5.h"

extern "C" void CBF_CB_main();

int main() {

	if (al_init()) {

	}
	//Begin execution
	CBF_CB_main();

	return 0;
}
