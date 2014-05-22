#include "floatvaluetype.h"
#include "llvm.h"
#include "value.h"
#include "builder.h"

FloatValueType::FloatValueType(Runtime *runtime, llvm::Module *mod) :
	ValueType(runtime){
	mType = llvm::Type::getFloatTy(mod->getContext());
}

ValueType::CastCostType FloatValueType::castingCostToOtherValueType(ValueType *to) const {
	switch (to->type()) {
		case Float:
			return 0;
		case Boolean:
			return 1;
		case Integer:
			return 5;
		case String:
			return 100;
		case Short:
			return 10;
		case Byte:
			return 10;
		default:
			return sMaxCastCost;
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

