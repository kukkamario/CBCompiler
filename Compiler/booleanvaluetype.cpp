#include "booleanvaluetype.h"
#include "value.h"
#include "llvm.h"
#include "builder.h"
#include "abstractsyntaxtree.h"
#include "stringvaluetype.h"

BooleanValueType::BooleanValueType(Runtime *r, llvm::Module *mod) :
	ValueType(r){
	mType = llvm::Type::getInt1Ty(mod->getContext());
}

ValueType::CastCost BooleanValueType::castingCostToOtherValueType(const ValueType *to) const {
	if (to == this || to->basicType() == ValueType::Integer) return ccNoCost;
	if (to->isNumber()) {
		return ccCastToBigger;
	}
	if (to == mRuntime->stringValueType()) {
		return ccCastToString;
	}
	return ccNoCast;
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


Value BooleanValueType::generateOperation(Builder *builder, int opType, const Value &operand1, const Value &operand2, OperationFlags &operationFlags) const {
	return generateBasicTypeOperation(builder, opType, operand1, operand2, operationFlags);
}

Value BooleanValueType::generateOperation(Builder *builder, int opType, const Value &operand, OperationFlags &operationFlags) const {
	return generateBasicTypeOperation(builder, opType, operand, operationFlags);
}
