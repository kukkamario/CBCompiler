#include "value.h"
#include "runtime.h"
#include "intvaluetype.h"
#include "stringvaluetype.h"
#include "floatvaluetype.h"
#include "booleanvaluetype.h"
#include "bytevaluetype.h"
#include "shortvaluetype.h"
#include "functionselectorvaluetype.h"
#include "builder.h"
#include <iostream>

Value::Value():
	mValueType(0), mValue(0), mType(tNormalValue) {
}

Value::Value(const Value &value) :
	mValueType(value.mValueType), mValue(value.mValue), mConstant(value.mConstant), mType(value.mType) {
}

Value::Value(const ConstantValue &c, Runtime *r) :
	mValueType(0),
	mValue(0),
	mConstant(c),
	mType(tConstant) {
	mValueType = r->valueTypeCollection().constantValueType(c.type());
	assert(mConstant.isValid());
}

Value::Value(ValueType *t, llvm::Value *v, bool reference):
	mValueType(t), mValue(v), mType(reference ? tReference : tNormalValue) {
}

Value::Value(ValueType *valType) :
	mValueType(valType), mValue(0), mType(tValueType) {

}

Value::Value(FunctionSelectorValueType *t) :
	mValueType(t),
	mValue(0),
	mType(tFunctionSelectorValueType) {
}

Value::~Value() {

}

Value &Value::operator=(const Value &value) {
	mValueType = value.mValueType;
	mValue = value.mValue,
	mConstant = value.mConstant;
	mType = value.mType;
	return *this;
}

bool Value::isValid() const {
	return mValueType != 0 && ((isConstant() ? mConstant.isValid() : false) || mType == tFunctionSelectorValueType || mType == tValueType || mValue);
}

void Value::toLLVMValue(Builder *builder) {
	if (isConstant()) {
		mValue = builder->llvmValue(constant());
		mConstant = ConstantValue();
		mType = tNormalValue;
	}
}

void Value::dump() const {
	if (!isValid()) {
		std::cout << "Invalid Value";
		return;
	}
	std::cout << "Value:" << mValueType->name() << (isReference() ? " (reference)" : "");
	if (isConstant())
		std::cout << "\t = " << constant().valueInfo();
}

bool Value::isNormalValue() const {
	return mType == tNormalValue || mType == tReference;
}

bool Value::isValueType() const {
	return mType == tValueType;
}

bool Value::isFunctionSelectorValueType() const {
	return mType == tFunctionSelectorValueType;
}
