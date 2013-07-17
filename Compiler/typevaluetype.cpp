#include "typevaluetype.h"
#include "value.h"
#include "runtime.h"

TypeValueType::TypeValueType(Runtime *r, llvm::Type *type) :
	ValueType(r) {
	mType = type;
}

ValueType::CastCostType TypeValueType::castingCostToOtherValueType(ValueType *to) const {
	if (to == this) return 0;
	return sMaxCastCost;
}

Value TypeValueType::cast(Builder *, const Value &v) const {
	if (v.valueType() == this) return v;
	return Value();
}

llvm::Constant *TypeValueType::defaultValue() const {
	return llvm::ConstantPointerNull::get(static_cast<llvm::PointerType*>(mType));
}


int TypeValueType::size() const {
	return mRuntime->dataLayout().getPointerSize();
}

