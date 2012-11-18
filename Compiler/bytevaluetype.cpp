#include "bytevaluetype.h"
#include "llvm.h"
#include "value.h"


ByteValueType::ByteValueType(llvm::Module *mod) {
	mType = llvm::Type::getInt8Ty(mod->getContext());
}

ValueType::CastCostType ByteValueType::castCost(ValueType *to) const {
	switch (to->type()) {
		case ValueType::Byte:
			return 0;
		case ValueType::Short:
			return 1;
		case ValueType::Integer:
			return 1;
		case ValueType::Float:
			return 2;
		case ValueType::String:
			return 10;
		default:
			return maxCastCost;
	}
}

Value ByteValueType::cast(const Value &v) const {
	return Value();
}
