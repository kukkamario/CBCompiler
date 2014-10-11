#include "lstring.h"
#include "error.h"
#include <algorithm>
#include <iomanip>

 CBString CBF_str(int i) {
	return LString::number(i);
}

 CBString CBF_str(float f) {
	return LString::number(f);
}

 CBString CBF_hex(int i) {
	 LString hex = LString::number((unsigned int)i, 16);
	 hex.leftJustify(8, U'0');
	 return hex;
}

CBString CBF_chr(int c) {
	char32_t cc[2];
	cc[0] = c;
	cc[1] = c;
	return LString(cc);
}

 CBString CBF_left(CBString cbstr, int chars) {
	if (chars <= 0) {
		error(U"Left: Positive number required");
		return LString();
	}
	LString str(cbstr);
	if (chars > (int)str.length()) {
		error(U"Left: Parameter must be less than the length of the string");
		return LString();
	}

	return str.left(chars);
}

 CBString CBF_right(CBString cbstr, int chars) {
	if (chars <= 0) {
		error(U"Right: Positive number required");
		return LString();
	}
	LString str(cbstr);
	if (chars > (int)str.length()) {
		error(U"Right: Parameter must be less than the length of the string");
		return LString();
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

LStringData *CB_StringConstruct(char32_t *txt) {
	if (txt) {
		return LStringData::createFromBuffer(txt);
	}
	return 0;
}

void CB_StringDestruct (LStringData *str) {
	if (str)
		str->decrease();
}

void CB_StringAssign(LStringData **target, LStringData *s) {
	if (s) {
		s->increase();
	}
	if (*target)
		(*target)->decrease();
	*target = s;
}


LStringData *CB_StringAddition(LStringData * a, LStringData * b) {
	return (LString(a) + LString(b)).returnData();
}

void CB_StringRef(LStringData *str) {
	if (str)
		str->increase();
}

int CB_StringToInt(LStringData *s) {
	return LString(s).toInt();
}

float CB_StringToFloat(LStringData *s) {
	return LString(s).toFloat();
}

LStringData *CB_FloatToString(float f) {
	return LString::number(f).returnData();
}

LStringData *CB_IntToString(int i) {
	return LString::number(i).returnData();
}

bool CB_StringEquality(LStringData *a, LStringData *b) {
	return LString(a) == LString(b);
}


