#include "lstring.h"
#include <stddef.h>
#include <string.h>
#include <algorithm>
#include <stdio.h>
#include <locale>
#include "error.h"

std::string LString::sNullStdString;


//LStringData
//-------------------------------------------------------------------------


LStringData *LStringData::create(size_t size) {
	assert(size > 0);
	char *buffer = new char[sizeof(LStringData) + size * sizeof(LChar)];
	LStringData *ret = reinterpret_cast<LStringData*>(buffer);
	ret->mRefCount = 1;
	ret->mSize = 0;
	ret->mCapacity = size;
	ret->mUtf8String = 0;
	ret->mOffset = sizeof(LStringData);
	return ret;
}

LStringData *LStringData::create(const LChar *text) {
	size_t len = 0;
	while(text[len]) {
		len++;
	}

	LStringData *d = create(len + 1);
	d->mSize = len;
	memcpy(d->begin(), text, (len + 1) * sizeof(LChar));
	return d;
}

LStringData *LStringData::create(const LChar *text, size_t len) {
	LStringData *d = create(len + 1);
	d->mSize = len;
	memcpy(d->begin(), text, len * sizeof(LChar));
	d->begin()[len] = 0; //null
	return d;
}

LStringData *LStringData::createFromBuffer(const LChar *buffer) {
	size_t len = 0;
	while(buffer[len]) {
		len++;
	}
	return createFromBuffer(buffer, len, len + 1);
}

LStringData *LStringData::createFromBuffer(const LChar *buffer, size_t stringLength, size_t bufferSize) {
	char *buf = new char[sizeof(LStringData)];
	LStringData *ret = reinterpret_cast<LStringData*>(buf);
	ret->mSize = stringLength;
	ret->mCapacity = bufferSize;
	ret->mUtf8String = 0;
	ret->mRefCount = 1;
	ret->mOffset = reinterpret_cast<const char*>(buffer) - buf;
	return ret;
}

LStringData *LStringData::copy(LStringData *o) {
	if (o->isStaticData()) {
		char *buffer = new char[sizeof(LStringData)];
		LStringData *ret = reinterpret_cast<LStringData*>(buffer);
		memcpy(o, ret, sizeof(LStringData));
		ret->mUtf8String = 0;
		ret->mRefCount = 1;
		return ret;
	}
	char *buffer = new char[sizeof(LStringData) + o->mCapacity * sizeof(LChar)];
	LStringData *ret = reinterpret_cast<LStringData*>(buffer);
	memcpy(ret, o, sizeof(LStringData) + o->mCapacity * sizeof(LChar));
	ret->mUtf8String = 0;
	ret->mRefCount = 1;
	return ret;
}

void LStringData::destruct(LStringData *d) {
	if (d->mUtf8String) delete d->mUtf8String;
	delete [] reinterpret_cast<char*>(d);
}

void LStringData::increase() {
	atomicIncrease(mRefCount);
}

bool LStringData::decrease() {
	if (atomicDecrease(mRefCount)) {
		atomicThreadFenceAcquire();
		LStringData::destruct(this);
		return true;
	}
	return false;
}


bool LStringData::isStaticData() const {
	return mOffset != sizeof(LStringData);
}

LChar *LStringData::begin() {
	return reinterpret_cast<LChar*>(reinterpret_cast<char*>(this) + this->mOffset);
}

LChar *LStringData::end() {
	return reinterpret_cast<LChar*>(reinterpret_cast<char*>(this) + this->mOffset) + this->mSize;
}

const LChar *LStringData::begin() const {
	return reinterpret_cast<const LChar*>(reinterpret_cast<const char*>(this) + this->mOffset);
}

const LChar *LStringData::end() const {
	return reinterpret_cast<const LChar*>(reinterpret_cast<const char*>(this) + this->mOffset) + this->mSize;
}


//LString
//-------------------------------------------------------------------------

LString::LString() : mData() {
}

LString::LString(const LChar *str) : mData(LStringData::create(str)) { }

LString::LString(const LChar *str, size_t len) : mData(LStringData::create(str, len)) { }

LString::LString(LString::ConstIterator begin, LString::ConstIterator end) : mData(LStringData::create(begin, (size_t)(end - begin))) { }

LString::LString(const LString &o) : mData(o.mData) { }

LString::LString(CBString cbString) : mData(cbString) {
	if (cbString) cbString->increase();
}

LString::LString(LStringData *data) : mData(data) { }

size_t LString::nextSize() const {
	//TODO: Improve
	return this->size() * 2;
}

LString::~LString() {
	// ~LSharedStringDataPointer will destroy everything
}

LString LString::fromBuffer(const LChar *buffer, size_t stringLength, size_t bufferSize) {
	assert(stringLength < bufferSize);
	return LString(LStringData::createFromBuffer(buffer, stringLength, bufferSize));
}


LString LString::fromUtf8(const std::string &s) {
	if (s.size() == 0) return LString();
	const uint8_t *begin = reinterpret_cast<const uint8_t*>(s.c_str());
	const uint8_t *end = begin + s.size();
	LString ret;
	ret.reserve(s.size());

	LChar *destBegin = ret.begin();
	LChar *destEnd = ret.end();
	bool conversionSucceeded = utf8ToUtf32(&begin, end, &destBegin, destEnd);
	assert(conversionSucceeded && "Failed conversion utf8 -> utf32");

	ret.resize(destBegin - ret.begin());
	return ret;
}

LString LString::fromAscii(const std::string &s) {
	LString ret;
	ret.resize(s.size());
	LString::Iterator out = ret.begin();
	for (std::string::const_iterator i = s.begin(); i != s.end(); ++i) {
		*out = *i;
	}
	return ret;
}

LString LString::fromBuffer(const LChar *buffer) {
	return LString(LStringData::createFromBuffer(buffer));
}

LString LString::number(int i, int base) {
	assert(base >= 2 && base <= 16);
	if (i == 0) return LString(U"0");
	static const char32_t nums[] = {U'0', U'1', U'2', U'3', U'4', U'5', U'6', U'7', U'8', U'9', U'A', U'B', U'C', U'D', U'E', U'F'};
	LStringData *data = LStringData::create(6 + 64 / base);
	LChar *str = data->begin();
	int size = 0;
	if (i < 0) {
		*str = U'-';
		i = -i;
		str++;
		++size;
	}

	int shifter = i;
	do{ //Move to where representation ends
		++str;
		shifter = shifter/base;
	}while(shifter);

	*str = 0;
	while (i) {
		*(--str) = nums[i % base];
		++size;
		i /= base;
	}

	data->mSize = size;
	return LString(data);
}

LString LString::number(unsigned int i, int b) {
	assert(b >= 2 && b <= 16);
	unsigned int base = b;
	if (i == 0) return LString(U"0");
	static const char32_t nums[] = {U'0', U'1', U'2', U'3', U'4', U'5', U'6', U'7', U'8', U'9', U'A', U'B', U'C', U'D', U'E', U'F'};
	LStringData *data = LStringData::create(6 + 64 / base);
	LChar *str = data->begin();
	int size = 0;
	unsigned int shifter = i;
	do{ //Move to where representation ends
		++str;
		shifter = shifter/base;
	}while(shifter);

	*str = 0;
	while (i) {
		*(--str) = nums[i % base];
		++size;
		i /= base;
	}

	data->mSize = size;
	return LString(data);
}

LString LString::number(float f) {
	const size_t bufferSize = 12;
	LStringData *data = LStringData::create(bufferSize);

#ifndef _WIN32
	assert(sizeof(wchar_t) == sizeof(char32_t));
	data->mSize = swprintf(reinterpret_cast<wchar_t*>(data->begin()), bufferSize, L"%g", f);
	return LString(data);
#else
	char charBuf[bufferSize];
	data->mSize = snprintf(charBuf, bufferSize, "%g", f);
	for (size_t i = 0; i < data->mSize; ++i) {
		data->begin()[i] = static_cast<char32_t>(charBuf[i]);
	}
	data->begin()[data->mSize] = 0;
	return LString(data);
#endif
}

LString &LString::operator =(const LString &o) {
	mData = o.mData;
	return *this;
}

bool LString::operator ==(const LString &o) const {
	LString::ConstIterator thisI = this->cbegin();
	LString::ConstIterator oI = o.cbegin();
	for (;thisI != this->cend() && oI != o.cend(); thisI++, oI++) {
		if (*thisI != *oI) return false;
	}
	return true;
}

bool LString::operator !=(const LString &o) const {
	LString::ConstIterator thisI = this->cbegin();
	LString::ConstIterator oI = o.cbegin();
	for (;thisI != this->cend() && oI != o.cend(); thisI++, oI++) {
		if (*thisI == *oI) return false;
	}
	return true;
}

bool LString::operator >(const LString &o) const {
	LString::ConstIterator thisI = this->cbegin();
	LString::ConstIterator oI = o.cbegin();
	for (;thisI != this->cend() && oI != o.cend(); thisI++, oI++) {
		if (*thisI == *oI) continue;
		if (*thisI > *oI) return true;
		return false;
	}
	return false;
}

bool LString::operator >=(const LString &o) const {
	LString::ConstIterator thisI = this->cbegin();
	LString::ConstIterator oI = o.cbegin();
	for (;thisI != this->cend() && oI != o.cend(); thisI++, oI++) {
		if (*thisI == *oI) continue;
		if (*thisI > *oI) return true;
		return false;
	}
	return true;
}

bool LString::operator <(const LString &o) const {
	LString::ConstIterator thisI = this->cbegin();
	LString::ConstIterator oI = o.cbegin();
	for (;thisI != this->cend() && oI != o.cend(); thisI++, oI++) {
		if (*thisI == *oI) continue;
		if (*thisI < *oI) return true;
		return false;
	}
	return false;
}

bool LString::operator <=(const LString &o) const {
	LString::ConstIterator thisI = this->cbegin();
	LString::ConstIterator oI = o.cbegin();
	for (;thisI != this->cend() && oI != o.cend(); thisI++, oI++) {
		if (*thisI == *oI) continue;
		if (*thisI < *oI) return true;
		return false;
	}
	return true;
}

LString LString::operator +(const LString &o) const {
	if (this->isEmpty()) return o;
	if (o.isEmpty()) return *this;

	LStringData *newStr = LStringData::create(this->length() + o.length() + 1);
	std::copy(this->mData->begin(), this->mData->end(), newStr->begin());
	std::copy(o.mData->begin(), o.mData->end(), newStr->begin() + this->length());

	newStr->mSize = this->length() + o.length();
	return LString(newStr);
}

LString &LString::operator +=(const LString &o) {
	if (o.isEmpty()) return *this;
	if (this->isEmpty()) {
		*this = o;
		return *this;
	}
	if (this->length() + o.length() > this->size()) {
		size_t next = nextSize();
		if (next < this->length() + o.length()) {
			reserve(this->length() + o.length());
		}
		else {
			reserve(next);
		}
	}
	std::copy(o.mData->begin(), o.mData->end(), this->mData->begin() + this->length());
	return *this;
}

LString &LString::operator +=(LChar c) {
	if (this->length() + 1 > this->capacity()) {
		reserve(nextSize());
	}
	this->mData->begin()[this->mData->mSize] = c;
	++this->mData->mSize;
	return *this;
}

LChar &LString::operator [](int i) {
	assert(i < (int)length());
	return this->mData->begin()[i];
}

const LChar &LString::operator [](int i) const {
	assert(i < (int)length());
	return this->mData->begin()[i];
}

LString::operator CBString() const {
	if (isNull()) return 0;
	this->mData.unsafePointer()->increase();
	return reinterpret_cast<CBString>(this->mData.unsafePointer());
}

LString LString::substr(int start, int len) const {
	ConstIterator b = cbegin() + start;
	ConstIterator e = b + len;
	assert(isValidIterator(b));
	assert(isValidIterator(e));
	return LString(b, e);
}

LString LString::left(int chars) const {
	assert(isEmpty() && "String is empty. Invalid Left function call");
	assert(chars <= (int)length() && chars > 0 && "Invalid Left function call");
	return LString(begin(), chars);
}

LString LString::right(int chars) const {
	assert(isEmpty() && "String is empty. Invalid Left function call");
	assert(chars <= (int)length() && chars > 0 && "Invalid Left function call");
	return LString(begin() + (length() - chars), (length() - chars));
}

LString LString::trimmed() const {
	LString::ConstIterator start = end();
	for (LString::ConstIterator i = begin(); i != end(); i++) {
		if (!isWhitespace(*i)) {
			start = i;
			break;
		}
	}
	if (start == end()) return *this;

	LString::ConstIterator e = end();
	for (LString::ConstIterator i = end() - 1;i != begin(); --i) {
		if (!isWhitespace(*i)) {
			e = i + 1;
			break;
		}
	}

	return LString(start, e);
}

LString LString::rightJustified(size_t width, LChar fill, bool truncate) const {
	LString ret(*this);
	ret.rightJustify(width, fill, truncate);
	return ret;
}

LString LString::leftJustified(size_t width, LChar fill, bool truncate) const {
	LString ret(*this);
	ret.leftJustify(width, fill, truncate);
	return ret;
}

LString LString::toUpper() const {
	LString ret;
	ret.resize(size());
	LString::Iterator out = ret.begin();
	for (LString::ConstIterator i = begin(); i != end(); i++) {
		char32_t ch = *i;
		ch = toUpper(ch);
		*out = ch;
		++out;
	}
	return ret;
}

LString LString::toLower() const {
	LString ret;
	ret.resize(size());
	LString::Iterator out = ret.begin();
	for (LString::ConstIterator i = begin(); i != end(); i++) {
		char32_t ch = *i;
		ch = toLower(ch);
		*out = ch;
		++out;
	}
	return ret;
}

void LString::rightJustify(size_t width, LChar fill, bool truncate) {
	if (size() >= width) {
		if (truncate) {
			resize(width);
		}
		return;
	}
	size_t oldSize = size();
	resize(width);
	Iterator i = begin() + oldSize;
	Iterator e = begin() + width;
	for (; i != e; i++) {
		*i = fill;
	}
}

void LString::leftJustify(size_t width, LChar fill, bool truncate) {
	if (size() >= width) {
		if (truncate) {
			*this = right(width);
		}
		return;
	}
	size_t oldSize = size();
	resize(width);
	if (oldSize * 2 <= width)
		std::copy(begin(), begin() + oldSize, begin() + (width - oldSize));
	else
		std::reverse_copy(begin(), begin() + oldSize, begin() + (width - oldSize));
	Iterator i = begin();
	Iterator e = begin() + (width - oldSize);
	for (; i != e; i++) {
		*i = fill;
	}
}

LString::Iterator LString::find(LChar c) {
	return find(c, begin());
}


LString::Iterator LString::find(LChar c, LString::Iterator start) {
	for (Iterator i = start; i != end(); i++) {
		if (*i == c) {
			return i;
		}
	}
	return end();
}

LString::ConstIterator LString::find(LChar c) const {
	return find(c, cbegin());
}

LString::ConstIterator LString::find(LChar c, LString::ConstIterator start) const {
	for (ConstIterator i = start; i != cend(); i++) {
		if (*i == c) {
			return i;
		}
	}
	return cend();
}

LString::ConstIterator LString::find(const LString &str) const {
	return find(str, cbegin());
}

LString::Iterator LString::find(const LString &str) {
	return find(str, begin());
}

LString::ConstIterator LString::find(const LString &str, LString::ConstIterator start) const {
	ConstIterator si = str.cbegin();
	ConstIterator index;
	for (ConstIterator i = start; i != cend(); i++) {
		if (*si == *i) {
			if (si == str.cbegin()) {
				index = i;
			}
			si++;
			if (si == str.cend()) {
				return index;
			}
			continue;
		}

		si = str.cbegin();
	}
	return cend();
}

LString::Iterator LString::find(const LString &str, LString::Iterator start) {
	ConstIterator si = str.cbegin();
	Iterator index;
	for (Iterator i = start; i != cend(); i++) {
		if (*si == *i) {
			if (si == str.cbegin()) {
				index = i;
			}
			si++;
			if (si == str.cend()) {
				return index;
			}
			continue;
		}

		si = str.cbegin();
	}
	return end();
}

int LString::indexOf(LChar c) const {
	int index = 0;
	for (ConstIterator i = cbegin(); i != cend(); ++i, index++) {
		if (*i == c) return index;
	}
	return -1;
}

int LString::indexOf(LChar c, int start) const {
	int index = start;
	for (ConstIterator i = at(start); i != cend(); ++i, index++) {
		if (*i == c) return index;
	}
	return -1;
}

int LString::indexOf(const LString &str) const {
	if (str.length() > this->length()) return -1;
	if (str.length() == 1) return indexOf(str[0]);

	ConstIterator si = str.cbegin();
	int indexCounter = 0;
	int index = 0;
	for (ConstIterator i = cbegin(); i != cend(); i++, indexCounter++) {
		if (*si == *i) {
			if (si == str.cbegin()) {
				index = indexCounter;
			}
			si++;
			if (si == str.cend()) {
				return index;
			}
			continue;
		}

		si = str.cbegin();
	}
	return -1;
}

int LString::indexOf(const LString &str, int start) const {
	if (str.length() > this->length()) return -1;
	if (str.length() == 1) return indexOf(str[0], start);

	ConstIterator si = str.cbegin();
	int indexCounter = start;
	int index = 0;
	for (ConstIterator i = at(start); i != cend(); i++, indexCounter++) {
		if (*si == *i) {
			if (si == str.cbegin()) {
				index = indexCounter;
			}
			si++;
			if (si == str.cend()) {
				return index;
			}
			continue;
		}

		si = str.cbegin();
	}
	return -1;
}

void LString::clear() {
	mData = 0;
}

void LString::remove(int start, int len) {
	assert(start >= 0 && len > 0 && start + len <= (int)length());
	detach();
	LStringData *d = mData.unsafePointer();
	if (start + len == (int)length()) {
		d->mSize -= len;
		d->begin()[start + len] = 0;
	}
	else {
		memmove(d->begin() + start + len, d->begin() + start, d->mSize - (start + len));
		d->mSize -= len;
		d->begin()[start + len] = 0;
	}
}



int LString::toInt(bool *success) const {
	if (isEmpty()) {
		if (success) *success = false;
		return 0;
	}

	int val = 0;
	LString::ConstIterator i;
	for (i = this->cbegin(); i != this->cend(); i++) {
		if (*i > U'9' || *i < U'0') break;
		val *= 10;
		val += *i - U'0';
	}
	if (i == this->cbegin() && success) *success = false;
	return val;

	/*if (sizeof(wchar_t) == sizeof(char32_t)) {
		int val;
		if (swscanf(reinterpret_cast<const wchar_t*>(mData->mData), L"%i", &val) != 1) {
			if (success) *success = false;
			return 0;
		}
		return val;
	}*/
}

int LString::toFloat(bool *success) const {
	if (isEmpty()) {
		if (success) *success = false;
		return 0;
	}

#ifndef _WIN32
	assert(sizeof(wchar_t) == sizeof(char32_t));
		float val = 0.0f;
		if (swscanf(reinterpret_cast<const wchar_t*>(mData->begin()), L"%f", &val) != 1) {
			if (success) *success = false;
			return 0;
		}
		return val;
#else
	float val = 0.0f;
	if (sscanf(toUtf8().c_str(), "%f", &val) != 1) {
		if (success) *success = false;
		return 0;
	}
	return val;
#endif
}

LString::Iterator LString::begin() {
	return isNull() ? 0 : mData->begin();
}

LString::ConstIterator LString::cbegin() const {
	return isNull() ? 0 : mData->begin();
}

LString::Iterator LString::end() {
	return isNull() ? 0 : mData->end();
}

LString::ConstIterator LString::cend() const {
	return isNull() ? 0 : mData->end();
}

LString::Iterator LString::at(int i) {
	assert(i >= 0 && i < (int)length());
	return mData->begin() + i;
}

LString::ConstIterator LString::at(int i) const {
	assert(i >= 0 && i < (int)length());
	return mData->begin() + i;
}

bool LString::isNull() const {
	return mData.isNull();
}

bool LString::isEmpty() const {
	return isNull() || mData->mSize == 0;
}

size_t LString::length() const {
	return isNull() ? 0 : mData->mSize;
}

size_t LString::capacity() const {
	return isNull() ? 0 : mData->mCapacity;
}

void LString::reserve(size_t size) {
	if (size <= capacity()) return;
	if (isNull()) {
		mData = LStringData::create(size + 1);
		return;
	}
	else {
		LStringData *newData = LStringData::create(size + 1);
		memcpy(newData->begin(), this->mData->begin(), this->mData->mSize * sizeof(LChar));
		this->mData = newData;
	}
}

void LString::resize(size_t size) {
	if (this->size() == size) return;
	if (isNull()) {
		mData = LStringData::create(size + 1);
		mData->mSize = size;
		return;
	}
	else if (size <= this->capacity()) {
		mData->mSize = size;
		return;
	}
	else {
		LStringData *oldData = this->mData.unsafePointer();
		LStringData *newData = LStringData::create(size + 1);
		memcpy(newData->begin(), oldData->begin(), oldData->mSize * sizeof(LChar));
		newData->mSize = size;
		this->mData = newData;
	}
}

std::u32string LString::toU32String() const {
	return std::u32string(cbegin(), cend());
}

std::wstring LString::toWString() const {
#ifndef _WIN32
	assert(sizeof(wchar_t) == sizeof(char32_t));
	return std::wstring(cbegin(), cend());
#else
	wchar_t buffer[size() * 2];
	size_t len = mbstowcs(buffer, toUtf8().c_str(), size());
	return std::wstring(buffer, len);
#endif
}

const std::string &LString::toUtf8() const {
	if (isEmpty()) return sNullStdString;
	if (mData->mUtf8String) return *mData->mUtf8String;

	std::string *utf8 = new std::string(mData->mSize * 4, '\0');
	const LChar* fromNext;
	uint8_t* toNext;
	bool conversionValid = ucs4ToUtf8(cbegin(), cend(), fromNext, (uint8_t*)&(*utf8)[0], (uint8_t*)&(*utf8)[utf8->size()], toNext);
	assert(conversionValid);
	utf8->resize((char*)toNext - &(*utf8)[0]);
	mData->mUtf8String = utf8;
	return *mData->mUtf8String;
}

ALLEGRO_USTR *LString::toAllegroUStr() const {
	if (isEmpty()) return 0;
	const std::string &utf8 = toUtf8();
	return al_ustr_new_from_buffer(utf8.c_str(), utf8.size());
}


bool LString::ucs4ToUtf8(const LChar *from, const LChar *fromEnd, const LChar *&fromNext, uint8_t *to, uint8_t *toEnd, uint8_t *&toNext) {
	const unsigned long Maxcode = 0x10FFFF;
	fromNext = from;
	toNext = to;
	for (; fromNext < fromEnd; ++fromNext) {
		uint32_t wc = *fromNext;
		if ((wc & 0xFFFFF800) == 0x00D800 || wc > Maxcode)
			return false;
		if (wc < 0x000080) {
			if (toEnd - toNext < 1)
				return false;
			*toNext++ = static_cast<uint8_t>(wc);
		}
		else if (wc < 0x000800) {
			if (toEnd - toNext < 2)
				return false;
			*toNext++ = static_cast<uint8_t>(0xC0 | (wc >> 6));
			*toNext++ = static_cast<uint8_t>(0x80 | (wc & 0x03F));
		}
		else if (wc < 0x010000) {
			if (toEnd - toNext < 3)
				return false;
			*toNext++ = static_cast<uint8_t>(0xE0 |  (wc >> 12));
			*toNext++ = static_cast<uint8_t>(0x80 | ((wc & 0x0FC0) >> 6));
			*toNext++ = static_cast<uint8_t>(0x80 |  (wc & 0x003F));
		}
		else { // if (wc < 0x110000)
			if (toEnd-toNext < 4)
				return false;
			*toNext++ = static_cast<uint8_t>(0xF0 |  (wc >> 18));
			*toNext++ = static_cast<uint8_t>(0x80 | ((wc & 0x03F000) >> 12));
			*toNext++ = static_cast<uint8_t>(0x80 | ((wc & 0x000FC0) >> 6));
			*toNext++ = static_cast<uint8_t>(0x80 |  (wc & 0x00003F));
		}
	}
	return true;
}

static const int halfShift  = 10; /* used for shifting by 10 bits */

static const char32_t halfBase = 0x0010000UL;
static const char32_t halfMask = 0x3FFUL;

#define UNI_SUR_HIGH_START  (char32_t)0xD800
#define UNI_SUR_HIGH_END    (char32_t)0xDBFF
#define UNI_SUR_LOW_START   (char32_t)0xDC00
#define UNI_SUR_LOW_END     (char32_t)0xDFFF

#define UNI_REPLACEMENT_CHAR (char32_t)0x0000FFFD
#define UNI_MAX_BMP (char32_t)0x0000FFFF
#define UNI_MAX_UTF16 (char32_t)0x0010FFFF
#define UNI_MAX_UTF32 (char32_t)0x7FFFFFFF
#define UNI_MAX_LEGAL_UTF32 (char32_t)0x0010FFFF

static const char trailingBytesForUTF8[256] = {
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2, 3,3,3,3,3,3,3,3,4,4,4,4,5,5,5,5
};

static const char32_t offsetsFromUTF8[6] = { 0x00000000UL, 0x00003080UL, 0x000E2080UL,
			 0x03C82080UL, 0xFA082080UL, 0x82082080UL };


static bool isLegalUTF8(const uint8_t *source, int length) {
	uint8_t a;
	const uint8_t *srcptr = source+length;
	switch (length) {
		default: return false;
		/* Everything else falls through when "true"... */
		case 4: if ((a = (*--srcptr)) < 0x80 || a > 0xBF) return false;
		case 3: if ((a = (*--srcptr)) < 0x80 || a > 0xBF) return false;
		case 2: if ((a = (*--srcptr)) > 0xBF) return false;

		switch (*source) {
			/* no fall-through in this inner switch */
			case 0xE0: if (a < 0xA0) return false; break;
			case 0xED: if (a > 0x9F) return false; break;
			case 0xF0: if (a < 0x90) return false; break;
			case 0xF4: if (a > 0x8F) return false; break;
			default:   if (a < 0x80) return false;
		}

		case 1: if (*source >= 0x80 && *source < 0xC2) return false;
	}
	if (*source > 0xF4) return false;
	return true;
}

bool LString::utf8ToUtf32(const uint8_t **sourceStart, const uint8_t *sourceEnd, LChar **targetStart, LChar *targetEnd) {
	const uint8_t* source = *sourceStart;
	LChar* target = *targetStart;
	bool result = true;
	while (source < sourceEnd) {
		LChar ch = 0;
		unsigned short extraBytesToRead = trailingBytesForUTF8[*source];
		if (source + extraBytesToRead >= sourceEnd) {
			result = false; break;
		}
		/* Do this check whether lenient or strict */
		if (! isLegalUTF8(source, extraBytesToRead+1)) {
			result = false;
			break;
		}
		/*
		 * The cases all fall through. See "Note A" below.
		 */
		switch (extraBytesToRead) {
			case 5: ch += *source++; ch <<= 6;
			case 4: ch += *source++; ch <<= 6;
			case 3: ch += *source++; ch <<= 6;
			case 2: ch += *source++; ch <<= 6;
			case 1: ch += *source++; ch <<= 6;
			case 0: ch += *source++;
		}
		ch -= offsetsFromUTF8[extraBytesToRead];

		if (target >= targetEnd) {
			source -= (extraBytesToRead+1); /* Back up the source pointer! */
			result = false; break;
		}
		if (ch <= UNI_MAX_LEGAL_UTF32) {
			/*
			 * UTF-16 surrogate values are illegal in UTF-32, and anything
			 * over Plane 17 (> 0x10FFFF) is illegal.
			 */
			if (ch >= UNI_SUR_HIGH_START && ch <= UNI_SUR_LOW_END) {
				*target++ = UNI_REPLACEMENT_CHAR;
			} else {
				*target++ = ch;
			}
		} else { /* i.e., ch > UNI_MAX_LEGAL_UTF32 */
			result = false;
			*target++ = UNI_REPLACEMENT_CHAR;
		}

	}
	*sourceStart = source;
	*targetStart = target;
	return result;
}

bool LString::isWhitespace(LChar c) {
	switch (c) {
		case U'\t':
		case U'\n':
		case U'\f':
		case U'\r':
		case U' ':
			return true;
		default:
			return false;
	}
}

char32_t LString::toUpper(char32_t c) {
	if (c >= U'a' && c <= U'z') {
		return U'A' + (c - U'a');
	}
	switch (c) {
		case U'ä':
			return U'Ä';
		case U'å':
			return U'Å';
		case U'ö':
			return U'Ö';
		default:
			return c;
	}
}

char32_t LString::toLower(char32_t c) {
	if (c >= U'A' && c <= U'Z') {
		return U'a' + (c - U'A');
	}
	switch (c) {
		case U'Ä':
			return U'ä';
		case U'Å':
			return U'å';
		case U'Ö':
			return U'ö';
		default:
			return c;
	}
}

int LString::indexOfIterator(LString::ConstIterator i) const {
	return i - cbegin();
}

bool LString::isValidIterator(LString::ConstIterator i) const {
	return cbegin() <= i && cend() <= i;
}

LString LString::arg(const LString &v1) {
	LString str;
	str.reserve(this->length() + v1.length());
	for (ConstIterator i = cbegin(); i != cend(); i++) {
		if (*i == U'%') {
			if (*(i + 1) == U'1') {
				i += 2;
				str += v1;
				continue;
			}
		}
		str += *i;
	}
	return str;
}

LString LString::arg(const LString &v1, const LString &v2) {
	LString str;
	str.reserve(this->length() + v1.length() + v2.length());
	for (ConstIterator i = cbegin(); i != cend(); i++) {
		if (*i == U'%') {
			if (*(i + 1) == U'1') {
				i++;
				str += v1;
				continue;
			} else if (*(i + 1) == U'2') {
				i++;
				str += v2;
				continue;
			}
		}
		str += *i;
	}
	return str;
}

LString LString::arg(const LString &v1, const LString &v2, const LString &v3) {
	LString str;
	str.reserve(this->length() + v1.length() + v2.length() + v3.length());
	for (ConstIterator i = cbegin(); i != cend(); i++) {
		if (*i == U'%') {
			if (*(i + 1) == U'1') {
				i++;
				str += v1;
				continue;
			} else if (*(i + 1) == U'2') {
				i++;
				str += v1;
				continue;
			} else if (*(i + 1) == U'3') {
				i++;
				str += v3;
				continue;
			}
		}
		str += *i;
	}
	return str;
}

void LString::detach() {
	mData.detach();
}



//LString::StringDataPointer
//-------------------------------------------------------------------------

LString::LSharedStringDataPointer::LSharedStringDataPointer(const LString::LSharedStringDataPointer &o) : mPointer(o.mPointer) {
	increase(mPointer);
}

LString::LSharedStringDataPointer::LSharedStringDataPointer(LStringData *data) {
	mPointer = data;
}

LString::LSharedStringDataPointer::~LSharedStringDataPointer() {
	decrease(mPointer);
}

LString::LSharedStringDataPointer &LString::LSharedStringDataPointer::operator =(const LString::LSharedStringDataPointer &o) {
	increase(o.mPointer);
	decrease(mPointer);
	mPointer = o.mPointer;
	return *this;
}


void LString::LSharedStringDataPointer::detach() {
	if (!mPointer) return;
	if (atomicLoad(mPointer->mRefCount) == 1) { //Only instance == no copy required
		return;
	}

	LStringData *newData = LStringData::copy(mPointer);
	decrease(mPointer);
	mPointer = newData;
}

void LString::LSharedStringDataPointer::increase(LStringData *d) {
	if (d)
		d->increase();
}

bool LString::LSharedStringDataPointer::decrease(LStringData *d) {
	if (d) return d->decrease();
	return false;
}

bool LString::LSharedStringDataPointer::isNull() const {
	return mPointer == 0;
}


std::ostream &operator <<(std::ostream &stream, const LString &str) {
	std::string utf8 = str.toUtf8();
	stream << utf8;
	return stream;
}




