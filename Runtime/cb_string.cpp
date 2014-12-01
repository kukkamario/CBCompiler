#include "LString.h"
#include "error.h"
#include <algorithm>
#include <iomanip>

LString CBF_str(int i) {
	return LString::number(i);
}

LString CBF_str(float f) {
	return LString::number(f);
}

LString CBF_hex(int i) {
	 LString hex = LString::number((unsigned int)i, 16);
	 hex.leftJustify(8, U'0');
	 return hex;
}

LString CBF_chr(int c) {
	char32_t cc[2];
	cc[0] = c;
	cc[1] = 0;
	return LString(cc);
}

uint8_t CBF_Asc(LString c) {
	return c[0] & 0x000000FF;
}

uint16_t CBF_UTF16(LString c) {
	return c[0] & 0x0000FFFF;
}

int CBF_UTF32(LString c) {
	return c[0];
}


LString CBF_left(LString str, int chars) {
	if (chars <= 0) {
		error(U"Left: Positive number required");
		return LString();
	}
	if (chars > (int)str.length()) {
		error(U"Left: Parameter must be less than the length of the string");
		return LString();
	}

	return str.left(chars);
}

 LString CBF_right(LString str, int chars) {
	if (chars <= 0) {
		error(U"Right: Positive number required");
		return LString();
	}
	if (chars > (int)str.length()) {
		error(U"Right: Parameter must be less than the length of the string");
		return LString();
	}

	return str.right(chars);
}

 int CBF_inStr(LString cbstr, LString find) {
	int index = LString(cbstr).indexOf(find);
	if (index == -1) return -1;
	return ++index;
}

 int CBF_inStr(LString cbstr, LString find, int start) {
	int index = LString(cbstr).indexOf(find, start - 1);
	if (index == -1) return -1;
	return ++index;
}

LString CBF_strRemove(LString cbstr, int begin, int len) {
	LString str(cbstr);
	str.remove(begin - 1, len);
	return str;
}

LString CBF_trim(LString cbstr) {
	LString str(cbstr);
	return str.trimmed();
}

LString CBF_lower(LString cbstr) {
	LString str(cbstr);
	return str.toLower();
}

LString CBF_upper(LString cbstr) {
	LString str(cbstr);
	return str.toUpper();
}

int CBF_len(LString cbstr) {
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
	return (LString(a, true) + LString(b, true)).returnData();
}

void CB_StringRef(LStringData *str) {
	if (str)
		str->increase();
}

int CB_StringToInt(LStringData *s) {
	return LString(s, true).toInt();
}

float CB_StringToFloat(LStringData *s) {
	return LString(s, true).toFloat();
}

LStringData *CB_FloatToString(float f) {
	return LString::number(f).returnData();
}

LStringData *CB_IntToString(int i) {
	return LString::number(i).returnData();
}

bool CB_StringEquality(LStringData *a, LStringData *b) {
	return LString(a, true) == LString(b, true);
}


