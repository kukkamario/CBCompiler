#include "lstring.h"
#include <algorithm>

extern "C" CBString CBF_CB_StringConstruct (char32_t *txt) {
	if (txt) {
		return reinterpret_cast<CBString>(LStringData::createFromBuffer(txt));
	}
	return 0;
}

extern "C" void CBF_CB_StringDestruct (CBString str) {
	if (str)
		str->decrease();
}

extern "C" void CBF_CB_StringAssign(CBString *target, CBString s) {
	if (s) {
		s->increase();
	}
	if ((*target)) {
		(*target)->decrease();
	}
	*target = s;
}


extern "C" CBString CBF_CB_StringAddition(CBString a, CBString b) {
	return LString(a) + LString(b);
}

extern "C" void CBF_CB_StringRef(CBString a) {
	if (a) a->increase();
}

extern "C" int CBF_CB_StringToInt(CBString s) {
	return LString(s).toInt();
}

extern "C" float CBF_CB_StringToFloat(CBString s) {
	return LString(s).toFloat();
}

extern "C" CBString CBF_CB_FloatToString(float f) {
	return LString::number(f);
}

extern "C" CBString CBF_CB_IntToString(int i) {
	return LString::number(i);
}

extern "C" bool CBF_CB_StringEquality(CBString a, CBString b) {
	return LString(a) == LString(b);
}


