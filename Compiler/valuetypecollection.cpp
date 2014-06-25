#include "valuetypecollection.h"
#include "valuetype.h"
#include "arrayvaluetype.h"
#include "intvaluetype.h"
#include "floatvaluetype.h"
#include "shortvaluetype.h"
#include "bytevaluetype.h"
#include "booleanvaluetype.h"
#include "stringvaluetype.h"
#include "typepointervaluetype.h"
#include "runtime.h"

ValueTypeCollection::ValueTypeCollection(Runtime *r) :
mRuntime(r) {
}

ValueTypeCollection::~ValueTypeCollection() {
}

void ValueTypeCollection::addValueType(ValueType *valType) {
	mLLVMTypeMapping[valType->llvmType()] = valType;
	if (valType->isNamedValueType())
		mNamedType[valType->name().toLower()] = valType;
}

ValueType *ValueTypeCollection::valueTypeForLLVMType(llvm::Type *type) {
	return mLLVMTypeMapping.value(type, 0);
}

ValueType *ValueTypeCollection::findNamedType(const QString &name) {
	return mNamedType.value(name.toLower(), 0);
}

ArrayValueType *ValueTypeCollection::arrayValueType(ValueType *baseValueType, int dimensions) {
	QMap<QPair<ValueType*, int> , ArrayValueType *>::Iterator i = mArrayMapping.find(QPair<ValueType*, int>(baseValueType, dimensions));
	if (i != mArrayMapping.end()) {
		return i.value();
	}

	ArrayValueType *valTy = new ArrayValueType(baseValueType, llvm::IntegerType::getInt8PtrTy(mRuntime->module()->getContext()), dimensions);
	mArrayMapping[QPair<ValueType*, int>(baseValueType, dimensions)] = valTy;
	return valTy;
}

ValueType *ValueTypeCollection::constantValueType(ConstantValue::Type type) const {
	switch (type) {
		case ConstantValue::Byte:
			return mRuntime->byteValueType();
		case ConstantValue::Short:
			return mRuntime->shortValueType();
		case ConstantValue::Integer:
			return mRuntime->intValueType();
		case ConstantValue::Float:
			return mRuntime->floatValueType();
		case ConstantValue::Boolean:
			return mRuntime->booleanValueType();
		case ConstantValue::String:
			return mRuntime->stringValueType();
		case ConstantValue::Null:
			return mRuntime->typePointerCommonValueType();
		default:
			assert("Invalid ConstantValue::Type" && 0);
			return 0;
	}
}

QList<ValueType *> ValueTypeCollection::namedTypes() const {
	return mNamedType.values();
}
