#include "valuetypecollection.h"
#include "valuetype.h"
#include "arrayvaluetype.h"
#include "runtime.h"

ValueTypeCollection::ValueTypeCollection(Runtime *r) :
mRuntime(r) {
}

ValueTypeCollection::~ValueTypeCollection() {
	for (QMap<llvm::Type*, ValueType*>::Iterator i = mLLVMTypeMapping.begin(); i != mLLVMTypeMapping.end(); i++) {
		delete i.value();
	}
}

void ValueTypeCollection::addValueType(ValueType *valType) {
	mLLVMTypeMapping[valType->llvmType()] = valType;
	if (valType->isNamedValueType())
		mNamedType[valType->name()] = valType;
}

ValueType *ValueTypeCollection::valueTypeForLLVMType(llvm::Type *type) {
	return mLLVMTypeMapping.value(type, 0);
}

ValueType *ValueTypeCollection::findNamedType(const QString &name) {
	return mNamedType.value(name, 0);
}

ArrayValueType *ValueTypeCollection::arrayValueType(ValueType *baseValueType, int dimensions) {
	QMap<QPair<ValueType*, int> , ValueType *>::Iterator i = mArrayMapping.find(QPair<ValueType*, int>(baseValueType, dimensions));
	if (i != mArrayMapping.end()) {
		return i.value();
	}

	ArrayValueType *valTy = new ArrayValueType(baseValueType, llvm::IntegerType::getInt8PtrTy(mRuntime->module()->getContext()), dimensions);
	mArrayMapping[QPair<ValueType*, int>(baseValueType, dimensions)] = valTy;
	return valTy;
}
