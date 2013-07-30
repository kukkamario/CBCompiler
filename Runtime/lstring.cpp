#include "lstring.h"
#include <stddef.h>
#include <string.h>
#include <algorithm>
#include <stdio.h>
#include <locale>


std::string LString::sNullStdString;


//LStringData
//-------------------------------------------------------------------------


LStringData *LStringData::create(size_t size) {
	assert(size > 0);
	char *buffer = new char[sizeof(LStringData) + size * sizeof(LChar)];
	LStringData *ret = reinterpret_cast<LStringData*>(buffer);
	ret->mRefCount = 1;
	ret->mLength = 0;
	ret->mSize = size;
	ret->mUtf8String = 0;
	ret->mData = reinterpret_cast<LChar*>(reinterpret_cast<char*>(ret) + sizeof(LStringData));
	return ret;
}

LStringData *LStringData::create(const LChar *text) {
	size_t len = 0;
	while(text[len]) {
		len++;
	}

	LStringData *d = create(len + 1);
	d->mLength = len;
	memcpy(d->mData, text, (len + 1) * sizeof(LChar));
	return d;
}

LStringData *LStringData::create(const LChar *text, size_t len) {
	LStringData *d = create(len + 1);
	d->mLength = len;
	memcpy(d->mData, text, len * sizeof(LChar));
	d->mData[len] = 0; //null
	return d;
}

LStringData *LStringData::createFromBuffer(LChar *buffer) {
	size_t len = 0;
	while(buffer[len]) {
		len++;
	}
	return createFromBuffer(buffer, len, len + 1);
}

LStringData *LStringData::createFromBuffer(LChar *buffer, size_t stringLength, size_t bufferSize) {
	char *buf = new char[sizeof(LStringData)];
	LStringData *ret = reinterpret_cast<LStringData*>(buf);
	ret->mData = buffer;
	ret->mLength = stringLength;
	ret->mSize = bufferSize;
	ret->mUtf8String = 0;
	ret->mRefCount = 1;
	return ret;
}

LStringData *LStringData::copy(LStringData *o) {
	if (o->isStaticData()) {
		char *buffer = new char[sizeof(LStringData)];
		LStringData *ret = reinterpret_cast<LStringData*>(buffer);
		memcpy(o, ret, sizeof(LStringData));
		ret->mRefCount = 1;
		return ret;
	}
	char *buffer = new char[sizeof(LStringData) + o->mSize * sizeof(LChar)];
	LStringData *ret = reinterpret_cast<LStringData*>(buffer);
	memcpy(o, ret, sizeof(LStringData) + o->mSize * sizeof(LChar));
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
	return mData != reinterpret_cast<const LChar*>(reinterpret_cast<const char*>(this) + sizeof(LStringData));
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

LString LString::fromBuffer(LChar *buffer, size_t stringLength, size_t bufferSize) {
	assert(stringLength < bufferSize);
	return LString(LStringData::createFromBuffer(buffer, stringLength, bufferSize));
}

LString LString::fromBuffer(LChar *buffer) {
	return LString(LStringData::createFromBuffer(buffer));
}

LString LString::number(int i) {
	if (i == 0) return LString(U"0");
	LStringData *data = LStringData::create(12);
	LChar *str = data->mData;
	if (i < 0) {
		*str = U'-';
		i = -i;
		str++;
	}
	LChar *begin = str;
	while (i) {
		*str = U'0' + i % 10;
		i /= 10;
		str++;
	}
	std::reverse(begin, str);
	*str = 0;
	data->mLength = str - data->mData;
	return LString(data);
}

LString LString::number(float f) {
	const size_t bufferSize = 12;
	LStringData *data = LStringData::create(bufferSize);

#ifndef _WIN32
	assert(sizeof(wchar_t) == sizeof(char32_t));
	data->mLength = swprintf(reinterpret_cast<wchar_t*>(data->mData), bufferSize, L"%g", f);
	return LString(data);
#else
	char charBuf[bufferSize];
	data->mLength = snprintf(charBuf, bufferSize, "%g", f);
	for (size_t i = 0; i < data->mLength; ++i) {
		data->mData[i] = static_cast<char32_t>(charBuf[i]);
	}
	data->mData[data->mLength] = 0;
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
	memcpy(newStr->mData, this->mData->mData, this->length() * sizeof(LChar));
	memcpy(newStr->mData + this->length(), o.mData->mData, o.length() * sizeof(LChar));

	newStr->mLength = this->length() + o.length();
	newStr->mData[newStr->mLength] = 0;
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
	memcpy(this->mData->mData + this->length(), o.mData->mData, o.length());

	this->mData->mData[this->length() + o.length()] = 0;
	return *this;
}

LString &LString::operator +=(LChar c) {
	if (this->length() + 1 > this->size()) {
		reserve(nextSize());
	}
	this->mData->mData[this->mData->mLength] = c;
	this->mData->mData[this->mData->mLength + 1] = 0;
	++this->mData->mLength;
	return *this;
}

LChar &LString::operator [](size_t i) {
	assert(i < length());
	return this->mData->mData[i];
}

LChar LString::operator [](size_t i) const {
	assert(i < length());
	return this->mData->mData[i];
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
	assert(chars <= (int)length() && chars > 0);
	return LString(begin(), chars);
}

LString LString::right(int chars) const {
	assert(chars <= (int)length() && chars > 0);
	return LString(begin() + (length() - chars), (length() - chars));
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

int LString::indexOf(const LString &str) const {
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
		if (swscanf(reinterpret_cast<const wchar_t*>(mData->mData), L"%f", &val) != 1) {
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
	return isNull() ? 0 : mData->mData;
}

LString::ConstIterator LString::cbegin() const {
	return isNull() ? 0 : mData->mData;
}

LString::Iterator LString::end() {
	return isNull() ? 0 : mData->mData + mData->mLength;
}

LString::ConstIterator LString::cend() const {
	return isNull() ? 0 : mData->mData + mData->mLength;
}

bool LString::isNull() const {
	return mData.isNull();
}

bool LString::isEmpty() const {
	return isNull() || mData->mLength == 0;
}

size_t LString::length() const {
	return isNull() ? 0 : mData->mLength;
}

size_t LString::capacity() const {
	return isNull() ? 0 : mData->mSize - 1;
}

void LString::reserve(size_t size) {
	if (size <= capacity()) return;
	if (isNull()) {
		mData = LStringData::create(size + 1);
		return;
	}
	else {
		LStringData *newData = LStringData::create(size + 1);
		memcpy(newData->mData, this->mData->mData, this->mData->mLength * sizeof(LChar));
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

	std::string *utf8 = new std::string(mData->mLength * 4, '\0');
	const LChar* fromNext;
	uint8_t* toNext;
	assert(ucs4ToUtf8(cbegin(), cend(), fromNext, (uint8_t*)&(*utf8)[0], (uint8_t*)&(*utf8)[utf8->size()], toNext));
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
			i++;
			if (*i == U'1') {
				i++;
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
			i++;
			if (*i == U'1') {
				i++;
				str += v1;
				continue;
			} else if (*i == U'2') {
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
			i++;
			if (*i == U'1') {
				i++;
				str += v1;
				continue;
			} else if (*i == U'2') {
				i++;
				str += v1;
				continue;
			} else if (*i == U'3') {
				i++;
				str += v3;
				continue;
			}
		}
		str += *i;
	}
	return str;
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


//Atomic operations
//-------------------------------------------------------------------------

#ifdef USE_BOOST_ATOMIC
void atomicIncrease(AtomicInt &i) {
	i.fetch_add(1u, boost::memory_order_relaxed);
}


bool atomicDecrease(AtomicInt &i) {
	return i.fetch_sub(1, boost::memory_order_release) == 1;
}


void atomicThreadFenceAcquire() {
	boost::atomic_thread_fence(boost::memory_order_acquire);
}

int atomicLoad(AtomicInt &i) {
	return i.load(boost::memory_order_release);
}
#else
void atomicIncrease(AtomicInt &i) {
	++i;
}


bool atomicDecrease(AtomicInt &i) {
	return --i == 0;
}


void atomicThreadFenceAcquire() {
}


int atomicLoad(AtomicInt &i) {
	return i;
}

#endif

