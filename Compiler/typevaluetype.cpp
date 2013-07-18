#include "typevaluetype.h"
#include "value.h"
#include "runtime.h"

TypeValueType::TypeValueType(Runtime *r, llvm::Type *type) :
	ValueType(r),
	mConstructTypeFunction(0),
	mNewFunction(0),
	mFirstFunction(0),
	mLastFunction(0),
	mBeforeFunction(0),
	mAfterFunction(0) {
	mType = type;
}

ValueType::CastCostType TypeValueType::castingCostToOtherValueType(ValueType *to) const {
	if (to == this) return 0;
	return sMaxCastCost;
}

Value TypeValueType::cast(Builder *, const Value &v) const {
	if (v.valueType() == this) return v;
	return Value();
}

llvm::Constant *TypeValueType::defaultValue() const {
	return llvm::ConstantPointerNull::get(static_cast<llvm::PointerType*>(mType));
}


int TypeValueType::size() const {
	return mRuntime->dataLayout().getPointerSize();
}

bool TypeValueType::isValid() {
	return mConstructTypeFunction && mNewFunction && mFirstFunction && mLastFunction && mBeforeFunction && mAfterFunction;
}

bool TypeValueType::setConstructTypeFunction(llvm::Function *func) {
	if (func->arg_size() != 2) return false;
	llvm::Function::arg_iterator i = func->arg_begin();
	if (i->getType() != mRuntime->typeLLVMType()->getPointerTo()) return false;
	i++;
	if (i->getType() != llvm::Type::getInt32Ty(mRuntime->module()->getContext())) return false;

	if (func->getReturnType() != llvm::Type::getVoidTy(mRuntime->module()->getContext())) return false;

	mConstructTypeFunction = func;
	return true;
}

bool TypeValueType::setNewFunction(llvm::Function *func) {
	if (func->arg_size() != 1) return false;
	llvm::Function::arg_iterator i = func->arg_begin();
	if (i->getType() != mRuntime->typeLLVMType()->getPointerTo()) return false;
	if (func->getReturnType() != mRuntime->typeMemberPointerLLVMType()) return false;
	mNewFunction = func;
	return true;
}

bool TypeValueType::setFirstFunction(llvm::Function *func) {
	if (func->arg_size() != 1) return false;
	llvm::Function::arg_iterator i = func->arg_begin();
	if (i->getType() != mRuntime->typeLLVMType()->getPointerTo()) return false;
	if (func->getReturnType() != mRuntime->typeMemberPointerLLVMType()) return false;
	mFirstFunction = func;
	return true;
}

bool TypeValueType::setLastFunction(llvm::Function *func) {
	if (func->arg_size() != 1) return false;
	llvm::Function::arg_iterator i = func->arg_begin();
	if (i->getType() != mRuntime->typeLLVMType()->getPointerTo()) return false;
	if (func->getReturnType() != mRuntime->typeMemberPointerLLVMType()) return false;
	mLastFunction = func;
	return true;
}

bool TypeValueType::setBeforeFunction(llvm::Function *func) {
	if (func->arg_size() != 1) return false;
	llvm::Function::arg_iterator i = func->arg_begin();
	if (i->getType() != mRuntime->typeMemberPointerLLVMType()) return false;
	if (func->getReturnType() != mRuntime->typeMemberPointerLLVMType()) return false;
	mBeforeFunction = func;
	return true;
}

bool TypeValueType::setAfterFunction(llvm::Function *func) {
	if (func->arg_size() != 1) return false;
	llvm::Function::arg_iterator i = func->arg_begin();
	if (i->getType() != mRuntime->typeMemberPointerLLVMType()) return false;
	if (func->getReturnType() != mRuntime->typeMemberPointerLLVMType()) return false;
	mAfterFunction = func;
	return true;
}

