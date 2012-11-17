#include "intvaluetype.h"
#include "llvm.h"
#include "value.h"
IntValueType::IntValueType(llvm::Module *mod) {
	mType = llvm::IntegerType::get(mod->getContext(), 32);
}


ValueType::CastCostType IntValueType::castCost(ValueType *to) const {
	switch (to->type()) {
		case ValueType::Integer:
			return 0;
		case ValueType::Float:
			return 2;
		case ValueType::Short:
			return 3;
		case ValueType::Byte:
			return 4;
		case ValueType::String:
			return 10;
		default:
			return maxCastCost;
	}
}

Value IntValueType::cast(const Value &v) const {
	return Value();
}
