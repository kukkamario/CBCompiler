#include "arrayvaluetype.h"
#include "runtime.h"
ArrayValueType::ArrayValueType(ValueType *baseType, llvm::Type *llvmType, int dimensions):
	ValueType(baseType->runtime(), llvmType) {
	assert(dimensions > 0);
}

QString ArrayValueType::name() const {
	return mBaseValueType->name() + "[" + QString(",").repeated(mDimensions - 1);
}

ValueType::CastCost ArrayValueType::castingCostToOtherValueType(const ValueType *to) const {
	if (to == this) return ccNoCost;
	return ccNoCast;
}

Value ArrayValueType::cast(Builder *, const Value &v) const {
	if (v.valueType() == this) return v;

	return Value();
}


llvm::Constant *ArrayValueType::defaultValue() const {
	return llvm::ConstantPointerNull::get(llvm::cast<llvm::PointerType>(mType));
}

int ArrayValueType::size() const {
	return mRuntime->dataLayout().getPointerSize();
}
