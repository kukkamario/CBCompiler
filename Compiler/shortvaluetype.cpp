#include "shortvaluetype.h"
#include "llvm.h"
#include "value.h"
ShortValueType::ShortValueType(llvm::Module *mod) {
	mType = llvm::Type::getInt16Ty(mod->getContext());
}

ValueType::CastCostType ShortValueType::castCost(ValueType *to) const {
	switch (to->type()) {
		case ValueType::Short:
			return 0;
		case ValueType::Integer:
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

Value ShortValueType::cast(const Value &v) const {
	return Value();
}
