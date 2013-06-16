#ifndef STRINGPOOL_H
#define STRINGPOOL_H
#include <QString>
#include <QLinkedList>
#include <QMap>
#include <llvm/IRBuilder.h>
#include "value.h"


class StringPool {
	public:
		StringPool();
		void generateStringLiterals(Builder *builder);
		Value globalString(Builder *builder, const QString &s);
	private:
		struct StringData {
				llvm::GlobalVariable *mConstData;
				llvm::GlobalVariable *mCBString;
		};

		QMap<QString, StringData> mStrings;
};

#endif // STRINGPOOL_H
