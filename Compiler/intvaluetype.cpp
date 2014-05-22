#include "intvaluetype.h"
#include "llvm.h"
#include "value.h"
#include "runtime.h"
#include "builder.h"
IntValueType::IntValueType(Runtime *r, llvm::Module *mod) :
	ValueType(r){
	mType = llvm::IntegerType::get(mod->getContext(), 32);
}



ValueType::CastCost IntValueType::castingCostToOtherValueType(ValueType *to) const {
	switch (to->type()) {
		case ValueType::Integer:
			return 0;
		case ValueType::Boolean:
			return 1;
		case ValueType::Float:
			return 2;
		case ValueType::Short:
			return 3;
		case ValueType::Byte:
			return 4;
		case ValueType::String:
			return 100;
		default:
			return sMaxCastCost;
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

Value FloatValueType::generateOperation(Builder *builder, int opType, const Value &operand1, const Value &operand2, OperationFlags &operationFlags) const {
	return generateBasicTypeOperation(builder, opType, operand1, operand2, operationFlags);
}

Value FloatValueType::generateOperation(Builder *builder, int opType, const Value &operand, OperationFlags &operationFlags) const {
	return generateBasicTypeOperation(builder, opType, operand, operationFlags);
}

