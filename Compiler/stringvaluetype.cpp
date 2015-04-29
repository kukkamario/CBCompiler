#include "stringvaluetype.h"
#include "value.h"
#include "stringpool.h"
#include "booleanvaluetype.h"
#include "builder.h"

StringValueType::StringValueType(StringPool *strPool, Runtime *r) :
	ValueType(r),
	mStringPool(strPool){

}

void StringValueType::setStringType(llvm::Type *t) {
	mType = t;
	mSharedDataPointerType = t->getContainedType(0);
	mStringDataPointerType = mSharedDataPointerType->getContainedType(0);
}

bool StringValueType::setConstructFunction(llvm::Function *func) {
	llvm::FunctionType *funcTy = func->getFunctionType();
	if (funcTy->getNumParams() != 1) return false;
	if (funcTy->getReturnType() != mStringDataPointerType) return false;

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
	if (arg1 != mStringDataPointerType->getPointerTo()) return false;
	i++;
	const llvm::Type *const arg2 = *i;
	if (arg2 != mStringDataPointerType) return false;

	mAssignmentFunction = func;
	return true;
}

bool StringValueType::setDestructFunction(llvm::Function *func) {
	llvm::FunctionType *funcTy = func->getFunctionType();
	if (funcTy->getReturnType() != llvm::Type::getVoidTy(func->getContext())) return false;
	if (funcTy->getNumParams() != 1) return false;
	llvm::FunctionType::param_iterator i = funcTy->param_begin();
	const llvm::Type *const arg1 = *i;
	if (arg1 != mStringDataPointerType) return false;
	mDestructFunction = func;
	return true;
}

bool StringValueType::setAdditionFunction(llvm::Function *func) {
	llvm::FunctionType *funcTy = func->getFunctionType();
	if (funcTy->getReturnType() != mStringDataPointerType) return false;
	if (funcTy->getNumParams() != 2) return false;
	llvm::FunctionType::param_iterator i = funcTy->param_begin();
	const llvm::Type *const arg1 = *i;
	if (arg1 != mStringDataPointerType) return false;
	i++;
	const llvm::Type *const arg2 = *i;
	if (arg2 != mStringDataPointerType) return false;

	mAdditionFunction = func;
	return true;
}

bool StringValueType::setFloatToStringFunction(llvm::Function *func) {
	llvm::FunctionType *funcTy = func->getFunctionType();
	if (funcTy->getReturnType() != mStringDataPointerType) return false;
	if (funcTy->getNumParams() != 1) return false;
	llvm::FunctionType::param_iterator i = funcTy->param_begin();
	const llvm::Type *const arg1 = *i;
	if (arg1 != llvm::Type::getFloatTy(func->getContext())) return false;
	mFloatToStringFunction = func;
	return true;
}

bool StringValueType::setIntToStringFunction(llvm::Function *func){
	llvm::FunctionType *funcTy = func->getFunctionType();
	if (funcTy->getReturnType() != mStringDataPointerType) return false;
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
	if (arg1 != mStringDataPointerType) return false;
	mStringToIntFunction = func;
	return true;
}

bool StringValueType::setStringToFloatFunction(llvm::Function *func) {
	llvm::FunctionType *funcTy = func->getFunctionType();
	if (funcTy->getReturnType() != llvm::Type::getFloatTy(func->getContext())) return false;
	if (funcTy->getNumParams() != 1) return false;
	llvm::FunctionType::param_iterator i = funcTy->param_begin();
	const llvm::Type *const arg1 = *i;
	if (arg1 != mStringDataPointerType) return false;
	mStringToFloatFunction = func;
	return true;
}

bool StringValueType::setEqualityFunction(llvm::Function *func) {
	llvm::FunctionType *funcTy = func->getFunctionType();
	if (funcTy->getReturnType() != mRuntime->booleanValueType()->llvmType()) return false;
	if (funcTy->getNumParams() != 2) return false;
	llvm::FunctionType::param_iterator i = funcTy->param_begin();
	const llvm::Type *const arg1 = *i;
	if (arg1 != mStringDataPointerType) return false;
	i++;
	const llvm::Type *const arg2 = *i;
	if (arg2 != mStringDataPointerType) return false;

	mEqualityFunction = func;
	return true;
}

bool StringValueType::setRefFunction(llvm::Function *func) {
	llvm::FunctionType *funcTy = func->getFunctionType();
	if (funcTy->getReturnType() != llvm::Type::getVoidTy(func->getContext())) return false;
	if (funcTy->getNumParams() != 1) return false;
	llvm::FunctionType::param_iterator i = funcTy->param_begin();
	const llvm::Type *const arg1 = *i;
	if (arg1 != mStringDataPointerType) return false;
	i++;

	mRefFunction = func;
	return true;
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
			return ccCastToBoolean;
		default:
			return ccNoCast;
	}
}



Value StringValueType::cast(Builder *builder, const Value &v) const {
	return builder->toString(v);
}

int StringValueType::size() const {
	return mRuntime->dataLayout().getTypeStoreSize(mType);
}

llvm::Constant *StringValueType::defaultValue() const {
	return llvm::Constant::getNullValue(mType);
}

llvm::Value *StringValueType::stringDataPointerToString(llvm::IRBuilder<> *builder, llvm::Value *ptr) const {
	unsigned idx[1] = {0};
	llvm::Value *sharedDataPointer = builder->CreateInsertValue(llvm::UndefValue::get(mSharedDataPointerType), ptr, idx);
	return builder->CreateInsertValue(llvm::UndefValue::get(mType), sharedDataPointer, idx);
}

llvm::Value *StringValueType::stringToStringDataPointer(llvm::IRBuilder<> *builder, llvm::Value *str) const {
	unsigned idx[1] = {0};

	return builder->CreateExtractValue(builder->CreateExtractValue(str, idx), idx);
}

llvm::Value *StringValueType::stringPointerToStringDataPointer(llvm::IRBuilder<> *builder, llvm::Value *str) const {
	llvm::Value *idx[] = {builder->getInt32(0), builder->getInt32(0), builder->getInt32(0)};
	return builder->CreateLoad(builder->CreateGEP(str, idx));
}

llvm::Value *StringValueType::constructString(llvm::IRBuilder<> *builder, llvm::Value *globalStrPtr) {
	llvm::Value *c = builder->CreateCall(mConstructFunction, globalStrPtr);
	return stringDataPointerToString(builder, c);
}

void StringValueType::destructString(llvm::IRBuilder<> *builder, llvm::Value *str) {
	builder->CreateCall(mDestructFunction, stringToStringDataPointer(builder, str));
}

llvm::Value *StringValueType::stringToIntCast(llvm::IRBuilder<> *builder, llvm::Value *str) {
	return builder->CreateCall(mStringToIntFunction, stringToStringDataPointer(builder, str));
}

llvm::Value *StringValueType::stringToFloatCast(llvm::IRBuilder<> *builder, llvm::Value *str) {
	return builder->CreateCall(mStringToFloatFunction, stringToStringDataPointer(builder, str));
}

llvm::Value *StringValueType::intToStringCast(llvm::IRBuilder<> *builder, llvm::Value *i) {
	llvm::Value *c = builder->CreateCall(mIntToStringFunction, i);
	return stringDataPointerToString(builder, c);
}

llvm::Value *StringValueType::floatToStringCast(llvm::IRBuilder<> *builder, llvm::Value *f) {
	llvm::Value *c = builder->CreateCall(mFloatToStringFunction, f);
	return stringDataPointerToString(builder, c);
}

llvm::Value *StringValueType::stringAddition(llvm::IRBuilder<> *builder, llvm::Value *str1, llvm::Value *str2) {
	llvm::Value *c = builder->CreateCall2(mAdditionFunction, stringToStringDataPointer(builder, str1), stringToStringDataPointer(builder, str2));
	return  stringDataPointerToString(builder, c);
}

llvm::Value *StringValueType::stringEquality(llvm::IRBuilder<> *builder, llvm::Value *a, llvm::Value *b) {
	return builder->CreateCall2(mEqualityFunction, stringToStringDataPointer(builder, a), stringToStringDataPointer(builder, b));
}

void StringValueType::refString(llvm::IRBuilder<> *builder, llvm::Value *str) const {
	builder->CreateCall(mRefFunction, stringToStringDataPointer(builder, str));
}

void StringValueType::assignString(llvm::IRBuilder<> *builder, llvm::Value *var, llvm::Value *val) {
	builder->CreateCall2(mAssignmentFunction, builder->CreateBitCast(var, mStringDataPointerType->getPointerTo()), stringToStringDataPointer(builder, val));
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

Value StringValueType::generateLoad(Builder *builder, const Value &var) const {
	assert(var.isReference());
	llvm::Value *v = builder->irBuilder().CreateLoad(var.value());
	refString(&builder->irBuilder(), v);
	return Value(var.valueType(), v, false);
}


bool StringValueType::isValid() const {
	return mAdditionFunction && mAssignmentFunction && mType && mIntToStringFunction && mFloatToStringFunction && mStringToFloatFunction && mStringToIntFunction;
}














