#include "cb_class.h"
#include "systeminterface.h"

void CBF_delete(CB_GenericClass *s) {
	delete[] reinterpret_cast<char*>(s);
}
