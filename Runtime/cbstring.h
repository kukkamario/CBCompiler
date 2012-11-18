#ifndef STRING_H
#define STRING_H
#include <string>
#include <boost/lexical_cast.hpp>
#include "referencecounter.h"
struct CB_StringData {
		CB_StringData(const char *txt) : mString(txt), mRefCount(1) {}
		CB_StringData(const std::string &txt) : mString(txt), mRefCount(1) {}
		std::string mString;
		ReferenceCounter mRefCount;
		void increase();
		bool decrease();
};
typedef CB_StringData * CBString;
class String {
	public:
		String() : mData(0) {}
		String(const char *txt);
		String(CB_StringData *d);
		String(const std::string &s);
		String(const String &o);
		String & operator=(const String &o);
		String operator+(const String &a);
		~String();
		const std::string &getRef() const;

		/** USE ONLY FOR RETURNING STRING FROM RUNTIME FUNCTION */
		CBString returnCBString();
	private:

		CB_StringData *mData;

		static std::string staticEmptyString;
};
#endif // STRING_H
