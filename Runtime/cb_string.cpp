#include "cbstring.h"
#include <algorithm>

extern "C" CBString CBF_CB_StringConstruct (const char32_t *txt) {
	if (txt) {
		return new CB_StringData(txt);
	}
	return 0;
}

extern "C" void CBF_CB_StringDestruct (CBString str) {
	if (str)
		if (str->decrease()) delete str;
}

extern "C" void CBF_CB_StringAssign(CBString *target, CBString s) {
	if (s) {
		s->increase();
	}
	if ((*target)) {
		if((*target)->decrease()) {
			delete *target;
		}
	}
	*target = s;
}


extern "C" CBString CBF_CB_StringAddition(CBString a, CBString b) {
	if (!a) {
		if (b) b->increase();
		return b;
	}
	if (!b) {
		if (a) a->increase();
		return a;
	}
	return new CB_StringData(a->mString + b->mString);
}

extern "C" void CBF_CB_StringRef(CBString a) {
	if (a) a->mRefCount.increase();
}

extern "C" int CBF_CB_StringToInt(CBString s) {
	/*if (!s) return 0;
	try {
		return boost::lexical_cast<int>(s->mString);
	}catch(boost::bad_lexical_cast &) {
		return 0;
	}*/
	return 0;
}

extern "C" float CBF_CB_StringToFloat(CBString s) {
	/*if (!s) return 0;
	try {
		return boost::lexical_cast<float>(s->mString);
	}catch(boost::bad_lexical_cast &) {
		return 0;
	}*/
	return 0;
}

extern "C" CBString CBF_CB_FloatToString(float f) {
	//return new CB_StringData(boost::lexical_cast<std::u32string>(f));
	//std::basic_ostringstream<char32_t> out;
	//out << f;
	//return new CB_StringData(out.str());
	return 0;
}

extern "C" CBString CBF_CB_IntToString(int i) {
	//return new CB_StringData(boost::lexical_cast<std::u32string>(i));
	if (i == 0) {
		return new CB_StringData(U"0");
	}
	std::u32string str;
	if (i < 0) {
		str += U'-';
		i = -i;
	}
	while (i) {
		char32_t c = U'0' + i % 10;
		i /= 10;
		str += c;
	}
	std::reverse(str.begin(), str.end());
	return new CB_StringData(str);
}

extern "C" bool CBF_CB_StringEquality(CBString a, CBString b) {
	if (a == b) return true;
	if (a == 0 || a->mString.empty()) {
		if (b == 0 || b->mString.empty()) {
			return true;
		}
		return false;
	}
	if (b == 0 || b->mString.empty()) return false;
	return a->mString == b->mString;
}


