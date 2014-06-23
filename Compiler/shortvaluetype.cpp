#include "shortvaluetype.h"
#include "llvm.h"
#include "value.h"
#include "builder.h"
ShortValueType::ShortValueType(Runtime *r, llvm::Module *mod) :
	ValueType(r){
	mType = llvm::Type::getInt16Ty(mod->getContext());
}

ValueType::CastCost ShortValueType::castingCostToOtherValueType(const ValueType *to) const {
	switch (to->basicType()) {
		case ValueType::Short:
			return ccNoCost;
		case ValueType::Integer:
			return ccCastToBigger;
		case Boolean:
			return ccCastToBoolean;
		case ValueType::Float:
			return ccCastToBigger;
		case ValueType::Byte:
			return ccCastToSmaller;
		case ValueType::String:
			return ccCastToString;
		default:
			return ccNoCast;
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

Value ShortValueType::generateOperation(Builder *builder, int opType, const Value &operand1, const Value &operand2, OperationFlags &operationFlags) const {
	return generateBasicTypeOperation(builder, opType, operand1, operand2, operationFlags);
}

Value ShortValueType::generateOperation(Builder *builder, int opType, const Value &operand, OperationFlags &operationFlags) const {
	return generateBasicTypeOperation(builder, opType, operand, operationFlags);
}

