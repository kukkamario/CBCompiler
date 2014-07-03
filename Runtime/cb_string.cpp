#include "lstring.h"
#include "error.h"
#include <algorithm>

 CBString CBF_str(int i) {
	return LString::number(i);
}

 CBString CBF_str(float f) {
	return LString::number(f);
}

 CBString CBF_left(CBString cbstr, int chars) {
	if (chars <= 0) {
		error(U"Left: Positive number required");
		return 0;
	}
	LString str(cbstr);
	if (chars > (int)str.length()) {
		error(U"Left: Parameter must be less than the length of the string");
		return 0;
	}

	return str.left(chars);
}

 CBString CBF_right(CBString cbstr, int chars) {
	if (chars <= 0) {
		error(U"Right: Positive number required");
		return 0;
	}
	LString str(cbstr);
	if (chars > (int)str.length()) {
		error(U"Right: Parameter must be less than the length of the string");
		return 0;
	}

	return str.right(chars);
}

 int CBF_inStr(CBString cbstr, CBString find) {
	int index = LString(cbstr).indexOf(find);
	if (index == -1) return -1;
	return ++index;
}

 int CBF_inStr(CBString cbstr, CBString find, int start) {
	int index = LString(cbstr).indexOf(find, start - 1);
	if (index == -1) return -1;
	return ++index;
}

CBString CBF_strRemove(CBString cbstr, int begin, int len) {
	LString str(cbstr);
	str.remove(begin - 1, len);
	return str;
}

CBString CBF_trim(CBString cbstr) {
	LString str(cbstr);
	return str.trimmed();
}

CBString CBF_lower(CBString cbstr) {
	LString str(cbstr);
	return str.toLower();
}

CBString CBF_upper(CBString cbstr) {
	LString str(cbstr);
	return str.toUpper();
}

int CBF_len(CBString cbstr) {
	return LString(cbstr).size();
}

 CBEXPORT CBString CB_StringConstruct(char32_t *txt) {
	if (txt) {
		return reinterpret_cast<CBString>(LStringData::createFromBuffer(txt));
	}
	return 0;
}

CBEXPORT void CB_StringDestruct (CBString str) {
	if (str)
		str->decrease();
}

CBEXPORT void CB_StringAssign(CBString *target, CBString s) {
	if (s) {
		s->increase();
	}
	if ((*target)) {
		(*target)->decrease();
	}
	*target = s;
}


CBEXPORT CBString CB_StringAddition(CBString a, CBString b) {
	return LString(a) + LString(b);
}

CBEXPORT void CB_StringRef(CBString a) {
	if (a) a->increase();
}

CBEXPORT int CB_StringToInt(CBString s) {
	return LString(s).toInt();
}

CBEXPORT float CB_StringToFloat(CBString s) {
	return LString(s).toFloat();
}

CBEXPORT CBString CB_FloatToString(float f) {
	return LString::number(f);
}

CBEXPORT CBString CB_IntToString(int i) {
	return LString::number(i);
}

CBEXPORT bool CB_StringEquality(CBString a, CBString b) {
	return LString(a) == LString(b);
}


