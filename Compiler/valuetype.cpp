#include "valuetype.h"
#include "llvm.h"
ValueType::ValueType(Runtime *r):
	mType(0),
	mRuntime(r)
{
}

llvm::LLVMContext &ValueType::context() {
	assert(mType);
	return mType->getContext();
}
