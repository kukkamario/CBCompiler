#include "builder.h"
#include "stringvaluetype.h"
#include "stringpool.h"
#include "intvaluetype.h"
#include "shortvaluetype.h"
#include "floatvaluetype.h"
#include "bytevaluetype.h"
#include "typepointervaluetype.h"
#include "booleanvaluetype.h"
#include "variablesymbol.h"
#include "typesymbol.h"
#include "typevaluetype.h"
#include "functionvaluetype.h"
#include <QDebug>

Builder::Builder(llvm::LLVMContext &context) :
	mIRBuilder(context),
	mRuntime(0),
	mStringPool(0) {
}

void Builder::setRuntime(Runtime *r) {
	mRuntime = r;

	//double    @llvm.pow.f64(double %Val, double %Power)
	llvm::Type *params[2] = {mIRBuilder.getDoubleTy(), mIRBuilder.getDoubleTy()};
	llvm::FunctionType *funcTy = llvm::FunctionType::get(mIRBuilder.getDoubleTy(), params, false);
	mPowFF = llvm::cast<llvm::Function>(r->module()->getOrInsertFunction("llvm.pow.f64", funcTy));

	//double    @llvm.powi.f64(double %Val, i32 %power)
	params[1] = mIRBuilder.getInt32Ty();
	funcTy = llvm::FunctionType::get(mIRBuilder.getDoubleTy(), params, false);
	mPowFI = llvm::cast<llvm::Function>(r->module()->getOrInsertFunction("llvm.powi.f64", funcTy));

	assert(mPowFF && mPowFI);
}

void Builder::setStringPool(StringPool *s) {
	mStringPool = s;
}

void Builder::setInsertPoint(llvm::BasicBlock *basicBlock) {
	mIRBuilder.SetInsertPoint(basicBlock);
}

Value Builder::toInt(const Value &v) {
	if (v.valueType()->basicType() == ValueType::Integer) return v;
	if (v.isConstant()) {
		return Value(ConstantValue(v.constant().toInt()), mRuntime);
	}
	assert(v.value());
	switch (v.valueType()->basicType()) {
		case ValueType::Float: {
			llvm::Value *r = mIRBuilder.CreateFAdd(llvmValue(v), llvm::ConstantFP::get(mIRBuilder.getFloatTy(), 0.5));
			return Value(mRuntime->intValueType(), mIRBuilder.CreateFPToSI(r,mIRBuilder.getInt32Ty()), false);
		}
		case ValueType::Boolean:
		case ValueType::Byte:
		case ValueType::Short: {
			llvm::Value *r = mIRBuilder.CreateCast(llvm::CastInst::ZExt, llvmValue(v), mIRBuilder.getInt32Ty());
			return Value(mRuntime->intValueType(), r, false);
		}
		case ValueType::String: {
			llvm::Value *i = mRuntime->stringValueType()->stringToIntCast(&mIRBuilder, llvmValue(v));
			return Value(mRuntime->intValueType(), i, false);
		}
		default: break;
	}
	assert("Invalid value" && 0);
	return Value();
}

Value Builder::toFloat(const Value &v) {
	if (v.valueType()->basicType() == ValueType::Float) return v;
	if (v.isConstant()) {
		return Value(ConstantValue(v.constant().toFloat()), mRuntime);
	}
	switch(v.valueType()->basicType()) {
		case ValueType::Integer:
			return Value(mRuntime->floatValueType(),mIRBuilder.CreateSIToFP(llvmValue(v), mIRBuilder.getFloatTy()), false);
		case ValueType::Boolean:
		case ValueType::Short:
		case ValueType::Byte:
			return Value(mRuntime->floatValueType(),mIRBuilder.CreateUIToFP(llvmValue(v), mIRBuilder.getFloatTy()), false);
		case ValueType::String: {
			llvm::Value *val = mRuntime->stringValueType()->stringToFloatCast(&mIRBuilder, llvmValue(v));
			return Value(mRuntime->floatValueType(), val, false);
		}
		default: break;
	}
	assert("Invalid value" && 0);
	return Value();
}

Value Builder::toString(const Value &v) {
	if (v.valueType()->basicType() == ValueType::String) return v;
	if (v.isConstant()) {
		return Value(ConstantValue(v.constant().toString()), mRuntime);
	}
	switch (v.valueType()->basicType()) {
		case ValueType::Integer:
		case ValueType::Boolean:
		case ValueType::Short:
		case ValueType::Byte: {
			Value i = toInt(v);
			llvm::Value *val = mRuntime->stringValueType()->intToStringCast(&mIRBuilder, llvmValue(v));
			return Value(mRuntime->stringValueType(), val, false);
		}
		case ValueType::Float: {
			llvm::Value *val = mRuntime->stringValueType()->floatToStringCast(&mIRBuilder, llvmValue(v));
			return Value(mRuntime->stringValueType(), val, false);
		}
		default: break;
	}

	assert("Invalid value" && 0);
	return Value();
}

Value Builder::toShort(const Value &v) {
	if (v.valueType()->basicType() == ValueType::Short) return v;
	if (v.isConstant()) {
		return Value(ConstantValue(v.constant().toShort()), mRuntime);
	}
	assert(v.value());
	switch (v.valueType()->basicType()) {
		case ValueType::Float: {
			llvm::Value *r = mIRBuilder.CreateAdd(llvmValue(v), llvm::ConstantFP::get(mIRBuilder.getFloatTy(), 0.5));
			return Value(mRuntime->intValueType(), mIRBuilder.CreateFPToSI(r,mIRBuilder.getInt16Ty()), false);
		}
		case ValueType::Boolean:
		case ValueType::Byte: {
			llvm::Value *r = mIRBuilder.CreateCast(llvm::CastInst::ZExt, llvmValue(v), mIRBuilder.getInt16Ty());
			return Value(mRuntime->intValueType(), r, false);
		}
		case ValueType::Integer: {
			llvm::Value *r = mIRBuilder.CreateCast(llvm::CastInst::Trunc, llvmValue(v), mIRBuilder.getInt16Ty());
			return Value(mRuntime->intValueType(), r, false);
		}

		case ValueType::String: {
			llvm::Value *i = mRuntime->stringValueType()->stringToIntCast(&mIRBuilder, llvmValue(v));
			return toShort(Value(mRuntime->intValueType(), i, false));
		}
		default: break;
	}
	assert("Invalid value" && 0);
	return Value();
}

Value Builder::toByte(const Value &v) {
	if (v.valueType()->basicType() == ValueType::Byte) return v;
	if (v.isConstant()) {
		return Value(ConstantValue(v.constant().toShort()), mRuntime);
	}
	assert(v.value());
	switch (v.valueType()->basicType()) {
		case ValueType::Float: {
			llvm::Value *r = mIRBuilder.CreateAdd(llvmValue(v), llvm::ConstantFP::get(mIRBuilder.getFloatTy(), 0.5));
			return Value(mRuntime->intValueType(), mIRBuilder.CreateFPToSI(r,mIRBuilder.getInt16Ty()), false);
		}
		case ValueType::Boolean: {
			llvm::Value *r = mIRBuilder.CreateCast(llvm::CastInst::ZExt, llvmValue(v), mIRBuilder.getInt16Ty());
			return Value(mRuntime->intValueType(), r, false);
		}
		case ValueType::Integer:
		case ValueType::Short: {
			llvm::Value *r = mIRBuilder.CreateCast(llvm::CastInst::Trunc, llvmValue(v), mIRBuilder.getInt16Ty());
			return Value(mRuntime->intValueType(), r, false);
		}

		case ValueType::String: {
			llvm::Value *i = mRuntime->stringValueType()->stringToIntCast(&mIRBuilder, llvmValue(v));
			return toShort(Value(mRuntime->intValueType(), i, false));
		}
		default: break;
	}
	assert("Invalid value" && 0);
	return Value();
}

Value Builder::toBoolean(const Value &v) {
	if (v.valueType()->basicType() == ValueType::Boolean) {
		return v;
	}
	if (v.isConstant()) {
		return Value(ConstantValue(v.constant().toBool()), mRuntime);
	}
	switch(v.valueType()->basicType()) {
		case ValueType::Integer:
			return Value(mRuntime->booleanValueType(), mIRBuilder.CreateICmpNE(llvmValue(v), mIRBuilder.getInt32(0)), false);
		case ValueType::Short:
			return Value(mRuntime->booleanValueType(), mIRBuilder.CreateICmpNE(llvmValue(v), mIRBuilder.getInt16(0)), false);
		case ValueType::Byte:
			return Value(mRuntime->booleanValueType(), mIRBuilder.CreateICmpNE(llvmValue(v), mIRBuilder.getInt8(0)), false);
		case ValueType::Float:
			return Value(mRuntime->booleanValueType(), mIRBuilder.CreateFCmpONE(llvmValue(v), llvm::ConstantFP::get(mIRBuilder.getFloatTy(), 0.0)), false);
		case ValueType::String: {
			llvm::Value *val = mIRBuilder.CreateIsNotNull(llvmValue(v));

			return Value(mRuntime->booleanValueType(), val, false);
		}
		default: break;
	}
	assert("Invalid value" && 0);
	return Value();
}

Value Builder::toValueType(ValueType *to, const Value &v) {
	return to->cast(this, v);
}

llvm::Value *Builder::llvmValue(const Value &v) {
	if (v.isConstant()) {
		return llvmValue(v.constant());
	}

	assert(v.value());
	if (v.isReference()) {
		return mIRBuilder.CreateLoad(v.value());
	}
	return v.value();
}

llvm::Value *Builder::llvmValue(int i) {
	return mIRBuilder.getInt32(i);
}

llvm::Value *Builder::llvmValue(uint16_t i) {
	return mIRBuilder.getInt16(i);
}

llvm::Value *Builder::llvmValue(uint8_t i) {
	return mIRBuilder.getInt8(i);
}

llvm::Value *Builder::llvmValue(float i) {
	return llvm::ConstantFP::get(mIRBuilder.getFloatTy(), (double)i);
}

llvm::Value *Builder::llvmValue(const QString &s) {
	if (s.isEmpty()) {
		return llvm::ConstantPointerNull::get((llvm::PointerType*)mRuntime->stringValueType()->llvmType());
	}
	return mStringPool->globalString(this, s).value();
}

llvm::Value *Builder::llvmValue(bool t) {
	return mIRBuilder.getInt1(t);
}

llvm::Value *Builder::llvmValue(const ConstantValue &v) {
	switch(v.type()) {
		case ConstantValue::Integer:
			return llvmValue(v.toInt());
		case ConstantValue::Float:
			return llvmValue(v.toFloat());
		case ConstantValue::Short:
			return llvmValue(v.toShort());
		case ConstantValue::Byte:
			return llvmValue(v.toByte());
		case ConstantValue::Boolean:
			return llvmValue(v.toBool());
		case ConstantValue::String: {
			return llvmValue(v.toString());
		case ConstantValue::Null:
			return mRuntime->typePointerCommonValueType()->defaultValue();
		}
		default: break;
	}
	assert("Invalid constant value" && 0);
	return 0;
}

llvm::Value *Builder::bitcast(llvm::Type *type, llvm::Value *val) {
	return mIRBuilder.CreateBitCast(val, type);
}


Value Builder::call(Function *func, QList<Value> &params) {
	Function::ParamList paramTypes = func->paramTypes();
	assert(func->requiredParams() >= params.size() && params.size() <= paramTypes.size());
	Function::ParamList::ConstIterator pi = paramTypes.begin();
	for (QList<Value>::Iterator i = params.begin(); i != params.end(); ++i) {
		//Cast to the right value type
		*i = (*pi)->cast(this, *i);
		//string destruction hack
		i->toLLVMValue(this);
		pi++;
	}
	Value ret = func->call(this, params);
	for (QList<Value>::ConstIterator i = params.begin(); i != params.end(); ++i) {
		destruct(*i);
	}
	return ret;
}

Value Builder::call(FunctionValueType *funcType, llvm::Value *func, QList<Value> &params) {
	QList<ValueType*> paramTypes = funcType->paramTypes();
	QList<ValueType*> ::ConstIterator pi = paramTypes.begin();
	std::vector<llvm::Value*> args;
	for (QList<Value>::Iterator i = params.begin(); i != params.end(); ++i) {
		//Cast to the right value type
		*i = (*pi)->cast(this, *i);
		//string destruction hack
		i->toLLVMValue(this);
		pi++;
		args.push_back(llvmValue(*i));
	}

	Value ret = Value(funcType->returnType(), mIRBuilder.CreateCall(func, args), false);
	for (QList<Value>::ConstIterator i = params.begin(); i != params.end(); ++i) {
		destruct(*i);
	}
	return ret;
}

void Builder::branch(llvm::BasicBlock *dest) {
	mIRBuilder.CreateBr(dest);
}


void Builder::branch(const Value &cond, llvm::BasicBlock *ifTrue, llvm::BasicBlock *ifFalse) {
	mIRBuilder.CreateCondBr(llvmValue(toBoolean(cond)), ifTrue, ifFalse);
}

void Builder::returnValue(ValueType *retType, const Value &v) {
	mIRBuilder.CreateRet(llvmValue(retType->cast(this, v)));
}

void Builder::returnVoid() {
	mIRBuilder.CreateRetVoid();
}

Value Builder::defaultValue(ValueType *valType) {
	return Value(valType, valType->defaultValue(), false);
}

void Builder::construct(VariableSymbol *var) {
	llvm::Value *allocaInst = mIRBuilder.CreateAlloca(var->valueType()->llvmType());
	var->setAlloca(allocaInst);
	mIRBuilder.CreateStore(var->valueType()->defaultValue(), allocaInst);
}

void Builder::store(const Value &ref, const Value &value) {
	assert(ref.isReference());
	assert(ref.valueType() == value.valueType());
	mIRBuilder.CreateStore(llvmValue(value), ref.value());
}

void Builder::store(llvm::Value *ptr, llvm::Value *val) {
	assert(ptr->getType() == val->getType()->getPointerTo());
	mIRBuilder.CreateStore(val, ptr);
}

void Builder::store(llvm::Value *ptr, const Value &v) {
	assert(ptr->getType() == v.valueType()->llvmType()->getPointerTo());
	if (v.valueType()->basicType() == ValueType::String) {
		mRuntime->stringValueType()->assignString(&mIRBuilder, ptr, llvmValue(v));
	}
	else {
		mIRBuilder.CreateStore(llvmValue(v), ptr);
	}
}

void Builder::store(VariableSymbol *var, const Value &v) {
	store(var, llvmValue(var->valueType()->cast(this, v)));
}

void Builder::store(VariableSymbol *var, llvm::Value *val) {
	assert(var->valueType()->llvmType() == val->getType());
	if (var->valueType()->basicType() == ValueType::String) {
		mRuntime->stringValueType()->assignString(&mIRBuilder, var->alloca_(), val);
	}
	else {
		store(var->alloca_(), val);
	}
}

/*
void Builder::store(const Value &ref, const Value &index, const Value &val) {
	store(arrayElementPointer(array, index), array->valueType()->cast(this, val));
}

void Builder::store(const Value &ref, const QList<Value> &dims, const Value &val) {
	store(arrayElementPointer(ref, dims), ref->valueType()->cast(this, val));
}*/


Value Builder::load(const VariableSymbol *var) {
	llvm::Value *val = mIRBuilder.CreateLoad(var->alloca_(), false);
	if (var->valueType()->basicType() == ValueType::String) {
		mRuntime->stringValueType()->refString(&mIRBuilder, val);
	}
	return Value(var->valueType(), val, false);
}

/*Value Builder::load(const Value &ref, const Value &index) {
	return Value(ref->valueType(), mIRBuilder.CreateLoad(arrayElementPointer(ref, index)), false);
}

Value Builder::load(const Value &ref, const QList<Value> &dims) {
	return Value(array->valueType(), mIRBuilder.CreateLoad(arrayElementPointer(array, dims)), false);
}*/

void Builder::destruct(VariableSymbol *var) {
	if (var->valueType()->basicType() == ValueType::String) {
		llvm::Value *str = mIRBuilder.CreateLoad(var->alloca_(), false);
		mRuntime->stringValueType()->destructString(&mIRBuilder, str);
	}
}

void Builder::destruct(const Value &a) {
	if (!a.isValid()) return;
	if (a.isReference()) return;
	if (a.isConstant() || a.valueType()->basicType() != ValueType::String) return;
	assert(a.value());
	mRuntime->stringValueType()->destructString(&mIRBuilder, a.value());
}

Value Builder::nullTypePointer() {
	return Value(ConstantValue(ConstantValue::Null), mRuntime);
}

/*void Builder::initilizeArray(VariableSymbol *array, const QList<Value> &dimSizes) {
	assert(array->dimensions() == dimSizes.size());
	array->valueType()

	llvm::Value *elements = calculateArrayElementCount(dimSizes);
	int sizeOfElement = array->valueType()->size();
	llvm::Value *memSize = mIRBuilder.CreateMul(elements, llvmValue(sizeOfElement));

	llvm::Value *mem = mIRBuilder.CreateBitCast(allocate(memSize), array->valueType()->llvmType()->getPointerTo());
	memSet(mem, memSize, llvmValue(uint8_t(0)));


	mIRBuilder.CreateStore(mem, array->globalArrayData());
	mIRBuilder.CreateStore(elements, array->globalArraySize());
	fillArrayIndexMultiplierArray(array, dimSizes);
}



llvm::Value *Builder::calculateArrayElementCount(const QList<Value> &dimSizes) {

	QList<Value>::ConstIterator i = dimSizes.begin();
	llvm::Value *result = llvmValue(toInt(*i));
	for (++i; i != dimSizes.end(); i++) {
		result = mIRBuilder.CreateMul(result, llvmValue(toInt(*i)));
	}
	return result;
}

llvm::Value *Builder::calculateArrayMemorySize(ArraySymbol *array, const QList<Value> &dimSizes) {
	llvm::Value *elements = calculateArrayElementCount(dimSizes);
	int sizeOfElement = array->valueType()->size();
	return mIRBuilder.CreateMul(elements, llvmValue(sizeOfElement));
}

llvm::Value *Builder::arrayElementPointer(ArraySymbol *array, const QList<Value> &index) {
	assert(array->dimensions() == index.size());
	if (array->dimensions() == 1) {
		return mIRBuilder.CreateGEP(mIRBuilder.CreateLoad(array->globalArrayData()), llvmValue(toInt(index.first())));
	}
	else { // array->dimensions() > 1
		QList<Value>::ConstIterator i = index.begin();
		llvm::Value *arrIndex =  mIRBuilder.CreateMul(llvmValue(toInt(*i)), arrayIndexMultiplier(array, 0));
		int multIndex = 1;
		i++;
		for (QList<Value>::ConstIterator end = --index.end(); i != end; ++i) {
			arrIndex = mIRBuilder.CreateAdd(arrIndex, mIRBuilder.CreateMul(llvmValue(toInt(*i)), arrayIndexMultiplier(array, multIndex)));
			multIndex++;
		}
		arrIndex = mIRBuilder.CreateAdd(arrIndex, llvmValue(toInt(*i)));
		return mIRBuilder.CreateGEP(mIRBuilder.CreateLoad(array->globalArrayData()), arrIndex);
	}
}

llvm::Value *Builder::arrayElementPointer(ArraySymbol *array, const Value &index) {
	return mIRBuilder.CreateGEP(mIRBuilder.CreateLoad(array->globalArrayData()), llvmValue(toInt(index)));
}

llvm::Value *Builder::arrayIndexMultiplier(ArraySymbol *array, int index) {
	assert(index >= 0 && index < array->dimensions() - 1);
	llvm::Value *gepParams[2] = { llvmValue(0), llvmValue(index) };
	return mIRBuilder.CreateLoad(mIRBuilder.CreateGEP(array->globalIndexMultiplierArray(), gepParams));
}

void Builder::fillArrayIndexMultiplierArray(ArraySymbol *array, const QList<Value> &dimSizes) {
	assert(array->dimensions() == dimSizes.size());

	if (array->dimensions() > 1) {
		QList<Value>::ConstIterator i = --dimSizes.end();
		llvm::Value *multiplier = llvmValue(toInt(*i));
		int arrIndex = array->dimensions() - 2;
		while(i != dimSizes.begin()) {
			llvm::Value *gepParams[2] = { llvmValue(0), llvmValue(arrIndex) };
			llvm::Value *pointerToArrayElement = mIRBuilder.CreateGEP(array->globalIndexMultiplierArray(), gepParams);
			mIRBuilder.CreateStore(multiplier, pointerToArrayElement);

			--i;
			if (i != dimSizes.begin()) {
				multiplier = mIRBuilder.CreateMul(multiplier, llvmValue(toInt(*i)));
				--arrIndex;
			}
		}
	}
}*/


Value Builder::typePointerFieldReference(Value typePtrVar, const QString &fieldName) {
	assert(typePtrVar.valueType()->isTypePointer());
	TypePointerValueType *typePointerValueType = static_cast<TypePointerValueType*>(typePtrVar.valueType());
	TypeSymbol *type = typePointerValueType->typeSymbol();
	const TypeField &field = type->field(fieldName);
	int fieldIndex = type->fieldIndex(fieldName);
	llvm::Value *fieldPtr = mIRBuilder.CreateStructGEP(llvmValue(typePtrVar), fieldIndex);
	return Value(field.valueType(), fieldPtr, true);
}

Value Builder::newTypeMember(TypeSymbol *type) {
	llvm::Value *typePtr = mIRBuilder.CreateCall(mRuntime->typeValueType()->newFunction(), type->globalTypeVariable());
	return Value(type->typePointerValueType(), bitcast(type->typePointerValueType()->llvmType(), typePtr), false);
}

Value Builder::firstTypeMember(TypeSymbol *type) {
	llvm::Value *typePtr = mIRBuilder.CreateCall(mRuntime->typeValueType()->firstFunction(), type->globalTypeVariable());
	return Value(type->typePointerValueType(), bitcast(type->typePointerValueType()->llvmType(), typePtr), false);
}

Value Builder::lastTypeMember(TypeSymbol *type) {
	llvm::Value *typePtr = mIRBuilder.CreateCall(mRuntime->typeValueType()->lastFunction(), type->globalTypeVariable());
	return Value(type->typePointerValueType(), bitcast(type->typePointerValueType()->llvmType(), typePtr), false);
}

Value Builder::afterTypeMember(const Value &ptr) {
	assert(ptr.valueType()->isTypePointer());
	llvm::Value *param = bitcast(mRuntime->typePointerCommonValueType()->llvmType(), llvmValue(ptr));
	llvm::Value *typePtr = mIRBuilder.CreateCall(mRuntime->typeValueType()->afterFunction(), param);
	return Value(ptr.valueType(), bitcast(ptr.valueType()->llvmType(), typePtr), false);
}

Value Builder::beforeTypeMember(const Value &ptr) {
	assert(ptr.valueType()->isTypePointer());
	llvm::Value *param = bitcast(mRuntime->typePointerCommonValueType()->llvmType(), llvmValue(ptr));
	llvm::Value *typePtr = mIRBuilder.CreateCall(mRuntime->typeValueType()->beforeFunction(), param);
	return Value(ptr.valueType(), bitcast(ptr.valueType()->llvmType(), typePtr), false);
}

Value Builder::typePointerNotNull(const Value &ptr) {
	assert(ptr.valueType()->isTypePointer());
	return Value(mRuntime->booleanValueType(), mIRBuilder.CreateIsNotNull(llvmValue(ptr)), false);
}

llvm::GlobalVariable *Builder::createGlobalVariable(ValueType *type, bool isConstant, llvm::GlobalValue::LinkageTypes linkage, llvm::Constant *initializer, const llvm::Twine &name) {
	return createGlobalVariable(type->llvmType(), isConstant, linkage, initializer, name);
}

llvm::GlobalVariable *Builder::createGlobalVariable(llvm::Type *type, bool isConstant, llvm::GlobalValue::LinkageTypes linkage, llvm::Constant *initializer, const llvm::Twine &name) {
	return new llvm::GlobalVariable(*mRuntime->module(),type, isConstant, linkage, initializer, name);
}

llvm::LLVMContext &Builder::context() {
	return mRuntime->module()->getContext();
}

Value Builder::not_(const Value &a) {
	if (a.isConstant()) {
		OperationFlags flags;
		return Value(ConstantValue::not_(a.constant(), flags), mRuntime);
	}

	return Value(mRuntime->booleanValueType(), mIRBuilder.CreateNot(llvmValue(toBoolean(a))), false);
}

Value Builder::minus(const Value &a) {
	if (a.isConstant()) {
		OperationFlags flags;
		return Value(ConstantValue::minus(a.constant(), flags), mRuntime);
	}

	if (a.valueType()->basicType() == ValueType::Boolean) {
		return Value(mRuntime->intValueType(), mIRBuilder.CreateNeg(llvmValue(toInt(a))), false);
	}
	if (a.valueType()->basicType() == ValueType::Float) {
		return Value(a.valueType(), mIRBuilder.CreateFNeg(llvmValue(a)), false);
	}
	return Value(a.valueType(), mIRBuilder.CreateNeg(llvmValue(a)), false);
}

Value Builder::plus(const Value &a) {
	assert("Unimplemented operator" && 0); return Value();
}

Value Builder::add(const Value &a, const Value &b) {
	if (a.isConstant() && b.isConstant()) {
		OperationFlags flags;
		return Value(ConstantValue::add(a.constant(), b.constant(), flags), mRuntime);
	}
	llvm::Value *result;
	switch(a.valueType()->basicType()) {
		case ValueType::Boolean:
		case ValueType::Integer:
			switch(b.valueType()->basicType()) {
				case ValueType::Integer:
				case ValueType::Boolean:
					result = mIRBuilder.CreateAdd(llvmValue(toInt(a)), llvmValue(toInt(b)));
					return Value(mRuntime->intValueType(), result, false);
				case ValueType::Float:
					result = mIRBuilder.CreateFAdd(llvmValue(toFloat(a)), llvmValue(b));
					return Value(mRuntime->floatValueType(), result, false);
				case ValueType::Short:
				case ValueType::Byte:
					result = mIRBuilder.CreateAdd(llvmValue(toInt(a)), llvmValue(toInt(b)));
					return Value(mRuntime->intValueType(), result, false);
				case ValueType::String: {
					Value as = toString(a);
					result = mRuntime->stringValueType()->stringAddition(&mIRBuilder, llvmValue(as), llvmValue(b));
					destruct(as);
					return Value(mRuntime->stringValueType(), result, false);
				}
				default: break;
			}
			break;
		case ValueType::Float:
			switch(b.valueType()->basicType()) {
				case ValueType::Integer:
				case ValueType::Short:
				case ValueType::Byte:
				case ValueType::Boolean:
					result = mIRBuilder.CreateFAdd(llvmValue(a), llvmValue(toFloat(b)));
					return Value(mRuntime->floatValueType(), result, false);
				case ValueType::Float:
					result = mIRBuilder.CreateFAdd(llvmValue(a), llvmValue(b));
					return Value(mRuntime->floatValueType(), result, false);
				case ValueType::String: {
					Value as = toString(a);
					result = mRuntime->stringValueType()->stringAddition(&mIRBuilder, llvmValue(as), llvmValue(b));
					destruct(as);
					return Value(mRuntime->stringValueType(), result, false);
				}
				default: break;
			}
			break;
		case ValueType::Short:
			switch(b.valueType()->basicType()) {
				case ValueType::Integer:
				case ValueType::Boolean:
					result = mIRBuilder.CreateAdd(llvmValue(toInt(a)), llvmValue(toInt(b)));
					return Value(mRuntime->intValueType(), result);
				case ValueType::Float:
					result = mIRBuilder.CreateFAdd(llvmValue(toFloat(a)), llvmValue(b));
					return Value(mRuntime->floatValueType(), result);
				case ValueType::Short:
				case ValueType::Byte:
					result = mIRBuilder.CreateAdd(llvmValue(a), llvmValue(toInt(b)));
					return Value(mRuntime->intValueType(), result);
				case ValueType::String: {
					Value as = toString(a);
					result = mRuntime->stringValueType()->stringAddition(&mIRBuilder, llvmValue(as), llvmValue(b));
					destruct(as);
					return Value(mRuntime->stringValueType(), result);
				}
				default: break;
			}
			break;
		case ValueType::Byte:
			switch (b.valueType()->basicType()) {
				case ValueType::Integer:
				case ValueType::Boolean:
					result = mIRBuilder.CreateAdd(llvmValue(toInt(a)), llvmValue(toInt(b)));
					return Value(mRuntime->intValueType(), result);
				case ValueType::Float:
					result = mIRBuilder.CreateFAdd(llvmValue(toFloat(a)), llvmValue(b));
					return Value(mRuntime->floatValueType(), result);
				case ValueType::Short:
					result = mIRBuilder.CreateAdd(llvmValue(toShort(a)), llvmValue(b));
					return Value(mRuntime->intValueType(), result);
				case ValueType::Byte:
					result = mIRBuilder.CreateAdd(llvmValue(a), llvmValue(b));
					return Value(mRuntime->intValueType(), result);
				case ValueType::String: {
					Value as = toString(a);
					result = mRuntime->stringValueType()->stringAddition(&mIRBuilder, llvmValue(as), llvmValue(b));
					destruct(as);
					return Value(mRuntime->stringValueType(), result);
				}
				default: break;
			}
			break;
		case ValueType::String:
			switch (b.valueType()->basicType()) {
				case ValueType::Integer:
				case ValueType::Boolean:
				case ValueType::Float:
				case ValueType::Short:
				case ValueType::Byte: {
					Value as = toString(b);
					result = mRuntime->stringValueType()->stringAddition(&mIRBuilder, llvmValue(a), llvmValue(as));
					destruct(as);
					return Value(mRuntime->stringValueType(), result);
				}
				case ValueType::String: {
					result = mRuntime->stringValueType()->stringAddition(&mIRBuilder, llvmValue(a), llvmValue(b));
					return Value(mRuntime->stringValueType(), result);
				}
				default: break;
			}
			break;
		default: break;
	}
	assert("Invalid value" && 0);
	return Value();
}

Value Builder::subtract(const Value &a, const Value &b) {
	if (a.isConstant() && b.isConstant()) {
		OperationFlags flags;
		return Value(ConstantValue::subtract(a.constant(), b.constant(), flags), mRuntime);
	}
	llvm::Value *result;
	switch(a.valueType()->basicType()) {
		case ValueType::Boolean:
		case ValueType::Integer:
			switch(b.valueType()->basicType()) {
				case ValueType::Integer:
				case ValueType::Boolean:
					result = mIRBuilder.CreateSub(llvmValue(toInt(a)), llvmValue(toInt(b)));
					return Value(mRuntime->intValueType(), result);
				case ValueType::Float:
					result = mIRBuilder.CreateFSub(llvmValue(toFloat(a)), llvmValue(b));
					return Value(mRuntime->floatValueType(), result);
				case ValueType::Short:
				case ValueType::Byte:
					result = mIRBuilder.CreateSub(llvmValue(toInt(a)), llvmValue(toInt(b)));
					return Value(mRuntime->intValueType(), result);
				default: break;
			}
			break;
		case ValueType::Float:
			switch(b.valueType()->basicType()) {
				case ValueType::Integer:
				case ValueType::Short:
				case ValueType::Byte:
				case ValueType::Boolean:
					result = mIRBuilder.CreateFSub(llvmValue(a), llvmValue(toFloat(b)));
					return Value(mRuntime->floatValueType(), result);
				case ValueType::Float:
					result = mIRBuilder.CreateFSub(llvmValue(a), llvmValue(b));
					return Value(mRuntime->floatValueType(), result);
				default: break;
			}
			break;
		case ValueType::Short:
			switch(b.valueType()->basicType()) {
				case ValueType::Integer:
				case ValueType::Boolean:
					result = mIRBuilder.CreateSub(llvmValue(toInt(a)), llvmValue(toInt(b)));
					return Value(mRuntime->intValueType(), result);
				case ValueType::Float:
					result = mIRBuilder.CreateFSub(llvmValue(toFloat(a)), llvmValue(b));
					return Value(mRuntime->floatValueType(), result);
				case ValueType::Short:
				case ValueType::Byte:
					result = mIRBuilder.CreateSub(llvmValue(a), llvmValue(toInt(b)));
					return Value(mRuntime->intValueType(), result);
				default: break;
			}
			break;
		case ValueType::Byte:
			switch (b.valueType()->basicType()) {
				case ValueType::Integer:
				case ValueType::Boolean:
					result = mIRBuilder.CreateSub(llvmValue(toInt(a)), llvmValue(toInt(b)));
					return Value(mRuntime->intValueType(), result);
				case ValueType::Float:
					result = mIRBuilder.CreateFSub(llvmValue(toFloat(a)), llvmValue(b));
					return Value(mRuntime->floatValueType(), result);
				case ValueType::Short:
					result = mIRBuilder.CreateSub(llvmValue(toShort(a)), llvmValue(b));
					return Value(mRuntime->intValueType(), result);
				case ValueType::Byte:
					result = mIRBuilder.CreateSub(llvmValue(a), llvmValue(b));
					return Value(mRuntime->intValueType(), result);
				default: break;
			}
			break;
		default: break;
	}
	assert("Invalid value" && 0);
	return Value();
}

Value Builder::multiply(const Value &a, const Value &b) {
	if (a.isConstant() && b.isConstant()) {
		OperationFlags flags;
		return Value(ConstantValue::multiply(a.constant(), b.constant(), flags), mRuntime);
	}
	llvm::Value *result;
	switch(a.valueType()->basicType()) {
		case ValueType::Boolean:
		case ValueType::Integer:
			switch(b.valueType()->basicType()) {
				case ValueType::Integer:
				case ValueType::Boolean:
					result = mIRBuilder.CreateMul(llvmValue(toInt(a)), llvmValue(toInt(b)));
					return Value(mRuntime->intValueType(), result);
				case ValueType::Float:
					result = mIRBuilder.CreateFMul(llvmValue(toFloat(a)), llvmValue(b));
					return Value(mRuntime->floatValueType(), result);
				case ValueType::Short:
				case ValueType::Byte:
					result = mIRBuilder.CreateMul(llvmValue(toInt(a)), llvmValue(toInt(b)));
					return Value(mRuntime->intValueType(), result);
				default: break;
			}
			break;
		case ValueType::Float:
			switch(b.valueType()->basicType()) {
				case ValueType::Integer:
				case ValueType::Short:
				case ValueType::Byte:
				case ValueType::Boolean:
					result = mIRBuilder.CreateFMul(llvmValue(a), llvmValue(toFloat(b)));
					return Value(mRuntime->floatValueType(), result);
				case ValueType::Float:
					result = mIRBuilder.CreateFMul(llvmValue(a), llvmValue(b));
					return Value(mRuntime->floatValueType(), result);
				default: break;
			}
			break;
		case ValueType::Short:
			switch(b.valueType()->basicType()) {
				case ValueType::Integer:
				case ValueType::Boolean:
					result = mIRBuilder.CreateMul(llvmValue(toInt(a)), llvmValue(toInt(b)));
					return Value(mRuntime->intValueType(), result);
				case ValueType::Float:
					result = mIRBuilder.CreateFMul(llvmValue(toFloat(a)), llvmValue(b));
					return Value(mRuntime->floatValueType(), result);
				case ValueType::Short:
				case ValueType::Byte:
					result = mIRBuilder.CreateMul(llvmValue(a), llvmValue(toInt(b)));
					return Value(mRuntime->intValueType(), result);
				default: break;
			}
			break;
		case ValueType::Byte:
			switch (b.valueType()->basicType()) {
				case ValueType::Integer:
				case ValueType::Boolean:
					result = mIRBuilder.CreateMul(llvmValue(toInt(a)), llvmValue(toInt(b)));
					return Value(mRuntime->intValueType(), result);
				case ValueType::Float:
					result = mIRBuilder.CreateFMul(llvmValue(toFloat(a)), llvmValue(b));
					return Value(mRuntime->floatValueType(), result);
				case ValueType::Short:
					result = mIRBuilder.CreateMul(llvmValue(toShort(a)), llvmValue(b));
					return Value(mRuntime->intValueType(), result);
				case ValueType::Byte:
					result = mIRBuilder.CreateMul(llvmValue(a), llvmValue(b));
					return Value(mRuntime->intValueType(), result);
				default: break;
			}
			break;
		default: break;
	}
	assert("Invalid value" && 0);
	return Value();
}

Value Builder::divide(const Value &a, const Value &b) {
	if (a.isConstant() && b.isConstant()) {
		OperationFlags flags;
		return Value(ConstantValue::divide(a.constant(), b.constant(), flags), mRuntime);
	}
	llvm::Value *result;
	switch(a.valueType()->basicType()) {
		case ValueType::Boolean:
		case ValueType::Integer:
			switch(b.valueType()->basicType()) {
				case ValueType::Integer:
				case ValueType::Boolean:
					result = mIRBuilder.CreateSDiv(llvmValue(toInt(a)), llvmValue(toInt(b)));
					return Value(mRuntime->intValueType(), result);
				case ValueType::Float:
					result = mIRBuilder.CreateFDiv(llvmValue(toFloat(a)), llvmValue(b));
					return Value(mRuntime->floatValueType(), result);
				case ValueType::Short:
				case ValueType::Byte:
					result = mIRBuilder.CreateUDiv(llvmValue(toInt(a)), llvmValue(toInt(b)));
					return Value(mRuntime->intValueType(), result);
				default: break;
			}
			break;
		case ValueType::Float:
			switch(b.valueType()->basicType()) {
				case ValueType::Integer:
				case ValueType::Short:
				case ValueType::Byte:
				case ValueType::Boolean:
					result = mIRBuilder.CreateFDiv(llvmValue(a), llvmValue(toFloat(b)));
					return Value(mRuntime->floatValueType(), result);
				case ValueType::Float:
					result = mIRBuilder.CreateFDiv(llvmValue(a), llvmValue(b));
					return Value(mRuntime->floatValueType(), result);
				default: break;
			}
			break;
		case ValueType::Short:
			switch(b.valueType()->basicType()) {
				case ValueType::Integer:
				case ValueType::Boolean:
					result = mIRBuilder.CreateSDiv(llvmValue(toInt(a)), llvmValue(toInt(b)));
					return Value(mRuntime->intValueType(), result);
				case ValueType::Float:
					result = mIRBuilder.CreateFDiv(llvmValue(toFloat(a)), llvmValue(b));
					return Value(mRuntime->floatValueType(), result);
				case ValueType::Short:
				case ValueType::Byte:
					result = mIRBuilder.CreateUDiv(llvmValue(a), llvmValue(toInt(b)));
					return Value(mRuntime->intValueType(), result);
				default: break;
			}
			break;
		case ValueType::Byte:
			switch (b.valueType()->basicType()) {
				case ValueType::Integer:
				case ValueType::Boolean:
					result = mIRBuilder.CreateSDiv(llvmValue(toInt(a)), llvmValue(toInt(b)));
					return Value(mRuntime->intValueType(), result);
				case ValueType::Float:
					result = mIRBuilder.CreateFDiv(llvmValue(toFloat(a)), llvmValue(b));
					return Value(mRuntime->floatValueType(), result);
				case ValueType::Short:
				case ValueType::Byte:
					result = mIRBuilder.CreateUDiv(llvmValue(toShort(a)), llvmValue(b));
					return Value(mRuntime->intValueType(), result);
				default: break;
			}
			break;
		default: break;
	}
	assert("Invalid value" && 0);
	return Value();
}

Value Builder::mod(const Value &a, const Value &b){
	if (a.isConstant() && b.isConstant()) {
		OperationFlags flags;
		return Value(ConstantValue::mod(a.constant(), b.constant(), flags), mRuntime);
	}
	llvm::Value *result;
	switch(a.valueType()->basicType()) {
		case ValueType::Boolean:
		case ValueType::Integer:
			switch(b.valueType()->basicType()) {
				case ValueType::Integer:
				case ValueType::Boolean:
					result = mIRBuilder.CreateSRem(llvmValue(toInt(a)), llvmValue(toInt(b)));
					return Value(mRuntime->intValueType(), result);
				case ValueType::Float:
					result = mIRBuilder.CreateFRem(llvmValue(toFloat(a)), llvmValue(b));
					return Value(mRuntime->floatValueType(), result);
				case ValueType::Short:
				case ValueType::Byte:
					result = mIRBuilder.CreateURem(llvmValue(toInt(a)), llvmValue(toInt(b)));
					return Value(mRuntime->intValueType(), result);
				default: break;
			}
			break;
		case ValueType::Float:
			switch(b.valueType()->basicType()) {
				case ValueType::Integer:
				case ValueType::Short:
				case ValueType::Byte:
				case ValueType::Boolean:
					result = mIRBuilder.CreateFRem(llvmValue(a), llvmValue(toFloat(b)));
					return Value(mRuntime->floatValueType(), result);
				case ValueType::Float:
					result = mIRBuilder.CreateFRem(llvmValue(a), llvmValue(b));
					return Value(mRuntime->floatValueType(), result);
				default: break;
			}
			break;
		case ValueType::Short:
			switch(b.valueType()->basicType()) {
				case ValueType::Integer:
				case ValueType::Boolean:
					result = mIRBuilder.CreateSRem(llvmValue(toInt(a)), llvmValue(toInt(b)));
					return Value(mRuntime->intValueType(), result);
				case ValueType::Float:
					result = mIRBuilder.CreateFRem(llvmValue(toFloat(a)), llvmValue(b));
					return Value(mRuntime->floatValueType(), result);
				case ValueType::Short:
				case ValueType::Byte:
					result = mIRBuilder.CreateURem(llvmValue(a), llvmValue(toInt(b)));
					return Value(mRuntime->intValueType(), result);
				default: break;
			}
			break;
		case ValueType::Byte:
			switch (b.valueType()->basicType()) {
				case ValueType::Integer:
				case ValueType::Boolean:
					result = mIRBuilder.CreateSRem(llvmValue(toInt(a)), llvmValue(toInt(b)));
					return Value(mRuntime->intValueType(), result);
				case ValueType::Float:
					result = mIRBuilder.CreateFRem(llvmValue(toFloat(a)), llvmValue(b));
					return Value(mRuntime->floatValueType(), result);
				case ValueType::Short:
				case ValueType::Byte:
					result = mIRBuilder.CreateURem(llvmValue(toShort(a)), llvmValue(b));
					return Value(mRuntime->intValueType(), result);
				default: break;
			}
			break;
		default: break;
	}
	assert("Invalid value" && 0);
	return Value();
}

Value Builder::shl(const Value &a, const Value &b){
	if (a.isConstant() && b.isConstant()) {
		OperationFlags flags;
		return Value(ConstantValue::shl(a.constant(),b.constant(), flags), mRuntime);
	}
	//No strings or floats
	assert(a.valueType()->basicType() != ValueType::Float && a.valueType()->basicType() != ValueType::String);
	assert(b.valueType()->basicType() != ValueType::Float && b.valueType()->basicType() != ValueType::String);

	if (a.valueType()->basicType() == ValueType::Integer || b.valueType()->basicType() == ValueType::Integer || a.valueType()->basicType() == ValueType::Boolean || b.valueType()->basicType() == ValueType::Boolean) {
		return Value(mRuntime->intValueType(), mIRBuilder.CreateShl(llvmValue(toInt(a)), llvmValue(toInt(b))));
	}
	if (a.valueType()->basicType() == ValueType::Short || b.valueType()->basicType() == ValueType::Short) {
		return Value(mRuntime->shortValueType(), mIRBuilder.CreateShl(llvmValue(toShort(a)), llvmValue(toShort(b))));
	}

	return Value(mRuntime->byteValueType(), mIRBuilder.CreateShl(llvmValue(toByte(a)), llvmValue(toByte(b))));
}

Value Builder::shr(const Value &a, const Value &b){
	if (a.isConstant() && b.isConstant()) {
		OperationFlags flags;
		return Value(ConstantValue::shl(a.constant(), b.constant(), flags), mRuntime);
	}
	//No strings or floats
	assert(a.valueType()->basicType() != ValueType::Float && a.valueType()->basicType() != ValueType::String);
	assert(b.valueType()->basicType() != ValueType::Float && b.valueType()->basicType() != ValueType::String);

	if (a.valueType()->basicType() == ValueType::Integer || b.valueType()->basicType() == ValueType::Integer || a.valueType()->basicType() == ValueType::Boolean || b.valueType()->basicType() == ValueType::Boolean) {
		return Value(mRuntime->intValueType(), mIRBuilder.CreateLShr(llvmValue(toInt(a)), llvmValue(toInt(b))));
	}
	if (a.valueType()->basicType() == ValueType::Short || b.valueType()->basicType() == ValueType::Short) {
		return Value(mRuntime->shortValueType(), mIRBuilder.CreateLShr(llvmValue(toShort(a)), llvmValue(toShort(b))));
	}

	return Value(mRuntime->byteValueType(), mIRBuilder.CreateLShr(llvmValue(toByte(a)), llvmValue(toByte(b))));
}

Value Builder::sar(const Value &a, const Value &b){
	if (a.isConstant() && b.isConstant()) {
		OperationFlags flags;
		return Value(ConstantValue::shl(a.constant(),b.constant(), flags), mRuntime);
	}
	//No strings or floats
	assert(a.valueType()->basicType() != ValueType::Float && a.valueType()->basicType() != ValueType::String);
	assert(b.valueType()->basicType() != ValueType::Float && b.valueType()->basicType() != ValueType::String);

	if (a.valueType()->basicType() == ValueType::Integer || b.valueType()->basicType() == ValueType::Integer || a.valueType()->basicType() == ValueType::Boolean || b.valueType()->basicType() == ValueType::Boolean) {
		return Value(mRuntime->intValueType(), mIRBuilder.CreateAShr(llvmValue(toInt(a)), llvmValue(toInt(b))));
	}
	if (a.valueType()->basicType() == ValueType::Short || b.valueType()->basicType() == ValueType::Short) {
		return Value(mRuntime->shortValueType(), mIRBuilder.CreateAShr(llvmValue(toShort(a)), llvmValue(toShort(b))));
	}

	return Value(mRuntime->byteValueType(), mIRBuilder.CreateAShr(llvmValue(toByte(a)), llvmValue(toByte(b))));
}

Value Builder::and_(const Value &a, const Value &b) {
	if (a.isConstant() && b.isConstant()) {
		OperationFlags flags;
		return Value(ConstantValue::and_(a.constant(), b.constant(), flags), mRuntime);
	}
	return Value(mRuntime->booleanValueType(), mIRBuilder.CreateAnd(llvmValue(toBoolean(a)), llvmValue(toBoolean(b))));
}

Value Builder::or_(const Value &a, const Value &b) {
	if (a.isConstant() && b.isConstant()) {
		OperationFlags flags;
		return Value(ConstantValue::or_(a.constant(), b.constant(), flags), mRuntime);
	}
	return Value(mRuntime->booleanValueType(), mIRBuilder.CreateOr(llvmValue(toBoolean(a)), llvmValue(toBoolean(b))));
}

Value Builder::xor_(const Value &a, const Value &b) {
	if (a.isConstant() && b.isConstant()) {
		OperationFlags flags;
		return Value(ConstantValue::xor_(a.constant(),b.constant(), flags), mRuntime);
	}
	return Value(mRuntime->booleanValueType(), mIRBuilder.CreateXor(llvmValue(toBoolean(a)), llvmValue(toBoolean(b))));
}

Value Builder::power(const Value &a, const Value &b) {
	if (a.isConstant() && b.isConstant()) {
		OperationFlags flags;
		return Value(ConstantValue::power(a.constant(), b.constant(), flags), mRuntime);
	}
	if (a.valueType()->basicType() == ValueType::Float || b.valueType()->basicType() == ValueType::Float) {
		llvm::Value *af = llvmValue(toFloat(a));
		llvm::Value *bf = llvmValue(toFloat(b));
		llvm::Value *ad = mIRBuilder.CreateFPExt(af, mIRBuilder.getDoubleTy());
		llvm::Value *bd = mIRBuilder.CreateFPExt(bf, mIRBuilder.getDoubleTy());
		llvm::Value *retD = mIRBuilder.CreateCall2(mPowFF, ad, bd);
		return Value(mRuntime->floatValueType(), mIRBuilder.CreateFPTrunc(retD, mIRBuilder.getFloatTy()));
	}
	llvm::Value *af = llvmValue(toFloat(a));
	llvm::Value *ad = mIRBuilder.CreateFPExt(af, mIRBuilder.getDoubleTy());
	llvm::Value *retD = mIRBuilder.CreateCall2(mPowFI, ad, llvmValue(toInt(b)));
	llvm::Value *greaterCond = mIRBuilder.CreateFCmpOGT(retD, llvm::ConstantFP::get( mIRBuilder.getDoubleTy(), (double)INT_MAX));
	llvm::Value *lessCond = mIRBuilder.CreateFCmpOLT(retD, llvm::ConstantFP::get( mIRBuilder.getDoubleTy(), (double)-INT_MAX));
	llvm::Value *cond = mIRBuilder.CreateOr(greaterCond, lessCond);
	llvm::Value *ret = mIRBuilder.CreateSelect(cond, llvmValue(-INT_MAX), mIRBuilder.CreateFPToSI(retD, mIRBuilder.getInt32Ty()));
	return Value(mRuntime->intValueType(), ret);
}

Value Builder::less(const Value &a, const Value &b) {
	if (a.isConstant() && b.isConstant()) {
		OperationFlags flags;
		return Value(ConstantValue::lessEqual(a.constant(), b.constant(), flags), mRuntime);
	}
	if (a.valueType()->basicType() == ValueType::String) {
		//TODO: finish this
		assert(0);
		return Value();
	}
	if (b.valueType()->basicType() == ValueType::String) {
		//TODO: finish this
		assert(0);
		return Value();
	}
	switch (a.valueType()->basicType()) {
		case ValueType::Boolean:
		case ValueType::Integer:
			switch (b.valueType()->basicType()) {
				case ValueType::Boolean:
				case ValueType::Integer:
				case ValueType::Short:
				case ValueType::Byte:
					return Value(mRuntime->booleanValueType(), mIRBuilder.CreateICmpSLT(llvmValue(toInt(a)), llvmValue(toInt(b))));
				case ValueType::Float:
					return Value(mRuntime->booleanValueType(), mIRBuilder.CreateFCmpOLT(llvmValue(toFloat(a)), llvmValue(toFloat(b))));
				default: break;
			}
			break;
		case ValueType::Short:
			switch (b.valueType()->basicType()) {
				case ValueType::Boolean:
				case ValueType::Integer:
					return Value(mRuntime->booleanValueType(), mIRBuilder.CreateICmpSLT(llvmValue(toInt(a)), llvmValue(toInt(b))));
				case ValueType::Short:
				case ValueType::Byte:
					return Value(mRuntime->booleanValueType(), mIRBuilder.CreateICmpULT(llvmValue(toShort(a)), llvmValue(toShort(b))));
				case ValueType::Float:
					return Value(mRuntime->booleanValueType(), mIRBuilder.CreateFCmpOLT(llvmValue(toFloat(a)), llvmValue(toFloat(b))));
				default: break;
			}
			break;
		case ValueType::Byte:
			switch (b.valueType()->basicType()) {
				case ValueType::Boolean:
				case ValueType::Integer:
					return Value(mRuntime->booleanValueType(), mIRBuilder.CreateICmpSLT(llvmValue(toInt(a)), llvmValue(toInt(b))));
				case ValueType::Short:
					return Value(mRuntime->booleanValueType(), mIRBuilder.CreateICmpULT(llvmValue(toShort(a)), llvmValue(toShort(b))));
				case ValueType::Byte:
					return Value(mRuntime->booleanValueType(), mIRBuilder.CreateICmpULT(llvmValue(a), llvmValue(b)));
				case ValueType::Float:
					return Value(mRuntime->booleanValueType(), mIRBuilder.CreateFCmpOLT(llvmValue(toFloat(a)), llvmValue(toFloat(b))));
				default: break;
			}
			break;
		case ValueType::Float:
			return Value(mRuntime->booleanValueType(), mIRBuilder.CreateFCmpOLT(llvmValue(toFloat(a)), llvmValue(toFloat(b))));
		default: break;
	}

	assert("Invalid value" && 0);
	return Value();
}

Value Builder::lessEqual(const Value &a, const Value &b) {
	if (a.isConstant() && b.isConstant()) {
		OperationFlags flags;
		return Value(ConstantValue::lessEqual(a.constant(), b.constant(), flags), mRuntime);
	}
	if (a.valueType()->basicType() == ValueType::String) {
		//TODO: finish this
		assert(0);
		return Value();
	}
	if (b.valueType()->basicType() == ValueType::String) {
		//TODO: finish this
		assert(0);
		return Value();
	}
	switch (a.valueType()->basicType()) {
		case ValueType::Boolean:
		case ValueType::Integer:
			switch (b.valueType()->basicType()) {
				case ValueType::Boolean:
				case ValueType::Integer:
				case ValueType::Short:
				case ValueType::Byte:
					return Value(mRuntime->booleanValueType(), mIRBuilder.CreateICmpSLE(llvmValue(toInt(a)), llvmValue(toInt(b))));
				case ValueType::Float:
					return Value(mRuntime->booleanValueType(), mIRBuilder.CreateFCmpOLE(llvmValue(toFloat(a)), llvmValue(toFloat(b))));
				default: break;
			}
			break;
		case ValueType::Short:
			switch (b.valueType()->basicType()) {
				case ValueType::Boolean:
				case ValueType::Integer:
					return Value(mRuntime->booleanValueType(), mIRBuilder.CreateICmpSLE(llvmValue(toInt(a)), llvmValue(toInt(b))));
				case ValueType::Short:
				case ValueType::Byte:
					return Value(mRuntime->booleanValueType(), mIRBuilder.CreateICmpULE(llvmValue(toShort(a)), llvmValue(toShort(b))));
				case ValueType::Float:
					return Value(mRuntime->booleanValueType(), mIRBuilder.CreateFCmpOLE(llvmValue(toFloat(a)), llvmValue(toFloat(b))));
				default: break;
			}
			break;
		case ValueType::Byte:
			switch (b.valueType()->basicType()) {
				case ValueType::Boolean:
				case ValueType::Integer:
					return Value(mRuntime->booleanValueType(), mIRBuilder.CreateICmpSLE(llvmValue(toInt(a)), llvmValue(toInt(b))));
				case ValueType::Short:
					return Value(mRuntime->booleanValueType(), mIRBuilder.CreateICmpULE(llvmValue(toShort(a)), llvmValue(toShort(b))));
				case ValueType::Byte:
					return Value(mRuntime->booleanValueType(), mIRBuilder.CreateICmpULE(llvmValue(a), llvmValue(b)));
				case ValueType::Float:
					return Value(mRuntime->booleanValueType(), mIRBuilder.CreateFCmpOLE(llvmValue(toFloat(a)), llvmValue(toFloat(b))));
				default: break;
			}
			break;
		case ValueType::Float:
			return Value(mRuntime->booleanValueType(), mIRBuilder.CreateFCmpOLE(llvmValue(toFloat(a)), llvmValue(toFloat(b))));
		default: break;
	}

	assert("Invalid value" && 0); return Value();
}

Value Builder::greater(const Value &a, const Value &b) {
	if (a.isConstant() && b.isConstant()) {
		OperationFlags flags;
		return Value(ConstantValue::lessEqual(a.constant(), b.constant(), flags), mRuntime);
	}
	if (a.valueType()->basicType() == ValueType::String) {
		//TODO: finish this
		assert(0);
		return Value();
	}
	if (b.valueType()->basicType() == ValueType::String) {
		//TODO: finish this
		assert(0);
		return Value();
	}
	switch (a.valueType()->basicType()) {
		case ValueType::Boolean:
		case ValueType::Integer:
			switch (b.valueType()->basicType()) {
				case ValueType::Boolean:
				case ValueType::Integer:
				case ValueType::Short:
				case ValueType::Byte:
					return Value(mRuntime->booleanValueType(), mIRBuilder.CreateICmpSGT(llvmValue(toInt(a)), llvmValue(toInt(b))));
				case ValueType::Float:
					return Value(mRuntime->booleanValueType(), mIRBuilder.CreateFCmpOGT(llvmValue(toFloat(a)), llvmValue(toFloat(b))));
				default: break;
			}
			break;
		case ValueType::Short:
			switch (b.valueType()->basicType()) {
				case ValueType::Boolean:
				case ValueType::Integer:
					return Value(mRuntime->booleanValueType(), mIRBuilder.CreateICmpSGT(llvmValue(toInt(a)), llvmValue(toInt(b))));
				case ValueType::Short:
				case ValueType::Byte:
					return Value(mRuntime->booleanValueType(), mIRBuilder.CreateICmpUGT(llvmValue(toShort(a)), llvmValue(toShort(b))));
				case ValueType::Float:
					return Value(mRuntime->booleanValueType(), mIRBuilder.CreateFCmpOGT(llvmValue(toFloat(a)), llvmValue(toFloat(b))));
				default: break;
			}
			break;
		case ValueType::Byte:
			switch (b.valueType()->basicType()) {
				case ValueType::Boolean:
				case ValueType::Integer:
					return Value(mRuntime->booleanValueType(), mIRBuilder.CreateICmpSGT(llvmValue(toInt(a)), llvmValue(toInt(b))));
				case ValueType::Short:
					return Value(mRuntime->booleanValueType(), mIRBuilder.CreateICmpUGT(llvmValue(toShort(a)), llvmValue(toShort(b))));
				case ValueType::Byte:
					return Value(mRuntime->booleanValueType(), mIRBuilder.CreateICmpUGT(llvmValue(a), llvmValue(b)));
				case ValueType::Float:
					return Value(mRuntime->booleanValueType(), mIRBuilder.CreateFCmpOGT(llvmValue(toFloat(a)), llvmValue(toFloat(b))));
				default: break;
			}
			break;
		case ValueType::Float:
			return Value(mRuntime->booleanValueType(), mIRBuilder.CreateFCmpOGT(llvmValue(toFloat(a)), llvmValue(toFloat(b))));
		default: break;
	}

	assert("Invalid value" && 0);
	return Value();
}

Value Builder::greaterEqual(const Value &a, const Value &b) {
	if (a.isConstant() && b.isConstant()) {
		OperationFlags flags;
		return Value(ConstantValue::lessEqual(a.constant(), b.constant(), flags), mRuntime);
	}
	if (a.valueType()->basicType() == ValueType::String) {
		//TODO: finish this
		assert(0);
		return Value();
	}
	if (b.valueType()->basicType() == ValueType::String) {
		//TODO: finish this
		assert(0);
		return Value();
	}
	switch (a.valueType()->basicType()) {
		case ValueType::Boolean:
		case ValueType::Integer:
			switch (b.valueType()->basicType()) {
				case ValueType::Boolean:
				case ValueType::Integer:
				case ValueType::Short:
				case ValueType::Byte:
					return Value(mRuntime->booleanValueType(), mIRBuilder.CreateICmpSGE(llvmValue(toInt(a)), llvmValue(toInt(b))));
				case ValueType::Float:
					return Value(mRuntime->booleanValueType(), mIRBuilder.CreateFCmpOGE(llvmValue(toFloat(a)), llvmValue(toFloat(b))));
				default: break;
			}
			break;
		case ValueType::Short:
			switch (b.valueType()->basicType()) {
				case ValueType::Boolean:
				case ValueType::Integer:
					return Value(mRuntime->booleanValueType(), mIRBuilder.CreateICmpSGE(llvmValue(toInt(a)), llvmValue(toInt(b))));
				case ValueType::Short:
				case ValueType::Byte:
					return Value(mRuntime->booleanValueType(), mIRBuilder.CreateICmpUGE(llvmValue(toShort(a)), llvmValue(toShort(b))));
				case ValueType::Float:
					return Value(mRuntime->booleanValueType(), mIRBuilder.CreateFCmpOGE(llvmValue(toFloat(a)), llvmValue(toFloat(b))));
				default: break;
			}
			break;
		case ValueType::Byte:
			switch (b.valueType()->basicType()) {
				case ValueType::Boolean:
				case ValueType::Integer:
					return Value(mRuntime->booleanValueType(), mIRBuilder.CreateICmpSGE(llvmValue(toInt(a)), llvmValue(toInt(b))));
				case ValueType::Short:
					return Value(mRuntime->booleanValueType(), mIRBuilder.CreateICmpUGE(llvmValue(toShort(a)), llvmValue(toShort(b))));
				case ValueType::Byte:
					return Value(mRuntime->booleanValueType(), mIRBuilder.CreateICmpUGE(llvmValue(a), llvmValue(b)));
				case ValueType::Float:
					return Value(mRuntime->booleanValueType(), mIRBuilder.CreateFCmpOGE(llvmValue(toFloat(a)), llvmValue(toFloat(b))));
				default: break;
			}
			break;
		case ValueType::Float:
			return Value(mRuntime->booleanValueType(), mIRBuilder.CreateFCmpOGE(llvmValue(toFloat(a)), llvmValue(toFloat(b))));
		default: break;
	}

	assert(0); return Value();
}

Value Builder::equal(const Value &a, const Value &b) {
	if (a.isConstant() && b.isConstant()) {
		OperationFlags flags;
		return Value(ConstantValue::equal(a.constant(), b.constant(), flags), mRuntime);
	}
	switch (a.valueType()->basicType()) {
		case ValueType::Integer:
			switch (b.valueType()->basicType()) {
				case ValueType::Short:
				case ValueType::Byte:
				case ValueType::Integer:
				case ValueType::Boolean:
					return Value(mRuntime->booleanValueType(), mIRBuilder.CreateICmpEQ(llvmValue(a), llvmValue(toInt(b))));
				case ValueType::Float:
					return Value(mRuntime->booleanValueType(), mIRBuilder.CreateFCmpOEQ(llvmValue(toFloat(a)), llvmValue(b)));
				case ValueType::String: {
					Value as = toString(a);
					llvm::Value *ret = mRuntime->stringValueType()->stringEquality(&mIRBuilder, llvmValue(as), llvmValue(b));
					destruct(b);
					destruct(as);
					return Value(mRuntime->booleanValueType(), ret);
				}
				default: break;
			}
			break;
		case ValueType::Float:
			switch (b.valueType()->basicType()) {
				case ValueType::Integer:
				case ValueType::Float:
				case ValueType::Short:
				case ValueType::Byte:
				case ValueType::Boolean:
					return Value(mRuntime->booleanValueType(), mIRBuilder.CreateFCmpOEQ(llvmValue(a), llvmValue(toFloat(b))));
				case ValueType::String: {
					Value as = toString(a);
					llvm::Value *ret = mRuntime->stringValueType()->stringEquality(&mIRBuilder, llvmValue(as), llvmValue(b));
					destruct(b);
					destruct(as);
					return Value(mRuntime->booleanValueType(), ret);
				}
				default: break;
			}
			break;
		case ValueType::Boolean:
			switch (b.valueType()->basicType()) {
				case ValueType::Boolean:
					return Value(mRuntime->booleanValueType(), mIRBuilder.CreateICmpEQ(llvmValue(a), llvmValue(b)));
				case ValueType::Byte:
					return Value(mRuntime->booleanValueType(), mIRBuilder.CreateICmpEQ(llvmValue(toByte(a)), llvmValue(b)));
				case ValueType::Short:
					return Value(mRuntime->booleanValueType(), mIRBuilder.CreateICmpEQ(llvmValue(toShort(a)), llvmValue(b)));
				case ValueType::Integer:
					return Value(mRuntime->booleanValueType(), mIRBuilder.CreateICmpEQ(llvmValue(toInt(a)), llvmValue(b)));
				case ValueType::Float:
					return Value(mRuntime->booleanValueType(), mIRBuilder.CreateICmpEQ(llvmValue(toFloat(a)), llvmValue(b)));
				case ValueType::String: {
					Value as = toString(a);
					llvm::Value *ret = mRuntime->stringValueType()->stringEquality(&mIRBuilder, llvmValue(as), llvmValue(b));
					destruct(as);
					destruct(b);
					return Value(mRuntime->booleanValueType(), ret);
				}
				default: break;
			}
			break;
		case ValueType::Short:
			switch (b.valueType()->basicType()) {
				case ValueType::Boolean:
				case ValueType::Short:
				case ValueType::Byte:
					return Value(mRuntime->booleanValueType(), mIRBuilder.CreateICmpEQ(llvmValue(a), llvmValue(toShort(b))));
				case ValueType::Integer:
					return Value(mRuntime->booleanValueType(), mIRBuilder.CreateICmpEQ(llvmValue(toInt(a)), llvmValue(b)));
				case ValueType::Float:
					return Value(mRuntime->booleanValueType(), mIRBuilder.CreateFCmpOEQ(llvmValue(toFloat(a)), llvmValue(b)));
				case ValueType::String: {
					Value as = toString(a);
					llvm::Value *ret = mRuntime->stringValueType()->stringEquality(&mIRBuilder, llvmValue(as), llvmValue(b));
					destruct(as);
					destruct(b);
					return Value(mRuntime->booleanValueType(), ret);
				}
				default: break;
			}
			break;
		case ValueType::Byte:
			switch (b.valueType()->basicType()) {
				case ValueType::Boolean:
				case ValueType::Byte:
					return Value(mRuntime->booleanValueType(), mIRBuilder.CreateICmpEQ(llvmValue(a), llvmValue(toByte(b))));
				case ValueType::Short:
					return Value(mRuntime->booleanValueType(), mIRBuilder.CreateICmpEQ(llvmValue(toShort(a)), llvmValue(b)));
				case ValueType::Integer:
					return Value(mRuntime->booleanValueType(), mIRBuilder.CreateICmpEQ(llvmValue(toInt(a)), llvmValue(b)));
				case ValueType::Float:
					return Value(mRuntime->booleanValueType(), mIRBuilder.CreateFCmpOEQ(llvmValue(toFloat(a)), llvmValue(b)));
				case ValueType::String: {
					Value as = toString(a);
					llvm::Value *ret = mRuntime->stringValueType()->stringEquality(&mIRBuilder, llvmValue(as), llvmValue(b));
					destruct(as);
					destruct(b);
					return Value(mRuntime->booleanValueType(), ret);
				}
				default: break;
			}
			break;
		case ValueType::String: {
			if (b.valueType()->basicType() == ValueType::String) {
				llvm::Value *ret = mRuntime->stringValueType()->stringEquality(&mIRBuilder, llvmValue(a), llvmValue(b));
				return Value(mRuntime->booleanValueType(), ret);
			}
			Value bs = toString(b);
			llvm::Value *ret = mRuntime->stringValueType()->stringEquality(&mIRBuilder, llvmValue(a), llvmValue(bs));
			destruct(bs);
			destruct(a);
			return Value(mRuntime->booleanValueType(), ret);
		}
		default:
			if (a.valueType()->isTypePointer() && b.valueType()->isTypePointer()) {
				llvm::Value *ret;
				if (a.valueType() == b.valueType()) {
					ret = mIRBuilder.CreateICmpEQ(llvmValue(a), llvmValue(b));
				}
				else {
					ret = mIRBuilder.CreateICmpEQ(llvmValue(a), bitcast(a.valueType()->llvmType(), llvmValue(b)));
				}
				return Value(mRuntime->booleanValueType(), ret, false);
			}
	}

	assert("Invalid equality operation" && 0); return Value();
}

Value Builder::ptrEqual(llvm::Value *a, llvm::Value *b) {
	return Value(mRuntime->booleanValueType(), mIRBuilder.CreateICmpEQ(a, b), false);
}


Value Builder::notEqual(const Value &a, const Value &b) {
	if (a.isConstant() && b.isConstant()) {
		OperationFlags flags;
		return Value(ConstantValue::equal(a.constant(), b.constant(), flags), mRuntime);
	}
	switch (a.valueType()->basicType()) {
		case ValueType::Integer:
			switch (b.valueType()->basicType()) {
				case ValueType::Short:
				case ValueType::Byte:
				case ValueType::Integer:
				case ValueType::Boolean:
					return Value(mRuntime->booleanValueType(), mIRBuilder.CreateICmpNE(llvmValue(a), llvmValue(toInt(b))));
				case ValueType::Float:
					return Value(mRuntime->booleanValueType(), mIRBuilder.CreateFCmpONE(llvmValue(toFloat(a)), llvmValue(b)));
				case ValueType::String: {
					//TODO: StringValueType::stringUnequality?
					Value as = toString(a);
					llvm::Value *ret = mRuntime->stringValueType()->stringEquality(&mIRBuilder, llvmValue(as), llvmValue(b));
					destruct(b);
					destruct(as);
					return Value(mRuntime->booleanValueType(), mIRBuilder.CreateNot(ret));
				}
				default: break;
			}
			break;
		case ValueType::Float:
			switch (b.valueType()->basicType()) {
				case ValueType::Integer:
				case ValueType::Float:
				case ValueType::Short:
				case ValueType::Byte:
				case ValueType::Boolean:
					return Value(mRuntime->booleanValueType(), mIRBuilder.CreateFCmpONE(llvmValue(a), llvmValue(toInt(b))));
				case ValueType::String: {
					Value as = toString(a);
					llvm::Value *ret = mRuntime->stringValueType()->stringEquality(&mIRBuilder, llvmValue(as), llvmValue(b));
					destruct(b);
					destruct(as);
					return Value(mRuntime->booleanValueType(), mIRBuilder.CreateNot(ret));
				}
				default: break;
			}
			break;
		case ValueType::Boolean:
			switch (b.valueType()->basicType()) {
				case ValueType::Boolean:
					return Value(mRuntime->booleanValueType(), mIRBuilder.CreateICmpNE(llvmValue(a), llvmValue(b)));
				case ValueType::Byte:
					return Value(mRuntime->booleanValueType(), mIRBuilder.CreateICmpNE(llvmValue(toByte(a)), llvmValue(b)));
				case ValueType::Short:
					return Value(mRuntime->booleanValueType(), mIRBuilder.CreateICmpNE(llvmValue(toShort(a)), llvmValue(b)));
				case ValueType::Integer:
					return Value(mRuntime->booleanValueType(), mIRBuilder.CreateICmpNE(llvmValue(toInt(a)), llvmValue(b)));
				case ValueType::Float:
					return Value(mRuntime->booleanValueType(), mIRBuilder.CreateICmpNE(llvmValue(toFloat(a)), llvmValue(b)));
				case ValueType::String: {
					Value as = toString(a);
					llvm::Value *ret = mRuntime->stringValueType()->stringEquality(&mIRBuilder, llvmValue(as), llvmValue(b));
					destruct(as);
					destruct(b);
					return Value(mRuntime->booleanValueType(), mIRBuilder.CreateNot(ret));
				}
				default: break;
			}
			break;
		case ValueType::Short:
			switch (b.valueType()->basicType()) {
				case ValueType::Boolean:
				case ValueType::Short:
				case ValueType::Byte:
					return Value(mRuntime->booleanValueType(), mIRBuilder.CreateICmpNE(llvmValue(a), llvmValue(toShort(b))));
				case ValueType::Integer:
					return Value(mRuntime->booleanValueType(), mIRBuilder.CreateICmpNE(llvmValue(toInt(a)), llvmValue(b)));
				case ValueType::Float:
					return Value(mRuntime->booleanValueType(), mIRBuilder.CreateFCmpONE(llvmValue(toFloat(a)), llvmValue(b)));
				case ValueType::String: {
					Value as = toString(a);
					llvm::Value *ret = mRuntime->stringValueType()->stringEquality(&mIRBuilder, llvmValue(as), llvmValue(b));
					destruct(as);
					destruct(b);
					return Value(mRuntime->booleanValueType(), mIRBuilder.CreateNot(ret));
				}
				default: break;
			}
			break;
		case ValueType::Byte:
			switch (b.valueType()->basicType()) {
				case ValueType::Boolean:
				case ValueType::Byte:
					return Value(mRuntime->booleanValueType(), mIRBuilder.CreateICmpNE(llvmValue(a), llvmValue(toByte(b))));
				case ValueType::Short:
					return Value(mRuntime->booleanValueType(), mIRBuilder.CreateICmpNE(llvmValue(toShort(a)), llvmValue(b)));
				case ValueType::Integer:
					return Value(mRuntime->booleanValueType(), mIRBuilder.CreateICmpNE(llvmValue(toInt(a)), llvmValue(b)));
				case ValueType::Float:
					return Value(mRuntime->booleanValueType(), mIRBuilder.CreateFCmpONE(llvmValue(toFloat(a)), llvmValue(b)));
				case ValueType::String: {
					Value as = toString(a);
					llvm::Value *ret = mRuntime->stringValueType()->stringEquality(&mIRBuilder, llvmValue(as), llvmValue(b));
					destruct(as);
					destruct(b);
					return Value(mRuntime->booleanValueType(), mIRBuilder.CreateNot(ret));
				}
				default: break;
			}
			break;
		case ValueType::String: {
			if (b.valueType()->basicType() == ValueType::String) {
				llvm::Value *ret = mRuntime->stringValueType()->stringEquality(&mIRBuilder, llvmValue(a), llvmValue(b));
				return Value(mRuntime->booleanValueType(), mIRBuilder.CreateNot(ret));
			}
			Value bs = toString(b);
			llvm::Value *ret = mRuntime->stringValueType()->stringEquality(&mIRBuilder, llvmValue(a), llvmValue(bs));
			destruct(bs);
			destruct(a);
			return Value(mRuntime->booleanValueType(), mIRBuilder.CreateNot(ret));
		}
		default:
			if (a.valueType()->isTypePointer() && b.valueType()->isTypePointer()) {
				llvm::Value *ret;
				if (a.valueType() == b.valueType()) {
					ret = mIRBuilder.CreateICmpNE(llvmValue(a), llvmValue(b));
				}
				else {
					ret = mIRBuilder.CreateICmpNE(llvmValue(a), bitcast(a.valueType()->llvmType(), llvmValue(b)));
				}
				return Value(mRuntime->booleanValueType(), ret, false);
			}
	}

	assert("Invalid operation" && 0); return Value();
}

void Builder::memCopy(llvm::Value *src, llvm::Value *dest, llvm::Value *num, int align) {
	assert(src->getType()->isPointerTy());
	assert(dest->getType()->isPointerTy());
	assert(num->getType() == llvm::IntegerType::get(context(), 32));

	//Cast src and dest to i8* if they are not already
	if (src->getType() != llvm::IntegerType::get(context(), 8)->getPointerTo()) {
		src = mIRBuilder.CreateBitCast(src, llvm::IntegerType::get(context(), 8)->getPointerTo());
	}
	if (dest->getType() != llvm::IntegerType::get(context(), 8)->getPointerTo()) {
		dest = mIRBuilder.CreateBitCast(dest, llvm::IntegerType::get(context(), 8)->getPointerTo());
	}

	mIRBuilder.CreateMemCpy(dest, src, num, align);
}

void Builder::memSet(llvm::Value *ptr, llvm::Value *num, llvm::Value *value, int align) {
	assert(ptr->getType()->isPointerTy());
	assert(num->getType() == llvm::IntegerType::get(context(), 32));
	assert(value->getType() == llvm::IntegerType::get(context(), 8));

	//Cast src and dest to i8* if they are not already
	if (ptr->getType() != llvm::IntegerType::get(context(), 8)->getPointerTo()) {
		ptr = mIRBuilder.CreateBitCast(ptr, llvm::IntegerType::get(context(), 8)->getPointerTo());
	}

	mIRBuilder.CreateMemSet(ptr, value, num, align);
}

llvm::Value *Builder::allocate(llvm::Value *size) {
	assert(size->getType() == mIRBuilder.getInt32Ty());
	return mIRBuilder.CreateCall(mRuntime->allocatorFunction(), size);
}

void Builder::free(llvm::Value *ptr) {
	assert(ptr->getType()->isPointerTy());
	ptr = pointerToBytePointer(ptr);
	mIRBuilder.CreateCall(mRuntime->freeFunction(), ptr);
}

llvm::Value *Builder::pointerToBytePointer(llvm::Value *ptr) {
	assert(ptr->getType()->isPointerTy());
	if (ptr->getType() != llvm::IntegerType::get(context(), 8)->getPointerTo()) {
		ptr = mIRBuilder.CreateBitCast(ptr, llvm::IntegerType::get(context(), 8)->getPointerTo());
	}
	return ptr;
}

llvm::BasicBlock *Builder::currentBasicBlock() const {
	return mIRBuilder.GetInsertBlock();
}




//TODO: These dont seems to work? Why?
void Builder::pushInsertPoint() {
	llvm::IRBuilder<>::InsertPoint insertPoint = mIRBuilder.saveIP();
	mInsertPointStack.push(insertPoint);
}

void Builder::popInsertPoint() {
	llvm::IRBuilder<>::InsertPoint insertPoint = mInsertPointStack.pop();
	mIRBuilder.restoreIP(insertPoint);
}


