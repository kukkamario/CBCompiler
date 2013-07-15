#include "booleanvaluetype.h"
#include "value.h"
#include "llvm.h"
#include "builder.h"
BooleanValueType::BooleanValueType(Runtime *r, llvm::Module *mod) :
	ValueType(r){
	mType = llvm::Type::getInt1Ty(mod->getContext());
}

ValueType::CastCostType BooleanValueType::castingCostToOtherValueType(ValueType *to) const {
	switch (to->type()) {
		case ValueType::Boolean:
			return 0;
		case ValueType::Byte:
			return 3;
		case ValueType::Short:
			return 3;
		case ValueType::Integer:
			return 3;
		case ValueType::Float:
			return 3;
		case ValueType::String:
			return 5;
		default:
			return sMaxCastCost;
	}
}

Value BooleanValueType::cast(Builder *builder, const Value &v) const {
	return builder->toBoolean(v);
}

llvm::Constant *BooleanValueType::constant(bool t) const {
	return llvm::ConstantInt::get(mType, llvm::APInt(1, t ? 1 : 0));
}

llvm::Constant *BooleanValueType::defaultValue() const {
	return constant(false);
}
