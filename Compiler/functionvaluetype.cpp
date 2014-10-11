#include "functionvaluetype.h"
#include "liststringjoin.h"
#include "value.h"
#include "function.h"
#include "builder.h"
#include "nullvaluetype.h"
#include "abstractsyntaxtree.h"
#include "booleanvaluetype.h"


FunctionValueType::FunctionValueType(Runtime *r, ValueType *retType, const QList<ValueType *> &paramList) :
	ValueType(r),
	mReturnType(retType),
	mParamTypes(paramList) {

	std::vector<llvm::Type*> params;
	params.reserve(mParamTypes.size());
	for (ValueType *vt : mParamTypes) {
		params.push_back(vt->llvmType());
	}
	if (mReturnType == 0) {
		mType = llvm::FunctionType::get(llvm::Type::getVoidTy(r->module()->getContext()), params, false)->getPointerTo();
	} else {
		mType = llvm::FunctionType::get(mReturnType->llvmType(), params, false)->getPointerTo();
	}
}

QString FunctionValueType::name() const {
	if (mReturnType) {
		return QStringLiteral("Function (") % listStringJoin(mParamTypes, [](ValueType *valTy) {
			return valTy->name();
		}) % QStringLiteral(") As ") % mReturnType->name();
	}
	else {
		return QStringLiteral("Command (") % listStringJoin(mParamTypes, [](ValueType *valTy) {
			return valTy->name();
		}) % QStringLiteral(")");
	}
}


llvm::Constant *FunctionValueType::defaultValue() const {
	return llvm::Constant::getNullValue(mType);
}


int FunctionValueType::size() const {
	return mRuntime->dataLayout().getTypeStoreSize(mType);
}

ValueType::CastCost FunctionValueType::castingCostToOtherValueType(const ValueType *to) const {
	if (to != this) return ValueType::ccNoCast;
	return ValueType::ccNoCost;
}

Value FunctionValueType::cast(Builder *, const Value &v) const {
	if (v.valueType() != this) return Value();
	return v;
}

Value FunctionValueType::generateLoad(Builder *builder, const Value &var) const {
	assert(var.valueType() == this);
	assert(var.isReference());
	return Value(const_cast<FunctionValueType*>(this), builder->irBuilder().CreateLoad(var.value()), false);
}

Value FunctionValueType::generateOperation(Builder *builder, int opType, const Value &operand1, const Value &operand2, OperationFlags &operationFlags) const {
	assert(operand1.valueType() == this);
	Value op2 = operand2;
	if (op2.valueType() == mRuntime->nullValueType()) {
		op2 = Value(const_cast<FunctionValueType*>(this), defaultValue());
	}
	if (operand1.valueType() != operand2.valueType()) {
		operationFlags |= OperationFlag::NoSuchOperation;
		return Value();
	}
	switch (opType) {
		case ast::ExpressionNode::opAssign:
			if (!operand1.isReference()) {
				operationFlags |= OperationFlag::ReferenceRequired;
				return Value();
			}
			builder->irBuilder().CreateStore(builder->llvmValue(operand2), operand1.value());
			return operand1;
		case ast::ExpressionNode::opEqual:
			return Value(mRuntime->booleanValueType(), builder->irBuilder().CreateICmpEQ(builder->llvmValue(operand1), builder->llvmValue(operand2)), false);
		case ast::ExpressionNode::opNotEqual:
			return Value(mRuntime->booleanValueType(), builder->irBuilder().CreateICmpNE(builder->llvmValue(operand1), builder->llvmValue(operand2)), false);
	}
	operationFlags |= OperationFlag::NoSuchOperation;
	return Value();
}


