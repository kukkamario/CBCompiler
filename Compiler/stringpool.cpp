#include "stringpool.h"
#include "llvm.h"
#include "builder.h"
#include "stringvaluetype.h"
#include <vector>
#include "utf8.h"
#include <cstdint>
StringPool::StringPool() {
}

void StringPool::generateStringLiterals(Builder *builder) {
	for (std::map<std::string, StringData>::const_iterator i = mStrings.begin(); i != mStrings.end(); ++i) {
		StringData sd = i->second;
		std::vector<llvm::Value*> indices;
		indices.push_back(builder->llvmValue(0));
		indices.push_back(builder->llvmValue(0));

		llvm::Value *ptr = builder->irBuilder().CreateInBoundsGEP(sd.mConstData, indices);
		llvm::Value *str = builder->runtime()->stringValueType()->constructString(&builder->irBuilder(), ptr);
		builder->irBuilder().CreateStore(str, sd.mCBString);
	}
}

Value StringPool::globalString(Builder *builder, const std::string &s) {
	assert(!s.empty());
	std::map<std::string, StringData>::iterator i = mStrings.find(s);
	if (i != mStrings.end()) {
		llvm::Value *str = builder->irBuilder().CreateLoad(i->second.mCBString, false);
		builder->runtime()->stringValueType()->refString(&builder->irBuilder(), str); //Increase reference counter
		return Value(builder->runtime()->stringValueType(), str, false);
	}
	StringData sd;
	sd.mCBString = builder->createGlobalVariable(
				builder->runtime()->stringValueType(),
				false,
				llvm::GlobalValue::CommonLinkage,
				builder->runtime()->stringValueType()->defaultValue(),
				("StringLiteral |" + s + "|"));

	std::vector<llvm::Constant*> stringChars;
	std::vector<char32_t> chars(s.size(), '\0');
	std::vector<char32_t>::iterator charsEnd = utf8::utf8to32(s.begin(), s.end(), chars.begin());
	chars.resize(charsEnd - chars.begin());
	stringChars.reserve(chars.size() + 1);
	for (char32_t c : chars) {
		stringChars.push_back( llvm::ConstantInt::getIntegerValue( llvm::Type::getInt32Ty( builder->context() ), llvm::APInt( 32, c ) ) );
	}
	stringChars.push_back( llvm::ConstantInt::getIntegerValue( llvm::Type::getInt32Ty( builder->context() ), llvm::APInt( 32, 0 ) ) );

	sd.mConstData = builder->createGlobalVariable(
				llvm::ArrayType::get(llvm::Type::getInt32Ty(builder->context()), s.length() + 1),
				true, //Constant
				llvm::GlobalValue::PrivateLinkage,
				llvm::ConstantArray::get(llvm::ArrayType::get(llvm::Type::getInt32Ty(builder->context()), s.length() + 1), stringChars), ("StringLiteralData |" + s + "|"));
	mStrings.insert(std::pair<std::string, StringData>(s, sd));

	llvm::Value *str = builder->irBuilder().CreateLoad(sd.mCBString, false);
	builder->runtime()->stringValueType()->refString(&builder->irBuilder(), str); //Increase reference counter
	return Value(builder->runtime()->stringValueType(), str, false);
}
