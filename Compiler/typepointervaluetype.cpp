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

	if (v.valueType()->type() == ValueType::TypePointerCommon) {
		if (v.isConstant()) {
			return Value(const_cast<TypePointerValueType*>(this), defaultValue());
		}
		return Value(const_cast<TypePointerValueType*>(this), builder->bitcast(mType, v.value()));
	}
	return Value();
}

llvm::Constant *TypePointerValueType::defaultValue() const {
	return llvm::ConstantPointerNull::get(mType->getPointerTo());
}

int TypePointerValueType::size() const {
	return mRuntime->dataLayout().getPointerSize();
}


ValueType::CastCostType TypePointerCommonValueType::castingCostToOtherValueType(ValueType *to) const {
	if (to == this) return 0;
	return sMaxCastCost;
}

Value TypePointerCommonValueType::cast(Builder *, const Value &v) const {
	if (v.valueType()->type() == ValueType::TypePointerCommon) {
		return v;
	}
	return Value();
}


llvm::Constant *TypePointerCommonValueType::defaultValue() const {
	return llvm::ConstantPointerNull::get(static_cast<llvm::PointerType*>(mType));
}

int TypePointerCommonValueType::size() const {
	return mRuntime->dataLayout().getPointerSize();
}

