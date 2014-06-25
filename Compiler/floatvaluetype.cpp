#include "floatvaluetype.h"
#include "llvm.h"
#include "value.h"
#include "builder.h"

FloatValueType::FloatValueType(Runtime *runtime, llvm::Module *mod) :
	ValueType(runtime){
	mType = llvm::Type::getFloatTy(mod->getContext());
}

ValueType::CastCost FloatValueType::castingCostToOtherValueType(const ValueType *to) const {
	switch (to->basicType()) {
		case Float:
			return ccNoCost;
		case Boolean:
			return ccCastToBoolean;
		case Integer:
			return ccCastToSmaller;
		case String:
			return ccCastToString;
		case Short:
			return ccCastToSmaller;
		case Byte:
			return ccCastToSmaller;
		default:
			return ccNoCast;
	}
}

Value FloatValueType::cast(Builder *builder, const Value &v) const {
	return builder->toFloat(v);
}


llvm::Constant *FloatValueType::constant(float f) const {
	return llvm::ConstantFP::get(mType->getContext(), llvm::APFloat(f));
}

llvm::Constant *FloatValueType::defaultValue() const {
	return constant(0.0f);
}

Value FloatValueType::generateOperation(Builder *builder, int opType, const Value &operand1, const Value &operand2, OperationFlags &operationFlags) const {
	return generateBasicTypeOperation(builder, opType, operand1, operand2, operationFlags);
}

Value FloatValueType::generateOperation(Builder *builder, int opType, const Value &operand, OperationFlags &operationFlags) const {
	return generateBasicTypeOperation(builder, opType, operand, operationFlags);
}

