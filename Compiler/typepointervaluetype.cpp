#include "typepointervaluetype.h"
#include "typesymbol.h"
#include "value.h"
#include "runtime.h"
#include "builder.h"

TypePointerValueType::TypePointerValueType(Runtime *r, TypeSymbol *s):
	ValueType(r),
	mTypeSymbol(s){
	mType = 0;
}

QString TypePointerValueType::name() const {
	return mTypeSymbol->name();
}

ValueType::CastCostType TypePointerValueType::castingCostToOtherValueType(ValueType *to) const {
	if (to == this) return 0;
	if (to->type() == ValueType::TypePointerCommon) return 1;
	return sMaxCastCost;
}

Value TypePointerValueType::cast(Builder *builder, const Value &v) const {
	if (v.valueType() == this) return v;
	assert("Invalid cast" && 0);
	return Value();
}

llvm::Constant *TypePointerValueType::defaultValue() const {
	return llvm::ConstantPointerNull::get(llvm::cast<llvm::PointerType>(mType));
}

int TypePointerValueType::size() const {
	return mRuntime->dataLayout().getPointerSize();
}


ValueType::CastCostType TypePointerCommonValueType::castingCostToOtherValueType(ValueType *to) const {
	if (to == this) return 0;
	return sMaxCastCost;
}

Value TypePointerCommonValueType::cast(Builder *builder, const Value &v) const {
	if (v.valueType()->type() == ValueType::TypePointerCommon) {
		return v;
	}
	if (v.valueType()->type() == ValueType::TypePointer) {
		if (v.isConstant()) {
			return Value(const_cast<TypePointerCommonValueType*>(this), defaultValue());
		}
		return Value(const_cast<TypePointerCommonValueType*>(this), builder->bitcast(mType, v.value()));
	}
	assert("Invalid cast" && 0);
	return Value();
}


llvm::Constant *TypePointerCommonValueType::defaultValue() const {
	return llvm::ConstantPointerNull::get(static_cast<llvm::PointerType*>(mType));
}

int TypePointerCommonValueType::size() const {
	return mRuntime->dataLayout().getPointerSize();
}

