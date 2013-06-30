#ifndef STRING_H
#define STRING_H
#include <allegro5/allegro.h>
#include <string>
#include "referencecounter.h"

struct CB_StringData {
		CB_StringData(const char32_t *txt) : mString(txt), mRefCount(1) {}
		CB_StringData(const std::u32string &txt) : mString(txt), mRefCount(1) {}
		std::u32string mString;
		mutable std::string mUtf8String;
		ReferenceCounter mRefCount;
		void increase();
		bool decrease();
};
typedef CB_StringData * CBString;
class String {
	public:
		String() : mData(0) {}
		String(const char32_t *txt);
		String(CB_StringData *d);
		String(const std::u32string &s);
		String(const String &o);
		~String();

		String & operator =(const String &o);
		String operator +(const String &a);
		bool operator == (const String &a);

		int size() const;
		bool empty() const;

		void detach();
		const std::string &toUtf8() const;
		ALLEGRO_USTR *toALLEGRO_USTR() const;
		const std::u32string &getRef() const;

		/** USE ONLY FOR RETURNING STRING FROM RUNTIME FUNCTION */
		CBString returnCBString();
		operator CBString ();
	private:

		CB_StringData *mData;

		static std::u32string sEmptyString;
		static std::string sEmptyUtf8String;
};
#endif // STRING_H
