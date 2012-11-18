#include "stringvaluetype.h"
#include "value.h"

StringValueType::StringValueType(llvm::Module *mod) {
}

bool StringValueType::setConstructFunction(llvm::Function *func) {
	llvm::FunctionType *funcTy = func->getFunctionType();
	if (funcTy->getNumParams() != 1) return false;
	if (funcTy->getReturnType() != mType) return false;

	llvm::FunctionType::param_iterator i = funcTy->param_begin();
	const llvm::Type *const arg1 = *i;
	if (arg1 != llvm::Type::getInt8PtrTy(func->getContext())) return false;

	mConstructFunction = func;
	return true;
}

bool StringValueType::setAssignmentFunction(llvm::Function *func) {
	llvm::FunctionType *funcTy = func->getFunctionType();
	if (funcTy->getReturnType() != llvm::Type::getVoidTy(func->getContext())) return false;
	if (funcTy->getNumParams() != 2) return false;
	llvm::FunctionType::param_iterator i = funcTy->param_begin();
	const llvm::Type *const arg1 = *i;
	if (arg1 != mType->getPointerTo()) return false;
	i++;
	const llvm::Type *const arg2 = *i;
	if (arg2 != mType) return false;

	mAssignmentFunction = func;
	return true;
}

bool StringValueType::setDestructFunction(llvm::Function *func) {
	llvm::FunctionType *funcTy = func->getFunctionType();
	if (funcTy->getReturnType() != llvm::Type::getVoidTy(func->getContext())) return false;
	if (funcTy->getNumParams() != 1) return false;
	llvm::FunctionType::param_iterator i = funcTy->param_begin();
	const llvm::Type *const arg1 = *i;
	if (arg1 != mType->getPointerTo()) return false;
	mDestructFunction = func;
	return true;
}

bool StringValueType::setAdditionFunction(llvm::Function *func) {
	llvm::FunctionType *funcTy = func->getFunctionType();
	if (funcTy->getReturnType() != mType) return false;
	if (funcTy->getNumParams() != 2) return false;
	llvm::FunctionType::param_iterator i = funcTy->param_begin();
	const llvm::Type *const arg1 = *i;
	if (arg1 != mType) return false;
	i++;
	const llvm::Type *const arg2 = *i;
	if (arg2 != mType) return false;

	mAdditionFunction = func;
	return true;
}

bool StringValueType::setFloatToStringFunction(llvm::Function *func) {
	llvm::FunctionType *funcTy = func->getFunctionType();
	if (funcTy->getReturnType() != mType) return false;
	if (funcTy->getNumParams() != 1) return false;
	llvm::FunctionType::param_iterator i = funcTy->param_begin();
	const llvm::Type *const arg1 = *i;
	if (arg1 != llvm::Type::getFloatTy(func->getContext())) return false;
	mFloatToStringFunction = func;
	return true;
}

bool StringValueType::setIntToStringFunction(llvm::Function *func){
	llvm::FunctionType *funcTy = func->getFunctionType();
	if (funcTy->getReturnType() != mType) return false;
	if (funcTy->getNumParams() != 1) return false;
	llvm::FunctionType::param_iterator i = funcTy->param_begin();
	const llvm::Type *const arg1 = *i;
	if (arg1 != llvm::Type::getInt32Ty(func->getContext())) return false;
	mIntToStringFunction = func;
	return true;
}

bool StringValueType::setStringToIntFunction(llvm::Function *func) {
	llvm::FunctionType *funcTy = func->getFunctionType();
	if (funcTy->getReturnType() != llvm::Type::getInt32Ty(func->getContext())) return false;
	if (funcTy->getNumParams() != 1) return false;
	llvm::FunctionType::param_iterator i = funcTy->param_begin();
	const llvm::Type *const arg1 = *i;
	if (arg1 != mType) return false;
	mStringToIntFunction = func;
	return true;
}

bool StringValueType::setStringToFloatFunction(llvm::Function *func) {
	llvm::FunctionType *funcTy = func->getFunctionType();
	if (funcTy->getReturnType() != llvm::Type::getFloatTy(func->getContext())) return false;
	if (funcTy->getNumParams() != 1) return false;
	llvm::FunctionType::param_iterator i = funcTy->param_begin();
	const llvm::Type *const arg1 = *i;
	if (arg1 != mType) return false;
	mStringToFloatFunction = func;
	return true;
}

ValueType::CastCostType StringValueType::castCost(ValueType *to) const {
	switch (to->type()) {
		case String:
			return 0;
		case Integer:
			return 50;
		case Float:
			return 50;
		case Short:
			return 100;
		case Byte:
			return 100;
		default:
			return maxCastCost;
	}

	return 0;
}


Value StringValueType::cast(const Value &v) const {
	return Value();
}
