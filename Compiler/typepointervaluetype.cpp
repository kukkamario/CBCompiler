#include "typepointervaluetype.h"
#include "typesymbol.h"
#include "value.h"
#include "runtime.h"

TypePointerValueType::TypePointerValueType(Runtime *r, TypeSymbol *s):
	ValueType(r),
	mTypeSymbol(s){
	mType = s->llvmMemberType();
}

QString TypePointerValueType::name() const {
	return mTypeSymbol->name();
}

ValueType::CastCostType TypePointerValueType::castingCostToOtherValueType(ValueType *to) const {
	if (to == this) return 0;
	return maxCastCost;
}

Value TypePointerValueType::cast(Builder *builder, const Value &v) const {
	if (v.valueType() != this) {
		return Value();
	}
	return v;
}

llvm::Constant *TypePointerValueType::defaultValue() const {
	return llvm::ConstantPointerNull::get(mType->getPointerTo());
}

int TypePointerValueType::size() const {
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


Value NullTypePointerValueType::cast(Builder *builder, const Value &v) const {
	return Value();
}

llvm::Constant *NullTypePointerValueType::defaultValue() const {
	return llvm::ConstantPointerNull::get(llvm::Type::getVoidTy(mRuntime->module()->getContext())->getPointerTo());
}

int NullTypePointerValueType::size() const {
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
