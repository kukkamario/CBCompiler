#include "cbstring.h"
#include <stdint.h>
#include <algorithm>
#include <allegro5/allegro5.h>
#include "error.h"

//#include <boost/lexical_cast.hpp>
std::u32string String::sEmptyString;
std::string String::sEmptyUtf8String;

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
		if (tD->decrease()) {
			delete tD;
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

bool String::empty() const {
	return mData == 0 || mData->mString.empty();
}

void String::detach() {
	if (!mData) return;
	if (mData->mRefCount == 1) {
		mData->mUtf8String.clear();
		return;
	}
	mData->decrease();
	CB_StringData *newData = new CB_StringData(mData->mString);
	mData = newData;
}

const std::string &String::toUtf8() const {
	//printf("toUtf8\n");
	if (empty()) {
		return sEmptyUtf8String;
	}
	if (mData->mUtf8String.empty()) {
		mData->mUtf8String.resize(mData->mString.size() * 4, '\0');
		const char32_t* fromNext;
		uint8_t* toNext;
		ucs4_to_utf8(&mData->mString[0], &mData->mString[mData->mString.size()], fromNext, (uint8_t*)&mData->mUtf8String[0], (uint8_t*)&mData->mUtf8String[mData->mUtf8String.size()], toNext);
		mData->mUtf8String.resize((char*)toNext - &mData->mUtf8String[0]);
	}
	 return mData->mUtf8String;
}

ALLEGRO_USTR *String::toALLEGRO_USTR() const {
	//printf("AL\n");
	if (empty()) return 0; //al_ustr_empty_string();
	const std::string &utf8Str = toUtf8();
	//printf("toALLEGRO_USTR \"%s\"\n", utf8Str.c_str());
	ALLEGRO_USTR *ret = al_ustr_new_from_buffer(utf8Str.c_str(), utf8Str.size());
	return ret;
}

const std::u32string &String::getRef() const {
	if (mData) {
		return mData->mString;
	}
	return sEmptyString;
}

CBString String::returnCBString() {
	if (mData)
		mData->increase();
	return mData;
}

String::operator CBString() {
	return returnCBString();
}


void CB_StringData::increase() {
	mRefCount.increase();
}

bool CB_StringData::decrease() {
	return !mRefCount.decrease();
}
