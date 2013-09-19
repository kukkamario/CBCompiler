#include "typesymbol.h"
#include "typepointervaluetype.h"
#include "llvm.h"
#include "runtime.h"
#include "builder.h"
#include "typevaluetype.h"


TypeSymbol::TypeSymbol(const QString &name, Runtime *r, const QString &file, int line):
	ValueTypeSymbol(name, file, line),
	mGlobalTypeVariable(0),
	mFirstFieldIndex(0),
	mMemberSize(0),
	mTypePointerValueType(new TypePointerValueType(r, this)) {
}

bool TypeSymbol::addField(const TypeField &field) {
	if (mFieldSearch.contains(field.name())) {
		return false;
	}
	mFields.append(field);
	mFieldSearch.insert(field.name(), mFields.size() - 1);
	return true;
}

bool TypeSymbol::hasField(const QString &name) {
	return mFieldSearch.contains(name);
}

const TypeField &TypeSymbol::field(const QString &name) const{
	QMap<QString, int>::ConstIterator i = mFieldSearch.find(name);
	assert(i != mFieldSearch.end());
	return mFields.at(i.value());
}

int TypeSymbol::fieldIndex(const QString &name) const {
	QMap<QString, int>::ConstIterator i = mFieldSearch.find(name);
	int fieldI = i.value();
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
}


TypeField::TypeField(const QString &name, ValueType *valueType, const QString &file, int line) :
	mName(name),
	mValueType(valueType),
	mLine(line),
	mFile(file){
}

QString TypeField::info() const {
	return QString("Field %1 %2").arg(mValueType->name(), mName);
}


void TypeSymbol::createTypePointerValueType(Builder *b) {
	mRuntime = b->runtime();
	createLLVMMemberType();
	mGlobalTypeVariable = b->createGlobalVariable(mRuntime->typeLLVMType(), false, llvm::GlobalValue::InternalLinkage, llvm::Constant::getNullValue(mRuntime->typeLLVMType()));
}

Value TypeSymbol::typeValue() {
	return Value(mRuntime->typeValueType(), mGlobalTypeVariable);
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

	mTypePointerValueType->setLLVMType(mMemberType->getPointerTo());
}


QString TypeSymbol::info() const {
	QString str("Type %1   |   Size: %2 bytes\n");
	str = str.arg(mName, QString::number(mMemberSize));
	for (QList<TypeField>::ConstIterator i = mFields.begin(); i != mFields.end(); i++) {
		str += "    " + i->info() + '\n';
	}
	return str;
}
