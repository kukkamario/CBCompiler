#include "stringpool.h"
#include "llvm.h"
StringPool::StringPool() {
}

QString *StringPool::add(const QString &s) {
	QMap<QString, StringItem>::Iterator i = mStrings.find(s);
	if (i != mStrings.end()) {
		return i.value().mString;
	}
	StringItem sp(s);
	mReverse.insert(sp.mString, mStrings.insert(s, sp));
	return sp.mString;
}

llvm::Value *StringPool::globalString(llvm::IRBuilder<> *builder, QString *s) {
	QMap<QString*, QMap<QString, StringItem>::Iterator >::Iterator i = mReverse.find(s);
	assert(i != mReverse.end());
	StringItem &item = i.value().value();
	if (!item.mGlobalString) {
		item.mGlobalString = builder->CreateGlobalStringPtr(item.mString->toStdString());
	}

	return item.mGlobalString;
}
