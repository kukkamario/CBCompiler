#include "cb_struct.h"
#include "systeminterface.h"

void CBF_delete(CB_GenericStruct *s) {
	delete[] reinterpret_cast<char*>(s);
}
