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
#include "genericarrayvaluetype.h"
#include "arrayvaluetype.h"
#include "structvaluetype.h"
#include <QDebug>

Builder::Builder(llvm::LLVMContext &context) :
	llvm::IRBuilder<>(context),
	mRuntime(0),
	mStringPool(0),
	mTempVarBB(0) {
}

void Builder::setRuntime(Runtime *r) {
	mRuntime = r;

	//double    @llvm.pow.f64(double %Val, double %Power)
	llvm::Type *params[2] = {getDoubleTy(), getDoubleTy()};
	llvm::FunctionType *funcTy = llvm::FunctionType::get(getDoubleTy(), params, false);
	mPowFF = llvm::cast<llvm::Function>(r->module()->getOrInsertFunction("llvm.pow.f64", funcTy));

	//double    @llvm.powi.f64(double %Val, i32 %power)
	params[1] = getInt32Ty();
	funcTy = llvm::FunctionType::get(getDoubleTy(), params, false);
	mPowFI = llvm::cast<llvm::Function>(r->module()->getOrInsertFunction("llvm.powi.f64", funcTy));

	assert(mPowFF && mPowFI);
}

void Builder::setStringPool(StringPool *s) {
	mStringPool = s;
}

Value Builder::toInt(const Value &v) {
	if (v.valueType()->basicType() == ValueType::Integer) return v;
	if (v.isConstant()) {
		return Value(ConstantValue(v.constant().toInt()), mRuntime);
	}
	assert(v.value());
	switch (v.valueType()->basicType()) {
		case ValueType::Float: {
			llvm::Value *r = CreateFAdd(llvmValue(v), llvm::ConstantFP::get(getFloatTy(), 0.5));
			return Value(mRuntime->intValueType(), CreateFPToSI(r,getInt32Ty()), false);
		}
		case ValueType::Boolean:
		case ValueType::Byte:
		case ValueType::Short: {
			llvm::Value *r = CreateCast(llvm::CastInst::ZExt, llvmValue(v), getInt32Ty());
			return Value(mRuntime->intValueType(), r, false);
		}
		case ValueType::String: {
			llvm::Value *i = mRuntime->stringValueType()->stringToIntCast(this, llvmValue(v));
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
			return Value(mRuntime->floatValueType(),CreateSIToFP(llvmValue(v), getFloatTy()), false);
		case ValueType::Boolean:
		case ValueType::Short:
		case ValueType::Byte:
			return Value(mRuntime->floatValueType(),CreateUIToFP(llvmValue(v), getFloatTy()), false);
		case ValueType::String: {
			llvm::Value *val = mRuntime->stringValueType()->stringToFloatCast(this, llvmValue(v));
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
			llvm::Value *val = mRuntime->stringValueType()->intToStringCast(this, llvmValue(i));
			return Value(mRuntime->stringValueType(), val, false);
		}
		case ValueType::Float: {
			llvm::Value *val = mRuntime->stringValueType()->floatToStringCast(this, llvmValue(v));
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
			llvm::Value *r = CreateFAdd(llvmValue(v), llvm::ConstantFP::get(getFloatTy(), 0.5));
			return Value(mRuntime->shortValueType(), CreateFPToSI(r,getInt16Ty()), false);
		}
		case ValueType::Boolean:
		case ValueType::Byte: {
			llvm::Value *r = CreateCast(llvm::CastInst::ZExt, llvmValue(v), getInt16Ty());
			return Value(mRuntime->shortValueType(), r, false);
		}
		case ValueType::Integer: {
			llvm::Value *r = CreateCast(llvm::CastInst::Trunc, llvmValue(v), getInt16Ty());
			return Value(mRuntime->shortValueType(), r, false);
		}

		case ValueType::String: {
			llvm::Value *i = mRuntime->stringValueType()->stringToIntCast(this, llvmValue(v));
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
		return Value(ConstantValue(v.constant().toByte()), mRuntime);
	}
	assert(v.value());
	switch (v.valueType()->basicType()) {
		case ValueType::Float: {
			llvm::Value *r = CreateFAdd(llvmValue(v), llvm::ConstantFP::get(getFloatTy(), 0.5));
			return Value(mRuntime->byteValueType(), CreateFPToUI(r,getInt8Ty()), false);
		}
		case ValueType::Boolean: {
			llvm::Value *r = CreateCast(llvm::CastInst::ZExt, llvmValue(v), getInt8Ty());
			return Value(mRuntime->byteValueType(), r, false);
		}
		case ValueType::Integer:
		case ValueType::Short: {
			llvm::Value *r = CreateCast(llvm::CastInst::Trunc, llvmValue(v), getInt8Ty());
			return Value(mRuntime->byteValueType(), r, false);
		}

		case ValueType::String: {
			llvm::Value *i = mRuntime->stringValueType()->stringToIntCast(this, llvmValue(v));
			return toByte(Value(mRuntime->intValueType(), i, false));
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
			return Value(mRuntime->booleanValueType(), CreateICmpNE(llvmValue(v), getInt32(0)), false);
		case ValueType::Short:
			return Value(mRuntime->booleanValueType(), CreateICmpNE(llvmValue(v), getInt16(0)), false);
		case ValueType::Byte:
			return Value(mRuntime->booleanValueType(), CreateICmpNE(llvmValue(v), getInt8(0)), false);
		case ValueType::Float:
			return Value(mRuntime->booleanValueType(), CreateFCmpONE(llvmValue(v), llvm::ConstantFP::get(getFloatTy(), 0.0)), false);
		case ValueType::String: {
			llvm::Value *val = CreateIsNotNull(llvmValue(v));

			return Value(mRuntime->booleanValueType(), val, false);
		}
		default: {
			if (v.valueType()->isTypePointer()) {
				return Value(mRuntime->booleanValueType(), CreateIsNotNull(llvmValue(v)), false);
			}
		}
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
		return CreateLoad(v.value(), false);
	}
	return v.value();
}

llvm::Value *Builder::llvmValue(int i) {
	return getInt32(i);
}

llvm::Value *Builder::llvmValue(uint16_t i) {
	return getInt16(i);
}

llvm::Value *Builder::llvmValue(uint8_t i) {
	return getInt8(i);
}

llvm::Value *Builder::llvmValue(float i) {
	return llvm::ConstantFP::get(getFloatTy(), (double)i);
}

llvm::Value *Builder::llvmValue(const QString &s) {
	if (s.isEmpty()) {
		return mRuntime->stringValueType()->defaultValue();
	}
	return mStringPool->globalString(this, s).value();
}

llvm::Value *Builder::llvmValue(bool t) {
	return getInt1(t);
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

llvm::Value *Builder::intPtrTypeValue(const Value &v) {
	Value i = toInt(v);
	llvm::Type *targetType = getIntPtrTy(&mRuntime->dataLayout());
	if (i.isConstant()) {
		return llvm::ConstantInt::get(targetType, i.constant().toInt());
	}

	return CreateSExt(llvmValue(i), targetType);
}

llvm::Value *Builder::bitcast(llvm::Type *type, llvm::Value *val) {
	return CreateBitCast(val, type);
}


Value Builder::call(Function *func, QList<Value> &params) {
	Function::ParamList paramTypes = func->paramTypes();
	assert(func->requiredParams() <= params.size() && params.size() <= paramTypes.size());
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

Value Builder::call(const Value &funcValue, QList<Value> &params) {


	FunctionValueType *funcType = static_cast<FunctionValueType*>(funcValue.valueType());
	QList<ValueType*> paramTypes = funcType->paramTypes();
	std::vector<llvm::Value*> p;
	if (funcValue.isReference()) {
		QList<ValueType*> ::ConstIterator pi = paramTypes.begin();

		for (QList<Value>::Iterator i = params.begin(); i != params.end(); ++i) {
			//Cast to the right value type
			*i = (*pi)->cast(this, *i);
			//string destruction hack
			i->toLLVMValue(this);
			pi++;

			p.push_back(llvmValue(*i));
		}


		llvm::Value *func = CreateLoad(funcValue.value(), false);

		Value ret = Value(funcType->returnType(), CreateCall(func, p), false);
		for (QList<Value>::ConstIterator i = params.begin(); i != params.end(); ++i) {
			destruct(*i);
		}
		return ret;
	}
	else {
		QList<ValueType*> ::ConstIterator pi = paramTypes.begin();
		for (QList<Value>::Iterator i = params.begin(); i != params.end(); ++i) {
			//Cast to the right value type
			*i = (*pi)->cast(this, *i);
			//string destruction hack
			i->toLLVMValue(this);
			pi++;
		}

		bool returnInParameters = false;
		llvm::AllocaInst *returnValueAlloca = 0;
		ValueType *returnType = funcType->returnType();
		llvm::Function *function = llvm::cast<llvm::Function>(funcValue.value());
		std::vector<llvm::AllocaInst*> allocas;
		if (!function->arg_empty()) {
			llvm::Function::const_arg_iterator i = function->arg_begin();
			if (i->hasStructRetAttr()) {
				returnValueAlloca = temporaryVariable(returnType->llvmType());
				allocas.push_back(returnValueAlloca);
				p.insert(p.begin(), returnValueAlloca);
				i++;
				returnInParameters = true;
			}
			foreach(const Value &val, params) {
				if (i->getType() == val.valueType()->llvmType()->getPointerTo()) {
					if (val.isReference()) {
						p.push_back(val.value());
					}
					else {
						llvm::AllocaInst *allocaInst = temporaryVariable(val.valueType()->llvmType());
						allocas.push_back(allocaInst);
						CreateStore(val.value(), allocaInst);
						p.push_back(allocaInst);
					}
				}
				else {
					assert(i->getType() == val.valueType()->llvmType());
					p.push_back(llvmValue(val));

				}
				i++;
			}
		}

		Value ret = Value(returnType, CreateCall(function, p), false);
		for (QList<Value>::ConstIterator i = params.begin(); i != params.end(); ++i) {
			destruct(*i);
		}

		for (llvm::AllocaInst *alloc : allocas) {
			removeTemporaryVariable(alloc);
		}
		return ret;

	}

}

void Builder::branch(llvm::BasicBlock *dest) {
	CreateBr(dest);
}


void Builder::branch(const Value &cond, llvm::BasicBlock *ifTrue, llvm::BasicBlock *ifFalse) {
	CreateCondBr(llvmValue(toBoolean(cond)), ifTrue, ifFalse);
}

void Builder::returnValue(ValueType *retType, const Value &v) {
	CreateRet(llvmValue(retType->cast(this, v)));
}

void Builder::returnVoid() {
	CreateRetVoid();
}

Value Builder::defaultValue(const ValueType *valType) {
	return Value(const_cast<ValueType*>(valType), valType->defaultValue(), false);
}

void Builder::construct(VariableSymbol *var) {
	llvm::Value *allocaInst = CreateAlloca(var->valueType()->llvmType());
	var->setAlloca(allocaInst);
	CreateStore(var->valueType()->defaultValue(), allocaInst);
}

void Builder::store(const Value &ref, const Value &value) {
	assert(ref.isReference());
	assert(ref.valueType() == value.valueType());
	if (ref.valueType() == mRuntime->stringValueType()) {
		mRuntime->stringValueType()->assignString(this, ref.value(), llvmValue(value));
	}
	else if (ref.valueType()->isArray()) {
		ArrayValueType *arrayValueType = static_cast<ArrayValueType*>(ref.valueType());
		arrayValueType->assignArray(this, ref.value(), llvmValue(value));
	}
	else {
		CreateStore(llvmValue(value), ref.value());
	}
}

void Builder::store(llvm::Value *ptr, llvm::Value *val) {
	assert(ptr->getType() == val->getType()->getPointerTo());
	CreateStore(val, ptr);
}

void Builder::store(VariableSymbol *var, const Value &v) {
	store(var, llvmValue(var->valueType()->cast(this, v)));
}

void Builder::store(VariableSymbol *var, llvm::Value *val) {
	assert(var->valueType()->llvmType() == val->getType());
	if (var->valueType()->basicType() == ValueType::String) {
		mRuntime->stringValueType()->assignString(this, var->alloca_(), val);
	} else if (var->valueType()->isArray()) {
		ArrayValueType *arrayValueType = static_cast<ArrayValueType*>(var->valueType());
		arrayValueType->assignArray(this, var->alloca_(), val);
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
	Value ref(var->valueType(), var->alloca_(), true);
	return var->valueType()->generateLoad(this, ref);
}

Value Builder::load(const Value &var) {
	assert(var.isReference());
	Value ret = var.valueType()->generateLoad(this, var);
	ret.value()->setName(("loaded_as_" + var.valueType()->name()).toUtf8().data());
	return ret;
}

/*Value Builder::load(const Value &ref, const Value &index) {
	return Value(ref->valueType(), CreateLoad(arrayElementPointer(ref, index)), false);
}

Value Builder::load(const Value &ref, const QList<Value> &dims) {
	return Value(array->valueType(), CreateLoad(arrayElementPointer(array, dims)), false);
}*/

void Builder::destruct(VariableSymbol *var) {
	if (var->valueType()->basicType() == ValueType::String) {
		llvm::Value *str = CreateLoad(var->alloca_(), false);
		mRuntime->stringValueType()->destructString(this, str);
	} else if (var->valueType()->isArray()) {
		llvm::Value *arr = CreateLoad(var->alloca_(), false);
		ArrayValueType *arrValueType = static_cast<ArrayValueType*>(var->valueType());
		arrValueType->destructArray(this, arr);
	}
}

void Builder::destruct(const Value &a) {
	if (!a.isValid()) return;
	a.valueType()->generateDestructor(this, a);
}

Value Builder::nullTypePointer() {
	return Value(ConstantValue(ConstantValue::Null), mRuntime);
}

/*void Builder::initilizeArray(VariableSymbol *array, const QList<Value> &dimSizes) {
	assert(array->dimensions() == dimSizes.size());
	array->valueType()

	llvm::Value *elements = calculateArrayElementCount(dimSizes);
	int sizeOfElement = array->valueType()->size();
	llvm::Value *memSize = CreateMul(elements, llvmValue(sizeOfElement));

	llvm::Value *mem = CreateBitCast(allocate(memSize), array->valueType()->llvmType()->getPointerTo());
	memSet(mem, memSize, llvmValue(uint8_t(0)));


	CreateStore(mem, array->globalArrayData());
	CreateStore(elements, array->globalArraySize());
	fillArrayIndexMultiplierArray(array, dimSizes);
}



llvm::Value *Builder::calculateArrayElementCount(const QList<Value> &dimSizes) {

	QList<Value>::ConstIterator i = dimSizes.begin();
	llvm::Value *result = llvmValue(toInt(*i));
	for (++i; i != dimSizes.end(); i++) {
		result = CreateMul(result, llvmValue(toInt(*i)));
	}
	return result;
}

llvm::Value *Builder::calculateArrayMemorySize(ArraySymbol *array, const QList<Value> &dimSizes) {
	llvm::Value *elements = calculateArrayElementCount(dimSizes);
	int sizeOfElement = array->valueType()->size();
	return CreateMul(elements, llvmValue(sizeOfElement));
}

llvm::Value *Builder::arrayElementPointer(ArraySymbol *array, const QList<Value> &index) {
	assert(array->dimensions() == index.size());
	if (array->dimensions() == 1) {
		return CreateGEP(CreateLoad(array->globalArrayData()), llvmValue(toInt(index.first())));
	}
	else { // array->dimensions() > 1
		QList<Value>::ConstIterator i = index.begin();
		llvm::Value *arrIndex =  CreateMul(llvmValue(toInt(*i)), arrayIndexMultiplier(array, 0));
		int multIndex = 1;
		i++;
		for (QList<Value>::ConstIterator end = --index.end(); i != end; ++i) {
			arrIndex = CreateAdd(arrIndex, CreateMul(llvmValue(toInt(*i)), arrayIndexMultiplier(array, multIndex)));
			multIndex++;
		}
		arrIndex = CreateAdd(arrIndex, llvmValue(toInt(*i)));
		return CreateGEP(CreateLoad(array->globalArrayData()), arrIndex);
	}
}

llvm::Value *Builder::arrayElementPointer(ArraySymbol *array, const Value &index) {
	return CreateGEP(CreateLoad(array->globalArrayData()), llvmValue(toInt(index)));
}

llvm::Value *Builder::arrayIndexMultiplier(ArraySymbol *array, int index) {
	assert(index >= 0 && index < array->dimensions() - 1);
	llvm::Value *gepParams[2] = { llvmValue(0), llvmValue(index) };
	return CreateLoad(CreateGEP(array->globalIndexMultiplierArray(), gepParams));
}

void Builder::fillArrayIndexMultiplierArray(ArraySymbol *array, const QList<Value> &dimSizes) {
	assert(array->dimensions() == dimSizes.size());

	if (array->dimensions() > 1) {
		QList<Value>::ConstIterator i = --dimSizes.end();
		llvm::Value *multiplier = llvmValue(toInt(*i));
		int arrIndex = array->dimensions() - 2;
		while(i != dimSizes.begin()) {
			llvm::Value *gepParams[2] = { llvmValue(0), llvmValue(arrIndex) };
			llvm::Value *pointerToArrayElement = CreateGEP(array->globalIndexMultiplierArray(), gepParams);
			CreateStore(multiplier, pointerToArrayElement);

			--i;
			if (i != dimSizes.begin()) {
				multiplier = CreateMul(multiplier, llvmValue(toInt(*i)));
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

	llvm::Value *val = llvmValue(typePtrVar);
	llvm::Value *fieldPtr = CreateStructGEP(val, fieldIndex);
	return Value(field.valueType(), fieldPtr, true);
}

Value Builder::newTypeMember(TypeSymbol *type) {
	llvm::Value *typePtr = CreateCall(mRuntime->typeValueType()->newFunction(), type->globalTypeVariable());
	return Value(type->typePointerValueType(), bitcast(type->typePointerValueType()->llvmType(), typePtr), false);
}

Value Builder::firstTypeMember(TypeSymbol *type) {
	llvm::Value *typePtr = CreateCall(mRuntime->typeValueType()->firstFunction(), type->globalTypeVariable());
	return Value(type->typePointerValueType(), bitcast(type->typePointerValueType()->llvmType(), typePtr), false);
}

Value Builder::lastTypeMember(TypeSymbol *type) {
	llvm::Value *typePtr = CreateCall(mRuntime->typeValueType()->lastFunction(), type->globalTypeVariable());
	return Value(type->typePointerValueType(), bitcast(type->typePointerValueType()->llvmType(), typePtr), false);
}

Value Builder::afterTypeMember(const Value &ptr) {
	assert(ptr.valueType()->isTypePointer());
	llvm::Value *param = bitcast(mRuntime->typePointerCommonValueType()->llvmType(), llvmValue(ptr));
	llvm::Value *typePtr = CreateCall(mRuntime->typeValueType()->afterFunction(), param);
	return Value(ptr.valueType(), bitcast(ptr.valueType()->llvmType(), typePtr), false);
}

Value Builder::beforeTypeMember(const Value &ptr) {
	assert(ptr.valueType()->isTypePointer());
	llvm::Value *param = bitcast(mRuntime->typePointerCommonValueType()->llvmType(), llvmValue(ptr));
	llvm::Value *typePtr = CreateCall(mRuntime->typeValueType()->beforeFunction(), param);
	return Value(ptr.valueType(), bitcast(ptr.valueType()->llvmType(), typePtr), false);
}

void Builder::deleteTypeMember(const Value &ptr) {
	llvm::Value *param = bitcast(mRuntime->typePointerCommonValueType()->llvmType(), llvmValue(ptr));
	llvm::Value *typePtr = CreateCall(mRuntime->typeValueType()->deleteFunction(), param);
	Value val = Value(ptr.valueType(), bitcast(ptr.valueType()->llvmType(), typePtr), false);
	if (ptr.isReference()) {
		store(ptr, val);
	}
}

Value Builder::typePointerNotNull(const Value &ptr) {
	assert(ptr.valueType()->isTypePointer());
	return Value(mRuntime->booleanValueType(), CreateIsNotNull(llvmValue(ptr)), false);
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

	return Value(mRuntime->booleanValueType(), CreateNot(llvmValue(toBoolean(a))), false);
}

Value Builder::minus(const Value &a) {
	if (a.isConstant()) {
		OperationFlags flags;
		return Value(ConstantValue::minus(a.constant(), flags), mRuntime);
	}

	if (a.valueType()->basicType() == ValueType::Boolean) {
		return Value(mRuntime->intValueType(), CreateNeg(llvmValue(toInt(a))), false);
	}
	if (a.valueType()->basicType() == ValueType::Float) {
		return Value(a.valueType(), CreateFNeg(llvmValue(a)), false);
	}
	return Value(a.valueType(), CreateNeg(llvmValue(a)), false);
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
					result = CreateAdd(llvmValue(toInt(a)), llvmValue(toInt(b)));
					return Value(mRuntime->intValueType(), result, false);
				case ValueType::Float:
					result = CreateFAdd(llvmValue(toFloat(a)), llvmValue(b));
					return Value(mRuntime->floatValueType(), result, false);
				case ValueType::Short:
				case ValueType::Byte:
					result = CreateAdd(llvmValue(toInt(a)), llvmValue(toInt(b)));
					return Value(mRuntime->intValueType(), result, false);
				case ValueType::String: {
					Value as = toString(a);
					result = mRuntime->stringValueType()->stringAddition(this, llvmValue(as), llvmValue(b));
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
					result = CreateFAdd(llvmValue(a), llvmValue(toFloat(b)));
					return Value(mRuntime->floatValueType(), result, false);
				case ValueType::Float:
					result = CreateFAdd(llvmValue(a), llvmValue(b));
					return Value(mRuntime->floatValueType(), result, false);
				case ValueType::String: {
					Value as = toString(a);
					result = mRuntime->stringValueType()->stringAddition(this, llvmValue(as), llvmValue(b));
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
					result = CreateAdd(llvmValue(toInt(a)), llvmValue(toInt(b)));
					return Value(mRuntime->intValueType(), result);
				case ValueType::Float:
					result = CreateFAdd(llvmValue(toFloat(a)), llvmValue(b));
					return Value(mRuntime->floatValueType(), result);
				case ValueType::Short:
				case ValueType::Byte:
					result = CreateAdd(llvmValue(a), llvmValue(toInt(b)));
					return Value(mRuntime->intValueType(), result);
				case ValueType::String: {
					Value as = toString(a);
					result = mRuntime->stringValueType()->stringAddition(this, llvmValue(as), llvmValue(b));
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
					result = CreateAdd(llvmValue(toInt(a)), llvmValue(toInt(b)));
					return Value(mRuntime->intValueType(), result);
				case ValueType::Float:
					result = CreateFAdd(llvmValue(toFloat(a)), llvmValue(b));
					return Value(mRuntime->floatValueType(), result);
				case ValueType::Short:
					result = CreateAdd(llvmValue(toShort(a)), llvmValue(b));
					return Value(mRuntime->intValueType(), result);
				case ValueType::Byte:
					result = CreateAdd(llvmValue(a), llvmValue(b));
					return Value(mRuntime->intValueType(), result);
				case ValueType::String: {
					Value as = toString(a);
					result = mRuntime->stringValueType()->stringAddition(this, llvmValue(as), llvmValue(b));
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
					result = mRuntime->stringValueType()->stringAddition(this, llvmValue(a), llvmValue(as));
					destruct(as);
					return Value(mRuntime->stringValueType(), result);
				}
				case ValueType::String: {
					result = mRuntime->stringValueType()->stringAddition(this, llvmValue(a), llvmValue(b));
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
					result = CreateSub(llvmValue(toInt(a)), llvmValue(toInt(b)));
					return Value(mRuntime->intValueType(), result);
				case ValueType::Float:
					result = CreateFSub(llvmValue(toFloat(a)), llvmValue(b));
					return Value(mRuntime->floatValueType(), result);
				case ValueType::Short:
				case ValueType::Byte:
					result = CreateSub(llvmValue(toInt(a)), llvmValue(toInt(b)));
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
					result = CreateFSub(llvmValue(a), llvmValue(toFloat(b)));
					return Value(mRuntime->floatValueType(), result);
				case ValueType::Float:
					result = CreateFSub(llvmValue(a), llvmValue(b));
					return Value(mRuntime->floatValueType(), result);
				default: break;
			}
			break;
		case ValueType::Short:
			switch(b.valueType()->basicType()) {
				case ValueType::Integer:
				case ValueType::Boolean:
					result = CreateSub(llvmValue(toInt(a)), llvmValue(toInt(b)));
					return Value(mRuntime->intValueType(), result);
				case ValueType::Float:
					result = CreateFSub(llvmValue(toFloat(a)), llvmValue(b));
					return Value(mRuntime->floatValueType(), result);
				case ValueType::Short:
				case ValueType::Byte:
					result = CreateSub(llvmValue(a), llvmValue(toInt(b)));
					return Value(mRuntime->intValueType(), result);
				default: break;
			}
			break;
		case ValueType::Byte:
			switch (b.valueType()->basicType()) {
				case ValueType::Integer:
				case ValueType::Boolean:
					result = CreateSub(llvmValue(toInt(a)), llvmValue(toInt(b)));
					return Value(mRuntime->intValueType(), result);
				case ValueType::Float:
					result = CreateFSub(llvmValue(toFloat(a)), llvmValue(b));
					return Value(mRuntime->floatValueType(), result);
				case ValueType::Short:
					result = CreateSub(llvmValue(toShort(a)), llvmValue(b));
					return Value(mRuntime->intValueType(), result);
				case ValueType::Byte:
					result = CreateSub(llvmValue(a), llvmValue(b));
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
					result = CreateMul(llvmValue(toInt(a)), llvmValue(toInt(b)));
					return Value(mRuntime->intValueType(), result);
				case ValueType::Float:
					result = CreateFMul(llvmValue(toFloat(a)), llvmValue(b));
					return Value(mRuntime->floatValueType(), result);
				case ValueType::Short:
				case ValueType::Byte:
					result = CreateMul(llvmValue(toInt(a)), llvmValue(toInt(b)));
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
					result = CreateFMul(llvmValue(a), llvmValue(toFloat(b)));
					return Value(mRuntime->floatValueType(), result);
				case ValueType::Float:
					result = CreateFMul(llvmValue(a), llvmValue(b));
					return Value(mRuntime->floatValueType(), result);
				default: break;
			}
			break;
		case ValueType::Short:
			switch(b.valueType()->basicType()) {
				case ValueType::Integer:
				case ValueType::Boolean:
					result = CreateMul(llvmValue(toInt(a)), llvmValue(toInt(b)));
					return Value(mRuntime->intValueType(), result);
				case ValueType::Float:
					result = CreateFMul(llvmValue(toFloat(a)), llvmValue(b));
					return Value(mRuntime->floatValueType(), result);
				case ValueType::Short:
				case ValueType::Byte:
					result = CreateMul(llvmValue(a), llvmValue(toInt(b)));
					return Value(mRuntime->intValueType(), result);
				default: break;
			}
			break;
		case ValueType::Byte:
			switch (b.valueType()->basicType()) {
				case ValueType::Integer:
				case ValueType::Boolean:
					result = CreateMul(llvmValue(toInt(a)), llvmValue(toInt(b)));
					return Value(mRuntime->intValueType(), result);
				case ValueType::Float:
					result = CreateFMul(llvmValue(toFloat(a)), llvmValue(b));
					return Value(mRuntime->floatValueType(), result);
				case ValueType::Short:
					result = CreateMul(llvmValue(toShort(a)), llvmValue(b));
					return Value(mRuntime->intValueType(), result);
				case ValueType::Byte:
					result = CreateMul(llvmValue(a), llvmValue(b));
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
		ConstantValue constVal = ConstantValue::divide(a.constant(), b.constant(), flags);
		assert(constVal.isValid());
		return Value(constVal, mRuntime);
	}
	llvm::Value *result;
	switch(a.valueType()->basicType()) {
		case ValueType::Boolean:
		case ValueType::Integer:
			switch(b.valueType()->basicType()) {
				case ValueType::Integer:
				case ValueType::Boolean:
					result = CreateSDiv(llvmValue(toInt(a)), llvmValue(toInt(b)));
					return Value(mRuntime->intValueType(), result);
				case ValueType::Float:
					result = CreateFDiv(llvmValue(toFloat(a)), llvmValue(b));
					return Value(mRuntime->floatValueType(), result);
				case ValueType::Short:
				case ValueType::Byte:
					result = CreateUDiv(llvmValue(toInt(a)), llvmValue(toInt(b)));
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
					result = CreateFDiv(llvmValue(a), llvmValue(toFloat(b)));
					return Value(mRuntime->floatValueType(), result);
				case ValueType::Float:
					result = CreateFDiv(llvmValue(a), llvmValue(b));
					return Value(mRuntime->floatValueType(), result);
				default: break;
			}
			break;
		case ValueType::Short:
			switch(b.valueType()->basicType()) {
				case ValueType::Integer:
				case ValueType::Boolean:
					result = CreateSDiv(llvmValue(toInt(a)), llvmValue(toInt(b)));
					return Value(mRuntime->intValueType(), result);
				case ValueType::Float:
					result = CreateFDiv(llvmValue(toFloat(a)), llvmValue(b));
					return Value(mRuntime->floatValueType(), result);
				case ValueType::Short:
				case ValueType::Byte:
					result = CreateUDiv(llvmValue(a), llvmValue(toInt(b)));
					return Value(mRuntime->intValueType(), result);
				default: break;
			}
			break;
		case ValueType::Byte:
			switch (b.valueType()->basicType()) {
				case ValueType::Integer:
				case ValueType::Boolean:
					result = CreateSDiv(llvmValue(toInt(a)), llvmValue(toInt(b)));
					return Value(mRuntime->intValueType(), result);
				case ValueType::Float:
					result = CreateFDiv(llvmValue(toFloat(a)), llvmValue(b));
					return Value(mRuntime->floatValueType(), result);
				case ValueType::Short:
				case ValueType::Byte:
					result = CreateUDiv(llvmValue(toShort(a)), llvmValue(b));
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
					result = CreateSRem(llvmValue(toInt(a)), llvmValue(toInt(b)));
					return Value(mRuntime->intValueType(), result);
				case ValueType::Float:
					result = CreateFRem(llvmValue(toFloat(a)), llvmValue(b));
					return Value(mRuntime->floatValueType(), result);
				case ValueType::Short:
				case ValueType::Byte:
					result = CreateURem(llvmValue(toInt(a)), llvmValue(toInt(b)));
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
					result = CreateFRem(llvmValue(a), llvmValue(toFloat(b)));
					return Value(mRuntime->floatValueType(), result);
				case ValueType::Float:
					result = CreateFRem(llvmValue(a), llvmValue(b));
					return Value(mRuntime->floatValueType(), result);
				default: break;
			}
			break;
		case ValueType::Short:
			switch(b.valueType()->basicType()) {
				case ValueType::Integer:
				case ValueType::Boolean:
					result = CreateSRem(llvmValue(toInt(a)), llvmValue(toInt(b)));
					return Value(mRuntime->intValueType(), result);
				case ValueType::Float:
					result = CreateFRem(llvmValue(toFloat(a)), llvmValue(b));
					return Value(mRuntime->floatValueType(), result);
				case ValueType::Short:
				case ValueType::Byte:
					result = CreateURem(llvmValue(a), llvmValue(toInt(b)));
					return Value(mRuntime->intValueType(), result);
				default: break;
			}
			break;
		case ValueType::Byte:
			switch (b.valueType()->basicType()) {
				case ValueType::Integer:
				case ValueType::Boolean:
					result = CreateSRem(llvmValue(toInt(a)), llvmValue(toInt(b)));
					return Value(mRuntime->intValueType(), result);
				case ValueType::Float:
					result = CreateFRem(llvmValue(toFloat(a)), llvmValue(b));
					return Value(mRuntime->floatValueType(), result);
				case ValueType::Short:
				case ValueType::Byte:
					result = CreateURem(llvmValue(toShort(a)), llvmValue(b));
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
		return Value(mRuntime->intValueType(), CreateShl(llvmValue(toInt(a)), llvmValue(toInt(b))));
	}
	if (a.valueType()->basicType() == ValueType::Short || b.valueType()->basicType() == ValueType::Short) {
		return Value(mRuntime->shortValueType(), CreateShl(llvmValue(toShort(a)), llvmValue(toShort(b))));
	}

	return Value(mRuntime->byteValueType(), CreateShl(llvmValue(toByte(a)), llvmValue(toByte(b))));
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
		return Value(mRuntime->intValueType(), CreateLShr(llvmValue(toInt(a)), llvmValue(toInt(b))));
	}
	if (a.valueType()->basicType() == ValueType::Short || b.valueType()->basicType() == ValueType::Short) {
		return Value(mRuntime->shortValueType(), CreateLShr(llvmValue(toShort(a)), llvmValue(toShort(b))));
	}

	return Value(mRuntime->byteValueType(), CreateLShr(llvmValue(toByte(a)), llvmValue(toByte(b))));
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
		return Value(mRuntime->intValueType(), CreateAShr(llvmValue(toInt(a)), llvmValue(toInt(b))));
	}
	if (a.valueType()->basicType() == ValueType::Short || b.valueType()->basicType() == ValueType::Short) {
		return Value(mRuntime->shortValueType(), CreateAShr(llvmValue(toShort(a)), llvmValue(toShort(b))));
	}

	return Value(mRuntime->byteValueType(), CreateAShr(llvmValue(toByte(a)), llvmValue(toByte(b))));
}

Value Builder::and_(const Value &a, const Value &b) {
	if (a.isConstant() && b.isConstant()) {
		OperationFlags flags;
		return Value(ConstantValue::and_(a.constant(), b.constant(), flags), mRuntime);
	}
	return Value(mRuntime->booleanValueType(), CreateAnd(llvmValue(toBoolean(a)), llvmValue(toBoolean(b))));
}

Value Builder::or_(const Value &a, const Value &b) {
	if (a.isConstant() && b.isConstant()) {
		OperationFlags flags;
		return Value(ConstantValue::or_(a.constant(), b.constant(), flags), mRuntime);
	}
	return Value(mRuntime->booleanValueType(), CreateOr(llvmValue(toBoolean(a)), llvmValue(toBoolean(b))));
}

Value Builder::xor_(const Value &a, const Value &b) {
	if (a.isConstant() && b.isConstant()) {
		OperationFlags flags;
		return Value(ConstantValue::xor_(a.constant(),b.constant(), flags), mRuntime);
	}
	return Value(mRuntime->booleanValueType(), CreateXor(llvmValue(toBoolean(a)), llvmValue(toBoolean(b))));
}

Value Builder::power(const Value &a, const Value &b) {
	if (a.isConstant() && b.isConstant()) {
		OperationFlags flags;
		return Value(ConstantValue::power(a.constant(), b.constant(), flags), mRuntime);
	}
	if (a.valueType()->basicType() == ValueType::Float || b.valueType()->basicType() == ValueType::Float) {
		llvm::Value *af = llvmValue(toFloat(a));
		llvm::Value *bf = llvmValue(toFloat(b));
		llvm::Value *ad = CreateFPExt(af, getDoubleTy());
		llvm::Value *bd = CreateFPExt(bf, getDoubleTy());
		llvm::Value *retD = CreateCall2(mPowFF, ad, bd);
		return Value(mRuntime->floatValueType(), CreateFPTrunc(retD, getFloatTy()));
	}
	llvm::Value *af = llvmValue(toFloat(a));
	llvm::Value *ad = CreateFPExt(af, getDoubleTy());
	llvm::Value *retD = CreateCall2(mPowFI, ad, llvmValue(toInt(b)));
	llvm::Value *greaterCond = CreateFCmpOGT(retD, llvm::ConstantFP::get( getDoubleTy(), (double)INT_MAX));
	llvm::Value *lessCond = CreateFCmpOLT(retD, llvm::ConstantFP::get( getDoubleTy(), (double)-INT_MAX));
	llvm::Value *cond = CreateOr(greaterCond, lessCond);
	llvm::Value *ret = CreateSelect(cond, llvmValue(-INT_MAX), CreateFPToSI(retD, getInt32Ty()));
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
					return Value(mRuntime->booleanValueType(), CreateICmpSLT(llvmValue(toInt(a)), llvmValue(toInt(b))));
				case ValueType::Float:
					return Value(mRuntime->booleanValueType(), CreateFCmpOLT(llvmValue(toFloat(a)), llvmValue(toFloat(b))));
				default: break;
			}
			break;
		case ValueType::Short:
			switch (b.valueType()->basicType()) {
				case ValueType::Boolean:
				case ValueType::Integer:
					return Value(mRuntime->booleanValueType(), CreateICmpSLT(llvmValue(toInt(a)), llvmValue(toInt(b))));
				case ValueType::Short:
				case ValueType::Byte:
					return Value(mRuntime->booleanValueType(), CreateICmpULT(llvmValue(toShort(a)), llvmValue(toShort(b))));
				case ValueType::Float:
					return Value(mRuntime->booleanValueType(), CreateFCmpOLT(llvmValue(toFloat(a)), llvmValue(toFloat(b))));
				default: break;
			}
			break;
		case ValueType::Byte:
			switch (b.valueType()->basicType()) {
				case ValueType::Boolean:
				case ValueType::Integer:
					return Value(mRuntime->booleanValueType(), CreateICmpSLT(llvmValue(toInt(a)), llvmValue(toInt(b))));
				case ValueType::Short:
					return Value(mRuntime->booleanValueType(), CreateICmpULT(llvmValue(toShort(a)), llvmValue(toShort(b))));
				case ValueType::Byte:
					return Value(mRuntime->booleanValueType(), CreateICmpULT(llvmValue(a), llvmValue(b)));
				case ValueType::Float:
					return Value(mRuntime->booleanValueType(), CreateFCmpOLT(llvmValue(toFloat(a)), llvmValue(toFloat(b))));
				default: break;
			}
			break;
		case ValueType::Float:
			return Value(mRuntime->booleanValueType(), CreateFCmpOLT(llvmValue(toFloat(a)), llvmValue(toFloat(b))));
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
					return Value(mRuntime->booleanValueType(), CreateICmpSLE(llvmValue(toInt(a)), llvmValue(toInt(b))));
				case ValueType::Float:
					return Value(mRuntime->booleanValueType(), CreateFCmpOLE(llvmValue(toFloat(a)), llvmValue(toFloat(b))));
				default: break;
			}
			break;
		case ValueType::Short:
			switch (b.valueType()->basicType()) {
				case ValueType::Boolean:
				case ValueType::Integer:
					return Value(mRuntime->booleanValueType(), CreateICmpSLE(llvmValue(toInt(a)), llvmValue(toInt(b))));
				case ValueType::Short:
				case ValueType::Byte:
					return Value(mRuntime->booleanValueType(), CreateICmpULE(llvmValue(toShort(a)), llvmValue(toShort(b))));
				case ValueType::Float:
					return Value(mRuntime->booleanValueType(), CreateFCmpOLE(llvmValue(toFloat(a)), llvmValue(toFloat(b))));
				default: break;
			}
			break;
		case ValueType::Byte:
			switch (b.valueType()->basicType()) {
				case ValueType::Boolean:
				case ValueType::Integer:
					return Value(mRuntime->booleanValueType(), CreateICmpSLE(llvmValue(toInt(a)), llvmValue(toInt(b))));
				case ValueType::Short:
					return Value(mRuntime->booleanValueType(), CreateICmpULE(llvmValue(toShort(a)), llvmValue(toShort(b))));
				case ValueType::Byte:
					return Value(mRuntime->booleanValueType(), CreateICmpULE(llvmValue(a), llvmValue(b)));
				case ValueType::Float:
					return Value(mRuntime->booleanValueType(), CreateFCmpOLE(llvmValue(toFloat(a)), llvmValue(toFloat(b))));
				default: break;
			}
			break;
		case ValueType::Float:
			return Value(mRuntime->booleanValueType(), CreateFCmpOLE(llvmValue(toFloat(a)), llvmValue(toFloat(b))));
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
					return Value(mRuntime->booleanValueType(), CreateICmpSGT(llvmValue(toInt(a)), llvmValue(toInt(b))));
				case ValueType::Float:
					return Value(mRuntime->booleanValueType(), CreateFCmpOGT(llvmValue(toFloat(a)), llvmValue(toFloat(b))));
				default: break;
			}
			break;
		case ValueType::Short:
			switch (b.valueType()->basicType()) {
				case ValueType::Boolean:
				case ValueType::Integer:
					return Value(mRuntime->booleanValueType(), CreateICmpSGT(llvmValue(toInt(a)), llvmValue(toInt(b))));
				case ValueType::Short:
				case ValueType::Byte:
					return Value(mRuntime->booleanValueType(), CreateICmpUGT(llvmValue(toShort(a)), llvmValue(toShort(b))));
				case ValueType::Float:
					return Value(mRuntime->booleanValueType(), CreateFCmpOGT(llvmValue(toFloat(a)), llvmValue(toFloat(b))));
				default: break;
			}
			break;
		case ValueType::Byte:
			switch (b.valueType()->basicType()) {
				case ValueType::Boolean:
				case ValueType::Integer:
					return Value(mRuntime->booleanValueType(), CreateICmpSGT(llvmValue(toInt(a)), llvmValue(toInt(b))));
				case ValueType::Short:
					return Value(mRuntime->booleanValueType(), CreateICmpUGT(llvmValue(toShort(a)), llvmValue(toShort(b))));
				case ValueType::Byte:
					return Value(mRuntime->booleanValueType(), CreateICmpUGT(llvmValue(a), llvmValue(b)));
				case ValueType::Float:
					return Value(mRuntime->booleanValueType(), CreateFCmpOGT(llvmValue(toFloat(a)), llvmValue(toFloat(b))));
				default: break;
			}
			break;
		case ValueType::Float:
			return Value(mRuntime->booleanValueType(), CreateFCmpOGT(llvmValue(toFloat(a)), llvmValue(toFloat(b))));
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
					return Value(mRuntime->booleanValueType(), CreateICmpSGE(llvmValue(toInt(a)), llvmValue(toInt(b))));
				case ValueType::Float:
					return Value(mRuntime->booleanValueType(), CreateFCmpOGE(llvmValue(toFloat(a)), llvmValue(toFloat(b))));
				default: break;
			}
			break;
		case ValueType::Short:
			switch (b.valueType()->basicType()) {
				case ValueType::Boolean:
				case ValueType::Integer:
					return Value(mRuntime->booleanValueType(), CreateICmpSGE(llvmValue(toInt(a)), llvmValue(toInt(b))));
				case ValueType::Short:
				case ValueType::Byte:
					return Value(mRuntime->booleanValueType(), CreateICmpUGE(llvmValue(toShort(a)), llvmValue(toShort(b))));
				case ValueType::Float:
					return Value(mRuntime->booleanValueType(), CreateFCmpOGE(llvmValue(toFloat(a)), llvmValue(toFloat(b))));
				default: break;
			}
			break;
		case ValueType::Byte:
			switch (b.valueType()->basicType()) {
				case ValueType::Boolean:
				case ValueType::Integer:
					return Value(mRuntime->booleanValueType(), CreateICmpSGE(llvmValue(toInt(a)), llvmValue(toInt(b))));
				case ValueType::Short:
					return Value(mRuntime->booleanValueType(), CreateICmpUGE(llvmValue(toShort(a)), llvmValue(toShort(b))));
				case ValueType::Byte:
					return Value(mRuntime->booleanValueType(), CreateICmpUGE(llvmValue(a), llvmValue(b)));
				case ValueType::Float:
					return Value(mRuntime->booleanValueType(), CreateFCmpOGE(llvmValue(toFloat(a)), llvmValue(toFloat(b))));
				default: break;
			}
			break;
		case ValueType::Float:
			return Value(mRuntime->booleanValueType(), CreateFCmpOGE(llvmValue(toFloat(a)), llvmValue(toFloat(b))));
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
					return Value(mRuntime->booleanValueType(), CreateICmpEQ(llvmValue(a), llvmValue(toInt(b))));
				case ValueType::Float:
					return Value(mRuntime->booleanValueType(), CreateFCmpOEQ(llvmValue(toFloat(a)), llvmValue(b)));
				case ValueType::String: {
					Value as = toString(a);
					llvm::Value *ret = mRuntime->stringValueType()->stringEquality(this, llvmValue(as), llvmValue(b));
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
					return Value(mRuntime->booleanValueType(), CreateFCmpOEQ(llvmValue(a), llvmValue(toFloat(b))));
				case ValueType::String: {
					Value as = toString(a);
					llvm::Value *ret = mRuntime->stringValueType()->stringEquality(this, llvmValue(as), llvmValue(b));
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
					return Value(mRuntime->booleanValueType(), CreateICmpEQ(llvmValue(a), llvmValue(b)));
				case ValueType::Byte:
					return Value(mRuntime->booleanValueType(), CreateICmpEQ(llvmValue(toByte(a)), llvmValue(b)));
				case ValueType::Short:
					return Value(mRuntime->booleanValueType(), CreateICmpEQ(llvmValue(toShort(a)), llvmValue(b)));
				case ValueType::Integer:
					return Value(mRuntime->booleanValueType(), CreateICmpEQ(llvmValue(toInt(a)), llvmValue(b)));
				case ValueType::Float:
					return Value(mRuntime->booleanValueType(), CreateICmpEQ(llvmValue(toFloat(a)), llvmValue(b)));
				case ValueType::String: {
					Value as = toString(a);
					llvm::Value *ret = mRuntime->stringValueType()->stringEquality(this, llvmValue(as), llvmValue(b));
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
					return Value(mRuntime->booleanValueType(), CreateICmpEQ(llvmValue(a), llvmValue(toShort(b))));
				case ValueType::Integer:
					return Value(mRuntime->booleanValueType(), CreateICmpEQ(llvmValue(toInt(a)), llvmValue(b)));
				case ValueType::Float:
					return Value(mRuntime->booleanValueType(), CreateFCmpOEQ(llvmValue(toFloat(a)), llvmValue(b)));
				case ValueType::String: {
					Value as = toString(a);
					llvm::Value *ret = mRuntime->stringValueType()->stringEquality(this, llvmValue(as), llvmValue(b));
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
					return Value(mRuntime->booleanValueType(), CreateICmpEQ(llvmValue(a), llvmValue(toByte(b))));
				case ValueType::Short:
					return Value(mRuntime->booleanValueType(), CreateICmpEQ(llvmValue(toShort(a)), llvmValue(b)));
				case ValueType::Integer:
					return Value(mRuntime->booleanValueType(), CreateICmpEQ(llvmValue(toInt(a)), llvmValue(b)));
				case ValueType::Float:
					return Value(mRuntime->booleanValueType(), CreateFCmpOEQ(llvmValue(toFloat(a)), llvmValue(b)));
				case ValueType::String: {
					Value as = toString(a);
					llvm::Value *ret = mRuntime->stringValueType()->stringEquality(this, llvmValue(as), llvmValue(b));
					destruct(as);
					destruct(b);
					return Value(mRuntime->booleanValueType(), ret);
				}
				default: break;
			}
			break;
		case ValueType::String: {
			if (b.valueType()->basicType() == ValueType::String) {
				llvm::Value *ret = mRuntime->stringValueType()->stringEquality(this, llvmValue(a), llvmValue(b));
				return Value(mRuntime->booleanValueType(), ret);
			}
			Value bs = toString(b);
			llvm::Value *ret = mRuntime->stringValueType()->stringEquality(this, llvmValue(a), llvmValue(bs));
			destruct(bs);
			destruct(a);
			return Value(mRuntime->booleanValueType(), ret);
		}
		default:
			if (a.valueType()->isTypePointer() && b.valueType()->isTypePointer()) {
				llvm::Value *ret;
				if (a.valueType() == b.valueType()) {
					ret = CreateICmpEQ(llvmValue(a), llvmValue(b));
				}
				else {
					ret = CreateICmpEQ(llvmValue(a), bitcast(a.valueType()->llvmType(), llvmValue(b)));
				}
				return Value(mRuntime->booleanValueType(), ret, false);
			}
	}

	assert("Invalid equality operation" && 0); return Value();
}

Value Builder::ptrEqual(llvm::Value *a, llvm::Value *b) {
	return Value(mRuntime->booleanValueType(), CreateICmpEQ(a, b), false);
}


Value Builder::notEqual(const Value &a, const Value &b) {
	if (a.isConstant() && b.isConstant()) {
		OperationFlags flags;
		return Value(ConstantValue::notEqual(a.constant(), b.constant(), flags), mRuntime);
	}
	switch (a.valueType()->basicType()) {
		case ValueType::Integer:
			switch (b.valueType()->basicType()) {
				case ValueType::Short:
				case ValueType::Byte:
				case ValueType::Integer:
				case ValueType::Boolean:
					return Value(mRuntime->booleanValueType(), CreateICmpNE(llvmValue(a), llvmValue(toInt(b))));
				case ValueType::Float:
					return Value(mRuntime->booleanValueType(), CreateFCmpONE(llvmValue(toFloat(a)), llvmValue(b)));
				case ValueType::String: {
					//TODO: StringValueType::stringUnequality?
					Value as = toString(a);
					llvm::Value *ret = mRuntime->stringValueType()->stringEquality(this, llvmValue(as), llvmValue(b));
					destruct(b);
					destruct(as);
					return Value(mRuntime->booleanValueType(), CreateNot(ret));
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
					return Value(mRuntime->booleanValueType(), CreateFCmpONE(llvmValue(a), llvmValue(toFloat(b))));
				case ValueType::String: {
					Value as = toString(a);
					llvm::Value *ret = mRuntime->stringValueType()->stringEquality(this, llvmValue(as), llvmValue(b));
					destruct(b);
					destruct(as);
					return Value(mRuntime->booleanValueType(), CreateNot(ret));
				}
				default: break;
			}
			break;
		case ValueType::Boolean:
			switch (b.valueType()->basicType()) {
				case ValueType::Boolean:
					return Value(mRuntime->booleanValueType(), CreateICmpNE(llvmValue(a), llvmValue(b)));
				case ValueType::Byte:
					return Value(mRuntime->booleanValueType(), CreateICmpNE(llvmValue(toByte(a)), llvmValue(b)));
				case ValueType::Short:
					return Value(mRuntime->booleanValueType(), CreateICmpNE(llvmValue(toShort(a)), llvmValue(b)));
				case ValueType::Integer:
					return Value(mRuntime->booleanValueType(), CreateICmpNE(llvmValue(toInt(a)), llvmValue(b)));
				case ValueType::Float:
					return Value(mRuntime->booleanValueType(), CreateICmpNE(llvmValue(toFloat(a)), llvmValue(b)));
				case ValueType::String: {
					Value as = toString(a);
					llvm::Value *ret = mRuntime->stringValueType()->stringEquality(this, llvmValue(as), llvmValue(b));
					destruct(as);
					destruct(b);
					return Value(mRuntime->booleanValueType(), CreateNot(ret));
				}
				default: break;
			}
			break;
		case ValueType::Short:
			switch (b.valueType()->basicType()) {
				case ValueType::Boolean:
				case ValueType::Short:
				case ValueType::Byte:
					return Value(mRuntime->booleanValueType(), CreateICmpNE(llvmValue(a), llvmValue(toShort(b))));
				case ValueType::Integer:
					return Value(mRuntime->booleanValueType(), CreateICmpNE(llvmValue(toInt(a)), llvmValue(b)));
				case ValueType::Float:
					return Value(mRuntime->booleanValueType(), CreateFCmpONE(llvmValue(toFloat(a)), llvmValue(b)));
				case ValueType::String: {
					Value as = toString(a);
					llvm::Value *ret = mRuntime->stringValueType()->stringEquality(this, llvmValue(as), llvmValue(b));
					destruct(as);
					destruct(b);
					return Value(mRuntime->booleanValueType(), CreateNot(ret));
				}
				default: break;
			}
			break;
		case ValueType::Byte:
			switch (b.valueType()->basicType()) {
				case ValueType::Boolean:
				case ValueType::Byte:
					return Value(mRuntime->booleanValueType(), CreateICmpNE(llvmValue(a), llvmValue(toByte(b))));
				case ValueType::Short:
					return Value(mRuntime->booleanValueType(), CreateICmpNE(llvmValue(toShort(a)), llvmValue(b)));
				case ValueType::Integer:
					return Value(mRuntime->booleanValueType(), CreateICmpNE(llvmValue(toInt(a)), llvmValue(b)));
				case ValueType::Float:
					return Value(mRuntime->booleanValueType(), CreateFCmpONE(llvmValue(toFloat(a)), llvmValue(b)));
				case ValueType::String: {
					Value as = toString(a);
					llvm::Value *ret = mRuntime->stringValueType()->stringEquality(this, llvmValue(as), llvmValue(b));
					destruct(as);
					return Value(mRuntime->booleanValueType(), CreateNot(ret));
				}
				default: break;
			}
			break;
		case ValueType::String: {
			if (b.valueType()->basicType() == ValueType::String) {
				llvm::Value *ret = mRuntime->stringValueType()->stringEquality(this, llvmValue(a), llvmValue(b));
				return Value(mRuntime->booleanValueType(), CreateNot(ret));
			}
			Value bs = toString(b);
			llvm::Value *ret = mRuntime->stringValueType()->stringEquality(this, llvmValue(a), llvmValue(bs));
			destruct(bs);
			return Value(mRuntime->booleanValueType(), CreateNot(ret));
		}
		default:
			if (a.valueType()->isTypePointer() && b.valueType()->isTypePointer()) {
				llvm::Value *ret;
				if (a.valueType() == b.valueType()) {
					ret = CreateICmpNE(llvmValue(a), llvmValue(b));
				}
				else {
					ret = CreateICmpNE(llvmValue(a), bitcast(a.valueType()->llvmType(), llvmValue(b)));
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
		src = CreateBitCast(src, llvm::IntegerType::get(context(), 8)->getPointerTo());
	}
	if (dest->getType() != llvm::IntegerType::get(context(), 8)->getPointerTo()) {
		dest = CreateBitCast(dest, llvm::IntegerType::get(context(), 8)->getPointerTo());
	}

	CreateMemCpy(dest, src, num, align);
}

void Builder::memSet(llvm::Value *ptr, llvm::Value *num, llvm::Value *value, int align) {
	assert(ptr->getType()->isPointerTy());
	assert(num->getType() == llvm::IntegerType::get(context(), 32));
	assert(value->getType() == llvm::IntegerType::get(context(), 8));

	//Cast src and dest to i8* if they are not already
	if (ptr->getType() != llvm::IntegerType::get(context(), 8)->getPointerTo()) {
		ptr = CreateBitCast(ptr, llvm::IntegerType::get(context(), 8)->getPointerTo());
	}

	CreateMemSet(ptr, value, num, align);
}

llvm::Value *Builder::allocate(llvm::Value *size) {
	assert(size->getType() == getInt32Ty());
	return CreateCall(mRuntime->allocatorFunction(), size);
}

void Builder::free(llvm::Value *ptr) {
	assert(ptr->getType()->isPointerTy());
	ptr = pointerToBytePointer(ptr);
	CreateCall(mRuntime->freeFunction(), ptr);
}

llvm::Value *Builder::pointerToBytePointer(llvm::Value *ptr) {
	assert(ptr->getType()->isPointerTy());
	if (ptr->getType() != llvm::IntegerType::get(context(), 8)->getPointerTo()) {
		ptr = CreateBitCast(ptr, llvm::IntegerType::get(context(), 8)->getPointerTo());
	}
	return ptr;
}

void Builder::setTemporaryVariableBasicBlock(llvm::BasicBlock *bb) {
	if (mTempVarBB != bb) {
		mTempVars.clear();
	}
	mTempVarBB = bb;
}

llvm::BasicBlock *Builder::temporaryVariableBasicBlock() const {
	return mTempVarBB;
}

llvm::AllocaInst *Builder::temporaryVariable(llvm::Type *type) {
	auto range = mTempVars.equal_range(type);
	for (std::multimap<llvm::Type*, TempAlloca>::iterator i = range.first; i != range.second; ++i) {
		TempAlloca &temp = i->second;
		if (!temp.mUsed) {
			temp.mUsed = true;
			return temp.mAlloca;
		}
	}

	llvm::IRBuilder<> builder(mTempVarBB);
	llvm::AllocaInst *alloc = builder.CreateAlloca(type, nullptr, "Temporary variable");
	TempAlloca temp;
	temp.mAlloca = alloc;
	temp.mUsed = true;
	mTempVars.insert(std::pair<llvm::Type*, TempAlloca>(type, temp));
	return alloc;
}

void Builder::removeTemporaryVariable(llvm::AllocaInst *alloc) {
	llvm::Type *elementType = alloc->getType()->getElementType();
	auto range = mTempVars.equal_range(elementType);
	for (std::multimap<llvm::Type*, TempAlloca>::iterator i = range.first; i != range.second; ++i) {
		TempAlloca &temp = i->second;
		if (temp.mAlloca == alloc) {
			assert(temp.mUsed);
			temp.mUsed = false;
			return;
		}
	}
	assert("Can't find temprorary" && 0);
}





