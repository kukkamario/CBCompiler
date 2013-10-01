#include "customvaluetype.h"
#include "runtime.h"
#include "value.h"
CustomValueType::CustomValueType(const QString &name, llvm::Type *type, Runtime *r) :
	ValueType(r, type),
	mName(name) {
}

ValueType::CastCostType CustomValueType::castingCostToOtherValueType(ValueType *to) const {
	if (to == this) return 0;
	return sMaxCastCost;
}

Value CustomValueType::cast(Builder *, const Value &v) const {
	if (v.valueType() == this) return v;
	return Value();
}

llvm::Constant *CustomValueType::defaultValue() const {
	return llvm::Constant::getNullValue(mType);
}

int CustomValueType::size() const {
	return mRuntime->dataLayout().getTypeAllocSize(mType);
}
