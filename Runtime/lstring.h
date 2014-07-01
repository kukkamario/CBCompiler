#ifndef LSTRING_H
#define LSTRING_H
#include <sstream>
#include <string>
#include <assert.h>
#include <cstdint>
#include <allegro5/allegro.h>
#include "common.h"
#include "atomicint.h"
typedef char32_t LChar;

class LStringData {
	public:
		static LStringData *create(size_t size);
		static LStringData *create(const LChar *text);
		static LStringData *create(const LChar *text, size_t len);
		static LStringData *createFromBuffer(LChar *buffer);
		static LStringData *createFromBuffer(LChar *buffer, size_t stringLength, size_t bufferSize);
		static LStringData *copy(LStringData *o);
		static void destruct(LStringData *d);
		void increase();
		bool decrease();
		bool isStaticData() const;

		mutable AtomicInt mRefCount;
		mutable std::string *mUtf8String;
		size_t mLength;
		size_t mSize;
		LChar *mData;

		//LChar mRealData[mSize];
	private:

		//Don't implement
		LStringData();
		~LStringData();
};
struct CB_StringData;
typedef CB_StringData* CBString;

class LString {
	public:
		typedef LChar * iterator;
		typedef const LChar * const_iterator;
		typedef iterator Iterator;
		typedef const_iterator ConstIterator;

		LString();
		LString(const LChar *str);
		LString(const LChar *str, size_t len);
		LString(ConstIterator begin, ConstIterator end);
		LString(const LString &o);
		LString(CBString cbString);

		~LString();

		static LString fromBuffer(LChar *buffer);
		static LString fromBuffer(LChar *buffer, size_t stringLength, size_t bufferSize);
		static LString fromUtf8(const std::string &s);
		static LString number(int i);
		static LString number(float f);

		LString & operator=(const LString &o);
		bool operator == (const LString &o) const;
		bool operator != (const LString &o) const;
		bool operator > (const LString &o) const;
		bool operator >= (const LString &o) const;
		bool operator < (const LString &o) const;
		bool operator <= (const LString &o) const;
		LString operator + (const LString &o) const;
		LString & operator += (const LString &o);
		LString & operator += (LChar c);
		LChar &operator[] (int i);
		const LChar &operator[] (int i) const;
		operator CBString() const;

		LString substr(int start, int len) const;
		LString left(int chars) const;
		LString right(int chars) const;

		Iterator find(LChar c);
		Iterator find(LChar c, Iterator start);
		ConstIterator find(LChar c) const;
		ConstIterator find(LChar c, ConstIterator start) const;

		ConstIterator find(const LString &str) const;
		Iterator find(const LString &str);
		ConstIterator find(const LString &str, ConstIterator start) const;
		Iterator find(const LString &str, Iterator start);

		int indexOf(LChar c) const;
		int indexOf(LChar c, int start) const;
		int indexOf(const LString &str) const;
		int indexOf(const LString &str, int start) const;

		void clear();

		void remove(int start, int len);

		int toInt(bool *success = 0) const;
		int toFloat(bool *success = 0) const;

		Iterator begin();
		ConstIterator cbegin() const;
		ConstIterator begin() const { return cbegin(); }
		Iterator end();
		ConstIterator cend() const;
		ConstIterator end() const { return cend(); }

		Iterator at(int i);
		ConstIterator at(int i) const;


		bool isNull() const;
		bool isEmpty() const;
		size_t length() const;
		size_t size() const { return length(); }
		size_t capacity() const;
		void reserve(size_t size);
		void resize(size_t size);

		std::u32string toU32String() const;
		std::wstring toWString() const;
		const std::string &toUtf8() const;
		ALLEGRO_USTR *toAllegroUStr() const;

		static bool ucs4ToUtf8(const LChar *from, const LChar *fromEnd, const LChar *&fromNext, uint8_t *to, uint8_t *toEnd, uint8_t *&toNext);

		static bool utf8ToUtf32(const uint8_t **sourceStart, const uint8_t *sourceEnd, LChar **targetStart, LChar *targetEnd);

		int indexOfIterator(ConstIterator i) const;
		bool isValidIterator(ConstIterator i) const;

		LString arg(const LString &v1);
		LString arg(const LString &v1, const LString &v2);
		LString arg(const LString &v1, const LString &v2, const LString &v3);

		void detach();
	private:
		LString(LStringData *data);
		class LSharedStringDataPointer {
			public:
				LSharedStringDataPointer() : mPointer(0) {}
				LSharedStringDataPointer(const LSharedStringDataPointer &o);
				LSharedStringDataPointer(LStringData *data);
				~LSharedStringDataPointer();
				LStringData *operator -> () { detach(); assert(mPointer); return mPointer; }
				const LStringData *operator -> () const { assert(mPointer); return mPointer; }
				LStringData & operator*() { detach(); assert(mPointer); return *mPointer; }
				const LStringData & operator*() const { assert(mPointer); return *mPointer; }
				LSharedStringDataPointer & operator =(const LSharedStringDataPointer &o);
				bool isNull() const;
				void detach();
				LStringData *unsafePointer() const { return mPointer; }
			private:
				void increase(LStringData *d);
				bool decrease(LStringData *d);
				LStringData *mPointer;
		};
		size_t nextSize() const;
		LSharedStringDataPointer mData;
		static std::string sNullStdString;
};

//Internal format
struct CB_StringData : LStringData {
};

std::ostream& operator<< (std::ostream &stream, const LString &str);

#endif // LSTRING_H
