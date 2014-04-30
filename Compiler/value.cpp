#include "value.h"
#include "runtime.h"
#include "intvaluetype.h"
#include "stringvaluetype.h"
#include "floatvaluetype.h"
#include "booleanvaluetype.h"
#include "bytevaluetype.h"
#include "shortvaluetype.h"
#include "builder.h"
#include <QDebug>

Value::Value():
	mValueType(0), mValue(0), mReference(false) {
}

Value::Value(const Value &value) :
	mValueType(value.mValueType), mValue(value.mValue), mConstant(value.mConstant), mReference(value.mReference) {
}

Value::Value(const ConstantValue &c, Runtime *r) :
	mValueType(0),
	mValue(0),
	mConstant(c),
	mReference(false) {
	mValueType = r->findValueType(c.type());
}

Value::Value(ValueType *t, llvm::Value *v, bool reference):
	mValueType(t), mValue(v), mReference(reference) {
}

void Value::toLLVMValue(Builder *builder) {
	if (isConstant()) {
		mValue = builder->llvmValue(constant());
		mConstant = ConstantValue();
	}
}

void Value::dump() const {
	if (!isValid()) {
		qDebug("Invalid Value");
		return;
	}
	qDebug() << "Value:" << mValueType->name() << (isReference() ? " (reference)" : "");
	if (isConstant())
		qDebug() << "\t = " << constant().valueInfo();
}
