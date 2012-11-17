#ifndef STRING_H
#define STRING_H
#include <string>

extern "C" class CB_String {
	public:
		CB_String() : mData(0) {}
		CB_String(const char *txt) :
			mData(0) {
			if (txt && txt[0] != 0) {
				mData = new StringData(txt);
			}
		}
		CB_String(const CB_String &o) : mData(o.mData) {
			if (mData) ++mData->mRefCount;
		}

		CB_String & operator==(const CB_String &o) {
			StringData *tD = this->mData;
			this->mData = o.mData;
			if (this->mData) this->mData->mRefCount++;
			if (tD) {
				if (--tD->mRefCount <= 0) {
					delete mData;
				}

			}
		}

		~CB_String() {
			if (mData) {
				if (--mData->mRefCount <= 0) { //No more references
					delete mData;
				}
			}
		}

		const std::string &getRef() const;

	private:
		struct StringData {
				StringData(const char *txt) : mRefCount(1), mString(txt) {}
				std::string mString;
				int mRefCount;
		};

		StringData *mData;

		static std::string staticEmptyString;
};

typedef CB_String String;
#endif // STRING_H
