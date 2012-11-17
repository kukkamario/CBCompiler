#include "cbstring.h"
std::string CB_String::staticEmptyString;

extern "C" CB_String CBF_CB_StringConstruct (const char *txt) {
	return CB_String(txt);
}

extern "C" void CBF_CB_StringDestruct (CB_String *str) {
	str->~CB_String();
}



const std::string &CB_String::getRef() const {
	if (mData) {
		return mData->mString;
	}
	return staticEmptyString;
}
