#ifndef STRINGPOOL_H
#define STRINGPOOL_H
#include <string>
#include <QLinkedList>
#include <map>
#include "llvm.h"
#include "value.h"


class StringPool {
	public:
		StringPool();
		void generateStringLiterals(Builder *builder);
		Value globalString(Builder *builder, const std::string &s);
	private:
		struct StringData {
				llvm::GlobalVariable *mConstData;
				llvm::GlobalVariable *mCBString;
		};

		std::map<std::string, StringData> mStrings;
};

#endif // STRINGPOOL_H
