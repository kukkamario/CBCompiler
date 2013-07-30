#ifndef LSTRING_H
#define LSTRING_H
#include <sstream>
#include <string>
#include <assert.h>
#include <cstdint>
#include <allegro5/allegro.h>

#ifdef USE_BOOST_ATOMIC
	#include <boost/atomic.hpp>
	typedef boost::atomic_int AtomicInt;
#else
	typedef int AtomicInt;
#endif
int atomicLoad(AtomicInt &i);
void atomicIncrease(AtomicInt &i);
bool atomicDecrease(AtomicInt &i);
void atomicThreadFenceAcquire();

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

		LString & operator=(const LString &o);
		bool operator == (const LString &o) const;
		bool operator != (const LString &o) const;
		bool operator > (const LString &o) const;
		bool operator >= (const LString &o) const;
		bool operator < (const LString &o) const;
		bool operator <= (const LString &o) const;
		LString operator + (const LString &o) const;
		LString & operator += (const LString &o);
		LChar &operator[] (size_t i);
		LChar operator[] (size_t i) const;
		operator CBString() const;

		LString substr(int start, int len) const;

		static LString fromBuffer(LChar *buffer);
		static LString fromBuffer(LChar *buffer, size_t stringLength, size_t bufferSize);
		static LString number(int i);
		static LString number(float f);

		int toInt(bool *success = 0) const;
		int toFloat(bool *success = 0) const;

		Iterator begin();
		ConstIterator cbegin() const;
		ConstIterator begin() const { return cbegin(); }
		Iterator end();
		ConstIterator cend() const;
		ConstIterator end() const { return cend(); }

		bool isNull() const;
		bool isEmpty() const;
		size_t length() const;
		size_t size() const { return length(); }
		size_t capacity() const;
		void reserve(size_t size);

		std::u32string toU32String() const;
		std::wstring toWString() const;
		const std::string &toUtf8() const;
		ALLEGRO_USTR *toAllegroUStr() const;

		static bool ucs4ToUtf8(const LChar *from, const LChar *fromEnd, const LChar *&fromNext, uint8_t *to, uint8_t *toEnd, uint8_t *&toNext);

		bool isValidIterator(ConstIterator i) const;
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
		LSharedStringDataPointer mData;
		static std::string sNullStdString;
};

//Internal format
struct CB_StringData : LStringData {
};

std::ostream& operator<< (std::ostream &stream, const LString &str);

#endif // LSTRING_H
