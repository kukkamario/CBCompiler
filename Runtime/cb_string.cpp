#include "lstring.h"
#include "error.h"
#include <algorithm>

CBEXPORT CBString CBF_strI(int i) {
	return LString::number(i);
}

CBEXPORT CBString CBF_strF(float f) {
	return LString::number(f);
}

CBEXPORT CBString CBF_leftSI(CBString cbstr, int chars) {
	if (chars <= 0) {
		error(U"Left: Positive number required");
		return 0;
	}
	LString str(cbstr);
	if (chars > str.length()) {
		error(U"Left: Parameter must be less than the length of the string");
		return 0;
	}

	return str.left(chars);
}

CBEXPORT CBString CBF_rightSI(CBString cbstr, int chars) {
	if (chars <= 0) {
		error(U"Right: Positive number required");
		return 0;
	}
	LString str(cbstr);
	if (chars > str.length()) {
		error(U"Right: Parameter must be less than the length of the string");
		return 0;
	}

	return str.right(chars);
}


CBEXPORT CBString CBF_CB_StringConstruct (char32_t *txt) {
	if (txt) {
		return reinterpret_cast<CBString>(LStringData::createFromBuffer(txt));
	}
	return 0;
}

CBEXPORT void CBF_CB_StringDestruct (CBString str) {
	if (str)
		str->decrease();
}

CBEXPORT void CBF_CB_StringAssign(CBString *target, CBString s) {
	if (s) {
		s->increase();
	}
	if ((*target)) {
		(*target)->decrease();
	}
	*target = s;
}


CBEXPORT CBString CBF_CB_StringAddition(CBString a, CBString b) {
	return LString(a) + LString(b);
}

CBEXPORT void CBF_CB_StringRef(CBString a) {
	if (a) a->increase();
}

CBEXPORT int CBF_CB_StringToInt(CBString s) {
	return LString(s).toInt();
}

CBEXPORT float CBF_CB_StringToFloat(CBString s) {
	return LString(s).toFloat();
}

CBEXPORT CBString CBF_CB_FloatToString(float f) {
	return LString::number(f);
}

CBEXPORT CBString CBF_CB_IntToString(int i) {
	return LString::number(i);
}

CBEXPORT bool CBF_CB_StringEquality(CBString a, CBString b) {
	return LString(a) == LString(b);
}


