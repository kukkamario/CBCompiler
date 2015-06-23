#include "stringpool.h"
#include "llvm.h"
#include "builder.h"
#include "stringvaluetype.h"
#include <vector>
StringPool::StringPool() {
}

void StringPool::generateStringLiterals(Builder *builder) {
	for (QMap<QString, StringData>::ConstIterator i = mStrings.begin(); i != mStrings.end(); ++i) {
		StringData sd = i.value();
		std::vector<llvm::Value*> indices;
		indices.push_back(builder->llvmValue(0));
		indices.push_back(builder->llvmValue(0));

		llvm::Value *ptr = builder->CreateInBoundsGEP(sd.mConstData, indices);
		llvm::Value *str = builder->runtime()->stringValueType()->constructString(builder, ptr);
		builder->CreateStore(str, sd.mCBString);
	}
}

Value StringPool::globalString(Builder *builder, const QString &s) {
	assert(!s.isEmpty());
	QMap<QString, StringData>::Iterator i = mStrings.find(s);
	if (i != mStrings.end()) {
		llvm::Value *str = builder->CreateLoad(i.value().mCBString, false);
		builder->runtime()->stringValueType()->refString(builder, str); //Increase reference counter
		return Value(builder->runtime()->stringValueType(), str, false);
	}
	StringData sd;
	sd.mCBString = builder->createGlobalVariable(
				builder->runtime()->stringValueType(),
				false,
				llvm::GlobalValue::CommonLinkage,
				builder->runtime()->stringValueType()->defaultValue(),
				("StringLiteral |" + s + "|").toStdString());

	std::vector<llvm::Constant*> stringChars;
	QVector<uint> chars = s.toUcs4();
	stringChars.reserve(chars.size() + 1);
	for (QVector<uint>::ConstIterator i = chars.begin(); i != chars.end(); ++i) {
		stringChars.push_back( llvm::ConstantInt::getIntegerValue( llvm::Type::getInt32Ty( builder->context() ), llvm::APInt( 32, *i ) ) );
	}
	stringChars.push_back( llvm::ConstantInt::getIntegerValue( llvm::Type::getInt32Ty( builder->context() ), llvm::APInt( 32, 0 ) ) );

	sd.mConstData = builder->createGlobalVariable(
				llvm::ArrayType::get(llvm::Type::getInt32Ty(builder->context()), s.length() + 1),
				true, //Constant
				llvm::GlobalValue::PrivateLinkage,
				llvm::ConstantArray::get(llvm::ArrayType::get(llvm::Type::getInt32Ty(builder->context()), s.length() + 1), stringChars), ("StringLiteralData |" + s + "|").toStdString());
	mStrings.insert(s, sd);

	llvm::Value *str = builder->CreateLoad(sd.mCBString, false);
	builder->runtime()->stringValueType()->refString(builder, str); //Increase reference counter
	return Value(builder->runtime()->stringValueType(), str, false);
}
