#include "intvaluetype.h"
#include "llvm.h"
#include "value.h"
#include "runtime.h"
#include "builder.h"
IntValueType::IntValueType(Runtime *r, llvm::Module *mod) :
	ValueType(r){
	mType = llvm::IntegerType::get(mod->getContext(), 32);
}



ValueType::CastCost IntValueType::castingCostToOtherValueType(const ValueType *to) const {
	switch (to->basicType()) {
		case ValueType::Integer:
			return ccNoCost;
		case ValueType::Boolean:
			return ccCastToBoolean;
		case ValueType::Float:
			return ccCastToSmaller;
		case ValueType::Short:
			return ccCastToSmaller;
		case ValueType::Byte:
			return ccCastToSmaller;
		case ValueType::String:
			return ccCastToString;
		default:
			return ccNoCast;
	}
}

Value IntValueType::cast(Builder *builder, const Value &v) const {
	return builder->toInt(v);
}

llvm::Constant *IntValueType::constant(int v) const {
	return llvm::ConstantInt::get(mType, llvm::APInt(32, v, true));
}

llvm::Constant *IntValueType::defaultValue() const {
	return constant(0);
}

Value IntValueType::generateOperation(Builder *builder, int opType, const Value &operand1, const Value &operand2, OperationFlags &operationFlags) const {
	return generateBasicTypeOperation(builder, opType, operand1, operand2, operationFlags);
}

Value IntValueType::generateOperation(Builder *builder, int opType, const Value &operand, OperationFlags &operationFlags) const {
	return generateBasicTypeOperation(builder, opType, operand, operationFlags);
}

