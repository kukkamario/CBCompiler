#include "stringvaluetype.h"
#include "value.h"
#include "stringpool.h"
#include "booleanvaluetype.h"
#include "builder.h"

StringValueType::StringValueType(StringPool *strPool, Runtime *r) :
	ValueType(r),
	mStringPool(strPool){
}

bool StringValueType::setConstructFunction(llvm::Function *func) {
	llvm::FunctionType *funcTy = func->getFunctionType();
	if (funcTy->getNumParams() != 1) return false;
	if (funcTy->getReturnType() != mType) return false;

	llvm::FunctionType::param_iterator i = funcTy->param_begin();
	const llvm::Type *const arg1 = *i;
	if (arg1 != llvm::Type::getInt32PtrTy(func->getContext())) return false;

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
	if (arg1 != mType) return false;
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

bool StringValueType::setEqualityFunction(llvm::Function *func) {
	llvm::FunctionType *funcTy = func->getFunctionType();
	if (funcTy->getReturnType() != mRuntime->booleanValueType()->llvmType()) return false;
	if (funcTy->getNumParams() != 2) return false;
	llvm::FunctionType::param_iterator i = funcTy->param_begin();
	const llvm::Type *const arg1 = *i;
	if (arg1 != mType) return false;
	i++;
	const llvm::Type *const arg2 = *i;
	if (arg2 != mType) return false;

	mEqualityFunction = func;
	return true;
}

bool StringValueType::setRefFunction(llvm::Function *func) {
	llvm::FunctionType *funcTy = func->getFunctionType();
	if (funcTy->getReturnType() != llvm::Type::getVoidTy(func->getContext())) return false;
	if (funcTy->getNumParams() != 1) return false;
	llvm::FunctionType::param_iterator i = funcTy->param_begin();
	const llvm::Type *const arg1 = *i;
	if (arg1 != mType) return false;
	i++;

	mRefFunction = func;
	return true;
}

void StringValueType::assignString(llvm::IRBuilder<> *builder, llvm::Value *var, llvm::Value *string) {
	builder->CreateCall2(mAssignmentFunction, var, string);
}


ValueType::CastCost StringValueType::castingCostToOtherValueType(const ValueType *to) const {
	switch (to->basicType()) {
		case String:
			return ccNoCost;
		case Integer:
			return ccCastFromString;
		case Float:
			return ccCastFromString;
		case Short:
			return ccCastFromString;
		case Byte:
			return ccCastFromString;
		case Boolean:
			return ccCastToSmaller;
		default:
			return ccNoCast;
	}
}



Value StringValueType::cast(Builder *builder, const Value &v) const {
	return builder->toString(v);
}

int StringValueType::size() const {
	switch (mRuntime->module()->getPointerSize()) {
		case llvm::Module::Pointer32:
			return 4;
		case llvm::Module::Pointer64:
			return 8;
		default:
			assert("Unknown pointer size" && 0);
			return false;
	}
}

llvm::Constant *StringValueType::defaultValue() const {
	return llvm::ConstantPointerNull::get(static_cast<llvm::PointerType*>(mType));
}

llvm::Value *StringValueType::constructString(llvm::IRBuilder<> *builder, llvm::Value *globalStrPtr) {
	return builder->CreateCall(mConstructFunction, globalStrPtr);
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

llvm::Value *StringValueType::stringEquality(llvm::IRBuilder<> *builder, llvm::Value *a, llvm::Value *b) {
	return builder->CreateCall2(mEqualityFunction, a, b);
}

void StringValueType::refString(llvm::IRBuilder<> *builder, llvm::Value *a) {
	builder->CreateCall(mRefFunction, a);
}

Value StringValueType::generateOperation(Builder *builder, int opType, const Value &operand1, const Value &operand2, OperationFlags &operationFlags) const {
	return generateBasicTypeOperation(builder, opType, operand1, operand2, operationFlags);
}

Value StringValueType::generateOperation(Builder *builder, int opType, const Value &operand, OperationFlags &operationFlags) const {
	return generateBasicTypeOperation(builder, opType, operand, operationFlags);
}

void StringValueType::generateDestructor(Builder *builder, const Value &value) {
	if (value.isConstant() || value.isReference()) return;
	destructString(&builder->irBuilder(), value.value());
}


bool StringValueType::isValid() const {
	return mAdditionFunction && mAssignmentFunction && mType && mIntToStringFunction && mFloatToStringFunction && mStringToFloatFunction && mStringToIntFunction;
}














