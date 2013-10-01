#include "valuetype.h"
#include "llvm.h"
ValueType::ValueType(Runtime *r):
	mType(0),
	mRuntime(r)
{
}

ValueType::ValueType(Runtime *r, llvm::Type *type) :
	mType(type),
	mRuntime(r) {
}

bool ValueType::canBeCastedToValueType(ValueType *to) const {
	return castingCostToOtherValueType(to) < sMaxCastCost;
}

llvm::LLVMContext &ValueType::context() {
	assert(mType);
	return mType->getContext();
}
