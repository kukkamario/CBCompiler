#include "value.h"
#include "runtime.h"
#include "intvaluetype.h"
#include "stringvaluetype.h"
#include "floatvaluetype.h"
#include "booleanvaluetype.h"
#include "bytevaluetype.h"
#include "shortvaluetype.h"

Value::Value():
	mValueType(0), mValue(0) {
}

Value::Value(const Value &value) :
	mValueType(value.mValueType), mValue(value.mValue), mConstant(value.mConstant){
}

Value::Value(const ConstantValue &c) :
	mValueType(0),
	mValue(0),
	mConstant(c) {

}

Value::Value(ValueType *t, llvm::Value *v):
	mValueType(t), mValue(v) {
}
