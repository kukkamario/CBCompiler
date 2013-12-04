#include "arrayvaluetype.h"

ArrayValueType::ArrayValueType(ValueType *baseType, int dimensions):
	ValueType(baseType->runtime()) {
	assert(dimensions > 0);
}

QString ArrayValueType::name() const {
	return mBaseValueType->name() + "[" + QString(",").repeated(mDimensions - 1);
}


llvm::Constant *ArrayValueType::defaultValue() const {
	return llvm::ConstantPointerNull()
}
