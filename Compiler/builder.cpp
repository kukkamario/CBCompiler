#include "builder.h"
#include "stringvaluetype.h"
#include "stringpool.h"
#include "intvaluetype.h"
#include "shortvaluetype.h"
#include "floatvaluetype.h"
#include "bytevaluetype.h"
#include "typepointervaluetype.h"
#include "booleanvaluetype.h"

Builder::Builder(llvm::LLVMContext &context) :
	mRuntime(0),
	mStringPool(0),
	mIRBuilder(context){
}

void Builder::setRuntime(Runtime *r) {
	mRuntime = r;
}

void Builder::setStringPool(StringPool *s) {
	mStringPool = s;
}

void Builder::setInsertPoint(llvm::BasicBlock *basicBlock) {
	mIRBuilder.SetInsertPoint(basicBlock);
}

Value Builder::toInt(const Value &v) {
	if (v.valueType()->type() == ValueType::Integer) return v;
	if (v.isConstant()) {
		return Value(ConstantValue(v.constant().toInt()), mRuntime);
	}
	assert(v.value());
	switch (v.valueType()->type()) {
		case ValueType::Float: {
			llvm::Value *r = mIRBuilder.CreateAdd(v.value(), llvm::ConstantFP::get(mIRBuilder.getFloatTy(), 0.5));
			return Value(mRuntime->intValueType(), mIRBuilder.CreateFPToSI(r,mIRBuilder.getInt32Ty()));
		}
		case ValueType::Boolean:
		case ValueType::Byte:
		case ValueType::Short: {
			llvm::Value *r = mIRBuilder.CreateCast(llvm::CastInst::ZExt, v.value(), mIRBuilder.getInt32Ty());
			return Value(mRuntime->intValueType(), r);
		}
		case ValueType::String: {
			llvm::Value *i = mRuntime->stringValueType()->stringToIntCast(&mIRBuilder, v.value());
			return Value(mRuntime->intValueType(), i);
		}
	}
	assert(0);
	return Value();
}

Value Builder::toFloat(const Value &v) {
	if (v.valueType()->type() == ValueType::Float) return v;
	if (v.isConstant()) {
		return Value(ConstantValue(v.constant().toFloat()), mRuntime);
	}
	switch(v.valueType()->type()) {
		case ValueType::Integer:
			return Value(mRuntime->floatValueType(),mIRBuilder.CreateSIToFP(v.value(), mIRBuilder.getFloatTy()));
		case ValueType::Boolean:
		case ValueType::Short:
		case ValueType::Byte:
			return Value(mRuntime->floatValueType(),mIRBuilder.CreateUIToFP(v.value(), mIRBuilder.getFloatTy()));
		case ValueType::String: {
			llvm::Value *val = mRuntime->stringValueType()->stringToFloatCast(&mIRBuilder, v.value());
			return Value(mRuntime->floatValueType(), val);
		}

	}
	assert(0);
	return Value();
}

Value Builder::toString(const Value &v) {
	assert(0);
	return Value();
}

Value Builder::toShort(const Value &v) {
	assert(0);
	return Value();
}

Value Builder::toByte(const Value &v) {
	assert(0);
	return Value();
}

Value Builder::toBoolean(const Value &v) {
	if (v.valueType()->type() == ValueType::Boolean) {
		return v;
	}
	if (v.isConstant()) {
		return Value(ConstantValue(v.constant().toBool()), mRuntime);
	}
	switch(v.valueType()->type()) {
		case ValueType::Integer:
			return Value(mRuntime->booleanValueType(), mIRBuilder.CreateICmpNE(v.value(), mIRBuilder.getInt32(0)));
		case ValueType::Short:
			return Value(mRuntime->booleanValueType(), mIRBuilder.CreateICmpNE(v.value(), mIRBuilder.getInt16(0)));
		case ValueType::Byte:
			return Value(mRuntime->booleanValueType(), mIRBuilder.CreateICmpNE(v.value(), mIRBuilder.getInt8(0)));
		case ValueType::Float:
			return Value(mRuntime->booleanValueType(), mIRBuilder.CreateFCmpONE(v.value(), llvm::ConstantFP::get(mIRBuilder.getFloatTy(), 0.0)));
		case ValueType::String:
			return Value(mRuntime->booleanValueType(), mIRBuilder.CreateIsNotNull(v.value()));
	}

	return Value();
}

Value Builder::toValueType(ValueType *to, const Value &v) {
	return to->cast(this, v);
}

llvm::Value *Builder::llvmValue(const Value &v) {
	if (v.isConstant()) {
		switch(v.valueType()->type()) {
			case ValueType::Integer:
				return llvmValue(v.constant().toInt());
			case ValueType::Float:
				return llvmValue(v.constant().toFloat());
			case ValueType::Short:
				return llvmValue(v.constant().toShort());
			case ValueType::Byte:
				return llvmValue(v.constant().toByte());
			case ValueType::Boolean:
				return llvmValue(v.constant().toBool());
			case ValueType::String: {
				return llvmValue(v.constant().toString());
			}
		}
	}

	assert(v.value());
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

	llvm::Value *v = mStringPool->globalString(&mIRBuilder, s);
	return mRuntime->stringValueType()->constructString(&mIRBuilder, s);
}

llvm::Value *Builder::llvmValue(bool t) {
	return mIRBuilder.getInt1(t);
}


Value Builder::call(Function *func, const QList<Value> &params) {
	return func->call(this, params);
}

void Builder::branch(llvm::BasicBlock *dest) {
	mIRBuilder.CreateBr(dest);
}

void Builder::branch(const Value &cond, llvm::BasicBlock *ifTrue, llvm::BasicBlock *ifFalse) {
	mIRBuilder.CreateCondBr(llvmValue(toBoolean(cond)), ifTrue, ifFalse);
}

void Builder::construct(VariableSymbol *var) {
	llvm::Value *allocaInst = mIRBuilder.CreateAlloca(var->valueType()->llvmType());
	var->setAlloca(allocaInst);
	switch(var->valueType()->type()) {
		case ValueType::Integer:
			mIRBuilder.CreateStore(llvmValue(0), allocaInst); break;
		case ValueType::Float:
			mIRBuilder.CreateStore(llvmValue(0.0f), allocaInst); break;
		case ValueType::Short:
			mIRBuilder.CreateStore(llvmValue((uint16_t)0), allocaInst); break;
		case ValueType::Byte:
			mIRBuilder.CreateStore(llvmValue((uint8_t)0), allocaInst); break;
		case ValueType::String:
			mIRBuilder.CreateStore(llvmValue(QString()), allocaInst); break;
	}

}

void Builder::store(VariableSymbol *var, const Value &v) {
	llvm::Value *val = llvmValue(var->valueType()->cast(this, v));
	mIRBuilder.CreateStore(val, var->alloca_());
}

Value Builder::load(const VariableSymbol *var) {
	return Value(var->valueType(), mIRBuilder.CreateLoad(var->alloca_(), false));
}

void Builder::destruct(VariableSymbol *var) {
	if (var->valueType()->type() == ValueType::String) {
		llvm::Value *str = mIRBuilder.CreateLoad(var->alloca_(), false);
		mRuntime->stringValueType()->destructString(&mIRBuilder, str);
	}
}

void Builder::destruct(const Value &a) {
	if (a.isConstant() || a.valueType()->type() != ValueType::String) return;
	assert(a.value());
	mRuntime->stringValueType()->destructString(&mIRBuilder, a.value());
}

Value Builder::not_(const Value &a) {
	if (a.isConstant()) {
		return Value(ConstantValue::not_(a.constant()), mRuntime);
	}

	return Value(mRuntime->booleanValueType(), mIRBuilder.CreateNot(llvmValue(toBoolean(a))));
}

Value Builder::minus(const Value &a) {
	if (a.isConstant()) {
		return Value(ConstantValue::minus(a.constant()), mRuntime);
	}

	return Value(a.valueType(), mIRBuilder.CreateNeg(llvmValue(a)));
}

Value Builder::plus(const Value &a) {
	assert(0); return Value();
}

Value Builder::add(const Value &a, const Value &b) {
	if (a.isConstant() && b.isConstant()) {
		return Value(ConstantValue::add(a.constant(), b.constant()), mRuntime);
	}

	//TODO: finish this
	llvm::Value *result;
	switch(a.valueType()->type()) {
		case ValueType::Integer:
			switch(b.valueType()->type()) {
				case ValueType::Integer:
					result = mIRBuilder.CreateAdd(llvmValue(a), llvmValue(b));
					return Value(mRuntime->intValueType(), result);
				case ValueType::Float:
					result = mIRBuilder.CreateAdd(llvmValue(toFloat(a)), llvmValue(b));
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
			}
		case ValueType::Float:
			switch(b.valueType()->type()) {
				case ValueType::Integer:
				case ValueType::Short:
				case ValueType::Byte:
					result = mIRBuilder.CreateAdd(llvmValue(a), llvmValue(toFloat(b)));
					return Value(mRuntime->floatValueType(), result);
				case ValueType::Float:
					result = mIRBuilder.CreateAdd(llvmValue(a), llvmValue(b));
					return Value(mRuntime->floatValueType(), result);
				case ValueType::String:
					Value as = toString(a);
					result = mRuntime->stringValueType()->stringAddition(&mIRBuilder, llvmValue(as), llvmValue(b));
					destruct(as);
					return Value(mRuntime->stringValueType(), result);
			}
		case ValueType::Short:
			switch(b.valueType()->type()) {
				case ValueType::Integer:
					result = mIRBuilder.CreateAdd(llvmValue(toInt(a)), llvmValue(b));
					return Value(mRuntime->intValueType(), result);
				case ValueType::Float:
					result = mIRBuilder.CreateAdd(llvmValue(toFloat(a)), llvmValue(b));
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
			}
	}
	assert(0);
	return Value();
}

Value Builder::subtract(const Value &a, const Value &b) {
	assert(0); return Value();
}

Value Builder::multiply(const Value &a, const Value &b) {
	assert(0); return Value();
}

Value Builder::divide(const Value &a, const Value &b) {
	assert(0); return Value();
}

Value Builder::mod(const Value &a, const Value &b){
	assert(0); return Value();
}

Value Builder::shl(const Value &a, const Value &b){
	assert(0); return Value();
}

Value Builder::shr(const Value &a, const Value &b){
	assert(0); return Value();
}

Value Builder::sar(const Value &a, const Value &b){
	assert(0); return Value();
}

Value Builder::and_(const Value &a, const Value &b) {
	if (a.isConstant() && b.isConstant()) {
		return Value(ConstantValue::and_(a.constant(),b.constant()), mRuntime);
	}
	return Value(mRuntime->booleanValueType(), mIRBuilder.CreateAnd(llvmValue(toBoolean(a)), llvmValue(toBoolean(b))));
}

Value Builder::or_(const Value &a, const Value &b) {
	if (a.isConstant() && b.isConstant()) {
		return Value(ConstantValue::or_(a.constant(),b.constant()), mRuntime);
	}
	return Value(mRuntime->booleanValueType(), mIRBuilder.CreateOr(llvmValue(toBoolean(a)), llvmValue(toBoolean(b))));
}

Value Builder::xor_(const Value &a, const Value &b) {
	if (a.isConstant() && b.isConstant()) {
		return Value(ConstantValue::xor_(a.constant(),b.constant()), mRuntime);
	}
	return Value(mRuntime->booleanValueType(), mIRBuilder.CreateXor(llvmValue(toBoolean(a)), llvmValue(toBoolean(b))));
}

Value Builder::power(const Value &a, const Value &b) {
	assert(0); return Value();
}

Value Builder::less(const Value &a, const Value &b) {
	if (a.isConstant() && b.isConstant()) {
		return Value(ConstantValue::less(a.constant(), b.constant()), mRuntime);
	}

	switch(a.valueType()->type()) {
		case ValueType::Integer:
			switch(a.valueType()->type()) {

			}
	}

	assert(0); return Value();
}

Value Builder::lessEqual(const Value &a, const Value &b) {
	assert(0); return Value();
}

Value Builder::greater(const Value &a, const Value &b) {
	assert(0); return Value();
}

Value Builder::greaterEqual(const Value &a, const Value &b) {
	assert(0); return Value();
}

Value Builder::equal(const Value &a, const Value &b) {
	assert(0); return Value();
}

Value Builder::notEqual(const Value &a, const Value &b) {
	assert(0); return Value();
}

