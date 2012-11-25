#include "stringpool.h"
#include "llvm.h"
StringPool::StringPool() {
}

llvm::Value *StringPool::globalString(llvm::IRBuilder<> *builder, const QString &s) {
	QMap<QString, llvm::Value*>::Iterator i = mStrings.find(s);
	if (i != mStrings.end()) {
		return i.value();
	}
	llvm::Value *gs = builder->CreateGlobalStringPtr(s.toStdString());
	mStrings.insert(s, gs);
	return gs;
}
