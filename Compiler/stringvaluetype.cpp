#include "stringvaluetype.h"
#include "value.h"
#include "stringpool.h"

StringValueType::StringValueType(StringPool *strPool, Runtime *r, llvm::Module *mod) :
	ValueType(r),
	mStringPool(strPool){
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
		case Boolean:
			return 2;
		default:
			return maxCastCost;
	}

	return 0;
}



Value StringValueType::cast(llvm::IRBuilder<> *builder, const Value &v) const {
	return Value();
}

llvm::Value *StringValueType::constructString(llvm::IRBuilder<> *builder, llvm::Value *globalStrPtr) {
	return builder->CreateCall(mConstructFunction, globalStrPtr);
}

llvm::Value *StringValueType::constructString(llvm::IRBuilder<> *builder, QString *str) {
	llvm::Value *globalStr = mStringPool->globalString(builder, str);
	return constructString(builder, globalStr);
}

void StringValueType::destructString(llvm::IRBuilder<> *builder, llvm::Value *stringVar) {
	builder->CreateCall(mDestructFunction, stringVar);
}

llvm::Value *StringValueType::stringToIntCast(llvm::IRBuilder<> *builder, llvm::Value *str) {
	return builder->CreateCall(mStringToIntFunction, str);
}

llvm::Value *StringValueType::stringToFloatCast(llvm::IRBuilder<> *builder, llvm::Value *str) {
	return builder->CreateCall(mStringToFloatFunction, str);
}

llvm::Value *StringValueType::intToStringCast(llvm::IRBuilder<> *builder, llvm::Value *i) {
	return builder->CreateCall(mIntToStringFunction, i);
}

llvm::Value *StringValueType::floatToStringCast(llvm::IRBuilder<> *builder, llvm::Value *f) {
	return builder->CreateCall(mFloatToStringFunction, f);
}

llvm::Value *StringValueType::stringAddition(llvm::IRBuilder<> *builder, llvm::Value *str1, llvm::Value *str2) {
	return builder->CreateCall2(mAdditionFunction, str1, str2);
}














