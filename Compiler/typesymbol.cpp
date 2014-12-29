#include "typesymbol.h"
#include "typepointervaluetype.h"
#include "llvm.h"
#include "runtime.h"
#include "builder.h"
#include "typevaluetype.h"
#include <boost/locale.hpp>


TypeSymbol::TypeSymbol(const std::string &name, Runtime *r, const CodePoint &cp):
	ValueTypeSymbol(name, cp),
	mGlobalTypeVariable(0),
	mTypePointerValueType(new TypePointerValueType(r, this)),
	mFirstFieldIndex(0),
	mMemberSize(0) {
}

bool TypeSymbol::addField(const TypeField &field) {
	if (mFieldSearch.find(field.name()) != mFieldSearch.end()) {
		return false;
	}
	mFields.push_back(field);
	mFieldSearch.insert(std::pair<std::string, int>(field.name(), mFields.size() - 1));
	return true;
}

bool TypeSymbol::hasField(const std::string &name) {
	return mFieldSearch.find(name) != mFieldSearch.end();
}

const TypeField &TypeSymbol::field(const std::string &name) const{
	std::map<std::string, int>::const_iterator i = mFieldSearch.find(name);
	assert(i != mFieldSearch.end());
	return mFields.at(i->second);
}

int TypeSymbol::fieldIndex(const std::string &name) const {
	std::map<std::string, int>::const_iterator i = mFieldSearch.find(name);
	int fieldI = i->second;
	return mFirstFieldIndex + fieldI;
}

ValueType *TypeSymbol::valueType() const {
	return typePointerValueType();
}

void TypeSymbol::initializeType(Builder *b) {
	b->irBuilder().CreateCall2(mRuntime->typeValueType()->constructTypeFunction(), mGlobalTypeVariable, b->llvmValue(mMemberSize));
}

void TypeSymbol::createOpaqueTypes(Builder *b) {
	mMemberType = llvm::StructType::create(b->context());
	mTypePointerValueType->setLLVMType(mMemberType->getPointerTo());
}


TypeField::TypeField(const std::string &name, ValueType *valueType, const CodePoint &cp) :
	mName(name),
	mValueType(valueType),
	mCodePoint(cp) {
}

std::string TypeField::info() const {
	return "Field " + mValueType->name() + " " + mName;
}


void TypeSymbol::createTypePointerValueType(Builder *b) {
	mRuntime = b->runtime();
	createLLVMMemberType();
	mGlobalTypeVariable = b->createGlobalVariable(mRuntime->typeLLVMType(), false, llvm::GlobalValue::InternalLinkage, llvm::Constant::getNullValue(mRuntime->typeLLVMType()));
}

Value TypeSymbol::typeValue() {
	return Value(mRuntime->typeValueType(), mGlobalTypeVariable, false);
}

void TypeSymbol::createLLVMMemberType() {
	assert(mMemberType);

	std::vector<llvm::Type*> elements;

	//Copy header
	unsigned i;
	for (i = 0; i < mRuntime->typeMemberLLVMType()->getStructNumElements(); i++) {
		elements.push_back(mRuntime->typeMemberLLVMType()->getStructElementType(i));
	}
	mFirstFieldIndex = i;
	foreach(const TypeField &field, mFields) {
		elements.push_back(field.valueType()->llvmType());
	}

	mMemberType->setBody(elements);
	mMemberSize = mRuntime->dataLayout().getTypeAllocSize(mMemberType);

}


std::string TypeSymbol::info() const {
	std::string str = "Type " + mName + "   |   Size: "+ boost::lexical_cast<std::string>(mMemberSize) + "bytes\n";
	for (std::vector<TypeField>::const_iterator i = mFields.begin(); i != mFields.end(); i++) {
		str += "    " + i->info() + '\n';
	}
	return str;
}
