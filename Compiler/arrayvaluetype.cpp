#include "arrayvaluetype.h"
#include "runtime.h"
ArrayValueType::ArrayValueType(ValueType *baseType, llvm::Type *llvmType, int dimensions):
	ValueType(baseType->runtime(), llvmType) {
	assert(dimensions > 0);
}

QString ArrayValueType::name() const {
	return mBaseValueType->name() + "[" + QString(",").repeated(mDimensions - 1);
}


llvm::Constant *ArrayValueType::defaultValue() const {
	return llvm::ConstantPointerNull(llvm::cast<llvm::PointerType>(mType));
}

int ArrayValueType::size() const {
	return mRuntime->dataLayout().getPointerSize();
}
