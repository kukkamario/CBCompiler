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
	mValueType(value.mValueType), mValue(value.mValue), mConstantData(value.mConstantData), mConstant(value.mConstant){
}

Value::Value(bool v) {
	mValueType = Runtime::instance()->booleanValueType();
	mValue = Runtime::instance()->booleanValueType()->constant(v);
	mConstantData.mBool = v;
	mConstant = true;
}

Value::Value(int v) {
	mValueType = Runtime::instance()->intValueType();
	mValue = Runtime::instance()->intValueType()->constant(v);
	mConstantData.mInt = v;
	mConstant = true;
}

Value::Value(float f) {
	mValueType = Runtime::instance()->floatValueType();
	mValue = Runtime::instance()->floatValueType()->constant(f);
	mConstantData.mFloat = f;
	mConstant = true;
}

Value::Value(quint16 v) {
	mValueType = Runtime::instance()->intValueType();
	mValue = Runtime::instance()->intValueType()->constant(v);
	mConstantData.mShort = v;
	mConstant = true;
}

Value::Value(quint8 v) {
	mValueType = Runtime::instance()->byteValueType();
	mValue = Runtime::instance()->byteValueType()->constant(v);
	mConstantData.mByte = v;
	mConstant = true;
}

Value::Value(ValueType *t, llvm::Value *v):
	mValueType(t), mValue(v)
{

}
