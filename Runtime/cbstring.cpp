#include "cbstring.h"
#include <stdint.h>
#include <algorithm>
//#include <boost/lexical_cast.hpp>
std::u32string String::staticEmptyString;

#ifndef _WIN32
typedef u_int32_t uint32_t;
typedef u_int8_t uint8_t;
#endif

static void ucs4_to_utf8(const char32_t* frm, const char32_t* frm_end, const char32_t*& frm_nxt,
			 uint8_t* to, uint8_t* to_end, uint8_t*& to_nxt,
			 unsigned long Maxcode = 0x10FFFF)
{
	frm_nxt = frm;
	to_nxt = to;
	/*if (mode & generate_header)
	{
		if (to_end-to_nxt < 3)
			return codecvt_base::partial;
		*to_nxt++ = static_cast<uint8_t>(0xEF);
		*to_nxt++ = static_cast<uint8_t>(0xBB);
		*to_nxt++ = static_cast<uint8_t>(0xBF);
	}*/
	for (; frm_nxt < frm_end; ++frm_nxt)
	{
		uint32_t wc = *frm_nxt;
		if ((wc & 0xFFFFF800) == 0x00D800 || wc > Maxcode)
			return;
		if (wc < 0x000080)
		{
			if (to_end-to_nxt < 1)
				return;
			*to_nxt++ = static_cast<uint8_t>(wc);
		}
		else if (wc < 0x000800)
		{
			if (to_end-to_nxt < 2)
				return;
			*to_nxt++ = static_cast<uint8_t>(0xC0 | (wc >> 6));
			*to_nxt++ = static_cast<uint8_t>(0x80 | (wc & 0x03F));
		}
		else if (wc < 0x010000)
		{
			if (to_end-to_nxt < 3)
				return;
			*to_nxt++ = static_cast<uint8_t>(0xE0 |  (wc >> 12));
			*to_nxt++ = static_cast<uint8_t>(0x80 | ((wc & 0x0FC0) >> 6));
			*to_nxt++ = static_cast<uint8_t>(0x80 |  (wc & 0x003F));
		}
		else // if (wc < 0x110000)
		{
			if (to_end-to_nxt < 4)
				return;
			*to_nxt++ = static_cast<uint8_t>(0xF0 |  (wc >> 18));
			*to_nxt++ = static_cast<uint8_t>(0x80 | ((wc & 0x03F000) >> 12));
			*to_nxt++ = static_cast<uint8_t>(0x80 | ((wc & 0x000FC0) >> 6));
			*to_nxt++ = static_cast<uint8_t>(0x80 |  (wc & 0x00003F));
		}
	}
	return;
}

String::String(CB_StringData *d) :
	mData(d) {
	if (mData) mData->increase();
}

String::String(const char32_t *txt) :
	mData(0) {
	if (txt && txt[0] != 0) {
		mData = new CB_StringData(txt);
	}
}

String::String(const std::u32string &s) :
	mData(0) {
	if (!s.empty()) {
		mData = new CB_StringData(s);
	}
}

String::String(const String &o) : mData(o.mData) {
	if (mData) mData->increase();
}

String & String::operator=(const String &o) {
	CB_StringData *tD = this->mData;
	this->mData = o.mData;
	if (this->mData) this->mData->increase();
	if (tD) {
		if (mData->decrease()) {
			delete mData;
		}

	}
	return *this;
}

String String::operator +(const String &a) {
	if (this->mData == 0) return a;
	if (a.mData == 0) return *this;
	return this->mData->mString + a.mData->mString;
}

bool String::operator ==(const String &a) {
	if (this->mData == 0 || this->mData->mString.empty()) {
		if (a.mData == 0 || a.mData->mString.empty()) {
			return true;
		}
		return false;
	}
	if (a.mData == 0 || a.mData->mString.empty()) return false;
	return a.mData->mString == this->mData->mString;
}


String::~String() {
	if (mData) {
		if (mData->decrease()) { //No more references
			delete mData;
		}
	}
}

int String::size() const {
	if (mData) {
		return mData->mString.size();
	}
	return 0;
}

std::string String::toUtf8() const {
	if (mData == 0 || mData->mString.empty()) {
		return std::string();
	}
	std::string utf8_str(mData->mString.size() * 4, '\0');
	const char32_t* fromNext;
	uint8_t* toNext;
	ucs4_to_utf8(&mData->mString[0], &mData->mString[mData->mString.size()], fromNext, (uint8_t*)&utf8_str[0], (uint8_t*)&utf8_str[utf8_str.size()], toNext);
	utf8_str.resize((char*)toNext - &utf8_str[0]);
	return utf8_str;
}

const std::u32string &String::getRef() const {
	if (mData) {
		return mData->mString;
	}
	return staticEmptyString;
}

CBString String::returnCBString() {
	if (mData)
		mData->increase();
	return mData;
}


void CB_StringData::increase() {
	mRefCount.increase();
}

bool CB_StringData::decrease() {
	return !mRefCount.decrease();
}



//CBF

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
	if ((*target)) {
		if((*target)->decrease()) {
			delete *target;
		}
	}
	if (s) {
		s->increase();
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
		str += '-';
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


