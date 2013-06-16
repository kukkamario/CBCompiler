#include "shortvaluetype.h"
#include "llvm.h"
#include "value.h"
#include "builder.h"
ShortValueType::ShortValueType(Runtime *r, llvm::Module *mod) :
	ValueType(r){
	mType = llvm::Type::getInt16Ty(mod->getContext());
}

ValueType::CastCostType ShortValueType::castCost(ValueType *to) const {
	switch (to->type()) {
		case ValueType::Short:
			return 0;
		case ValueType::Integer:
			return 1;
		case Boolean:
			return 1;
		case ValueType::Float:
			return 2;
		case ValueType::Byte:
			return 4;
		case ValueType::String:
			return 10;
		default:
			return maxCastCost;
	}
}

Value ShortValueType::cast(Builder *builder, const Value &v) const {
	return builder->toShort(v);
}

llvm::Constant *ShortValueType::constant(quint16 i) const {
	return llvm::ConstantInt::get(mType, llvm::APInt(16, i, false));
}

llvm::Constant *ShortValueType::defaultValue() const {
	return constant(0);
}
