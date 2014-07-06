#include "typepointervaluetype.h"
#include "typesymbol.h"
#include "value.h"
#include "runtime.h"
#include "builder.h"
#include "booleanvaluetype.h"
#include "nullvaluetype.h"
#include "abstractsyntaxtree.h"
#include "llvm.h"

TypePointerValueType::TypePointerValueType(Runtime *r, TypeSymbol *s):
	ValueType(r),
	mTypeSymbol(s){
	mType = 0;
}

QString TypePointerValueType::name() const {
	return mTypeSymbol->name();
}

ValueType::CastCost TypePointerValueType::castingCostToOtherValueType(const ValueType *to) const {
	if (to == this) return ccNoCost;
	if (to == mRuntime->typePointerCommonValueType()) return ccCastToBigger;
	return ccNoCast;
}

Value TypePointerValueType::cast(Builder *b, const Value &v) const {
	if (v.valueType() == this) return v;
	if (v.valueType() == mRuntime->nullValueType()) return b->defaultValue(this);
	assert("Invalid cast" && 0);
	return Value();
}

llvm::Constant *TypePointerValueType::defaultValue() const {
	return llvm::ConstantPointerNull::get(llvm::cast<llvm::PointerType>(mType));
}

int TypePointerValueType::size() const {
	return mRuntime->dataLayout().getPointerSize();
}

Value TypePointerValueType::generateOperation(Builder *builder, int opType, const Value &operand1, const Value &operand2, OperationFlags &operationFlags) const {
	assert(operand1.valueType() == this);
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
			if (operand1.valueType() == operand2.valueType()) {
				return Value(mRuntime->booleanValueType(), builder->irBuilder().CreateICmpEQ(builder->llvmValue(operand1), builder->llvmValue(operand2)));
			} else if (operand2.valueType() == mRuntime->typePointerCommonValueType()) {
				return Value(mRuntime->booleanValueType(),
							 builder->irBuilder().CreateICmpEQ(
								 builder->bitcast(mRuntime->typePointerCommonValueType()->llvmType(), builder->llvmValue(operand1)),
								 builder->llvmValue(operand2)), false);
			} else if (operand2.valueType() == mRuntime->nullValueType()) {
				return Value(mRuntime->booleanValueType(),
							 builder->irBuilder().CreateIsNull(builder->llvmValue(operand1)),
							 false);
			}
			break;
		}
		case ast::ExpressionNode::opNotEqual: {
			if (operand1.valueType() == operand2.valueType()) {
				return Value(mRuntime->booleanValueType(), builder->irBuilder().CreateICmpNE(builder->llvmValue(operand1), builder->llvmValue(operand2)));
			} else if (operand2.valueType() == mRuntime->typePointerCommonValueType()) {
				return Value(mRuntime->booleanValueType(),
							 builder->irBuilder().CreateICmpNE(
								 builder->bitcast(mRuntime->typePointerCommonValueType()->llvmType(), builder->llvmValue(operand1)),
								 builder->llvmValue(operand2)), false);
			} else if (operand2.valueType() == mRuntime->nullValueType()) {
				return Value(mRuntime->booleanValueType(),
							 builder->irBuilder().CreateIsNotNull(builder->llvmValue(operand1)),
							 false);
			}
			break;
		}
	}
	operationFlags |= OperationFlag::NoSuchOperation;
	return Value();
}

Value TypePointerValueType::member(Builder *builder, const Value &a, const QString &memberName) const {
	assert(a.valueType() == this);
	if (!mTypeSymbol->hasField(memberName)) return Value();
	return builder->typePointerFieldReference(a, memberName);
}

ValueType *TypePointerValueType::memberType(const QString &memberName) const {
	if (!mTypeSymbol->hasField(memberName)) return 0;
	const TypeField &field = mTypeSymbol->field(memberName);
	return field.valueType();
}


ValueType::CastCost TypePointerCommonValueType::castingCostToOtherValueType(const ValueType *to) const {
	if (to == this) return ccNoCost;
	return ccNoCast;
}

Value TypePointerCommonValueType::cast(Builder *builder, const Value &v) const {
	if (v.valueType() == this) {
		return v;
	}
	if (v.valueType()->isTypePointer()) {
		if (v.isConstant()) {
			return Value(const_cast<TypePointerCommonValueType*>(this), defaultValue(), false);
		}
		if (v.isReference()) {
			return Value(const_cast<TypePointerCommonValueType*>(this), builder->bitcast(mType->getPointerTo(), v.value()), true);
		}
		else {
			return Value(const_cast<TypePointerCommonValueType*>(this), builder->bitcast(mType, v.value()), false);
		}

	}
	assert("Invalid cast" && 0);
	return Value();
}


llvm::Constant *TypePointerCommonValueType::defaultValue() const {
	return llvm::ConstantPointerNull::get(static_cast<llvm::PointerType*>(mType));
}

int TypePointerCommonValueType::size() const {
	return mRuntime->dataLayout().getPointerSize();
}

Value TypePointerCommonValueType::generateOperation(Builder *builder, int opType, const Value &operand1, const Value &operand2, OperationFlags &operationFlags) const {
	operationFlags |= OperationFlag::NoSuchOperation;
	return Value();
}

