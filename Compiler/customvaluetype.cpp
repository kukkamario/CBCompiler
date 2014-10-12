#include "customvaluetype.h"
#include "runtime.h"
#include "value.h"
#include "abstractsyntaxtree.h"
#include "nullvaluetype.h"
#include "builder.h"


CustomValueType::CustomValueType(const QString &name, llvm::Type *type, Runtime *r) :
	ValueType(r, type),
	mName(name) {
}

ValueType::CastCost CustomValueType::castingCostToOtherValueType(const ValueType *to) const {
	if (to == this) return ccNoCost;
	return ccNoCast;
}

Value CustomValueType::cast(Builder *, const Value &v) const {
	if (v.valueType() == this) return v;
	return Value();
}

llvm::Constant *CustomValueType::defaultValue() const {
	return llvm::Constant::getNullValue(mType);
}

int CustomValueType::size() const {
	return mRuntime->dataLayout().getTypeAllocSize(mType);
}

Value CustomValueType::generateOperation(Builder *builder, int opType, const Value &operand1, const Value &operand2, OperationFlags &operationFlags) const {
	if (operand2.valueType() == mRuntime->nullValueType()) {
		return generateOperation(builder, opType, operand1, builder->defaultValue(this), operationFlags);
	}
	switch (opType) {
		case ast::ExpressionNode::opAssign: {
			if (!operand1.isReference()) {
				operationFlags = OperationFlag::ReferenceRequired;
				return Value();
			}
			CastCost cc = operand2.valueType()->castingCostToOtherValueType(this);
			operationFlags = castCostOperationFlags(cc);
			if (operationFlagsContainFatalFlags(operationFlags)) return Value();
			Value op2 = this->cast(builder, operand2);
			builder->store(operand1, op2);
			return operand1;
		}
		case ast::ExpressionNode::opEqual: {
			if (operand1.valueType() == this && operand2.valueType() == this) {
				return builder->ptrEqual(builder->llvmValue(operand1), builder->llvmValue(operand2));
			}
			break;
		}
		case ast::ExpressionNode::opNotEqual: {
			if (operand1.valueType() == this && operand2.valueType() == this) {
				return builder->not_(builder->ptrEqual(builder->llvmValue(operand1), builder->llvmValue(operand2)));
			}
			break;
		}

	}
	operationFlags|= OperationFlag::NoSuchOperation;
	return Value();
}

