#include "nullvaluetype.h"
#include "abstractsyntaxtree.h"
#include "builder.h"

NullValueType::NullValueType(Runtime *runtime) :
	ValueType(runtime)
{
}

ValueType::CastCost NullValueType::castingCostToOtherValueType(const ValueType *to) const {
	return ccCastToSmaller;
}

Value NullValueType::cast(Builder *builder, const Value &v) const {
	return Value();
}

llvm::Constant *NullValueType::defaultValue() const {
	assert("Null doesn't have a type");
	return 0;
}

int NullValueType::size() const {
	assert("Null doesn't have a type");
	return 0;
}

Value NullValueType::generateOperation(Builder *builder, int opType, const Value &operand1, const Value &operand2, OperationFlags &operationFlags) const {
	assert(operand1.valueType() == this);
	switch (opType) {
		case ast::ExpressionNode::opEqual:
			if (operand2.isConstant()) {
				const ConstantValue &val = operand2.constant();
				switch (val.type()) {
					case ConstantValue::Byte:
					case ConstantValue::Short:
					case ConstantValue::Integer:
					case ConstantValue::Boolean:
						return Value(ConstantValue::equal(ConstantValue(0), val, operationFlags), mRuntime);
					case ConstantValue::Float:
						return Value(ConstantValue::equal(ConstantValue(0.0), val, operationFlags), mRuntime);
					case ConstantValue::String:
						return Value(ConstantValue::equal(ConstantValue(""), val, operationFlags), mRuntime);
					case ConstantValue::Null:
						return Value(ConstantValue(true), mRuntime);
					default:
						break;
				}
				break;
			}

			return operand2.valueType()->generateOperation(builder, opType, builder->defaultValue(operand2.valueType()), operand2, operationFlags);
		case ast::ExpressionNode::opNotEqual:
			if (operand2.isConstant()) {
				const ConstantValue &val = operand2.constant();
				switch (val.type()) {
					case ConstantValue::Byte:
					case ConstantValue::Short:
					case ConstantValue::Integer:
					case ConstantValue::Boolean:
						return Value(ConstantValue::notEqual(ConstantValue(0), val, operationFlags), mRuntime);
					case ConstantValue::Float:
						return Value(ConstantValue::notEqual(ConstantValue(0.0), val, operationFlags), mRuntime);
					case ConstantValue::String:
						return Value(ConstantValue::notEqual(ConstantValue(""), val, operationFlags), mRuntime);
					case ConstantValue::Null:
						return Value(ConstantValue(true), mRuntime);
					default:
						break;
				}
				break;
			}

			return operand2.valueType()->generateOperation(builder, opType, builder->defaultValue(operand2.valueType()), operand2, operationFlags);
	}
	operationFlags |= OperationFlag::NoSuchOperation;
	return Value();
}
