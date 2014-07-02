#include "structvaluetype.h"
#include "builder.h"
#include "operationflags.h"
#include "booleanvaluetype.h"
#include "abstractsyntaxtree.h"
#include "genericstructvaluetype.h"

StructValueType::StructValueType(const QString &name, const CodePoint &cp, Runtime *runtime) :
	ValueType(runtime),
	mName(name),
	mCodePoint(cp),
	mStructType(0) {

}

StructValueType::StructValueType(const QString &name, const CodePoint &cp, const QList<StructField> &fields, Runtime *runtime) :
	ValueType(runtime),
	mName(name),
	mCodePoint(cp),
	mStructType(0) {
	setFields(fields);
}

StructValueType::~StructValueType() {

}

QString StructValueType::name() const {
	return mName;
}

ValueType::CastCost StructValueType::castingCostToOtherValueType(const ValueType *to) const {
	if (to == this) return ccNoCost;
	if (to == mRuntime->genericStructValueType()) return ccCastToBigger;
	return ccNoCast;
}

Value StructValueType::cast(Builder *builder, const Value &v) const {
	if (v.valueType() == this) return v;
	return Value();
}

llvm::Constant *StructValueType::defaultValue() const {
	return llvm::ConstantPointerNull::get(llvm::cast<llvm::PointerType>(mType));
}

int StructValueType::size() const {
	return mRuntime->dataLayout().getPointerSize();
}

void StructValueType::setFields(const QList<StructField> &fields) {
	mFields = fields;
	for (QList<StructField>::Iterator i = mFields.begin(); i != mFields.end(); ++i) {
		mFieldSearch[i->name()] = i;
	}
}

void StructValueType::createOpaqueType(Builder *builder) {
	mStructType = llvm::StructType::create(builder->context(), mName.toStdString());
	mType = mStructType->getPointerTo();
}

void StructValueType::generateLLVMType() {
	std::vector<llvm::Type*> body;
	body.reserve(mFields.size());

	for (const StructField &f : mFields) {
		body.push_back(f.valueType()->llvmType());
	}

	mStructType->setBody(body);
}

Value StructValueType::generateOperation(Builder *builder, int opType, const Value &operand1, const Value &operand2, OperationFlags &operationFlags) const {
	if (operand1.valueType() == this && operand2.valueType() == this) {
		switch (opType) {
			case ast::ExpressionNode::opAssign: {
				if (!operand1.isReference()) {
					operationFlags |= OperationFlag::ReferenceRequired;
					return Value();
				}
				builder->store(operand1, operand2);
				return operand1;
			}
			case ast::ExpressionNode::opEqual: {
				return Value(mRuntime->booleanValueType(), builder->irBuilder().CreateICmpEQ(builder->llvmValue(operand1), builder->llvmValue(operand2)));
			}
			case ast::ExpressionNode::opNotEqual: {
				return Value(mRuntime->booleanValueType(), builder->irBuilder().CreateICmpNE(builder->llvmValue(operand1), builder->llvmValue(operand2)));
			}
		}
	}
	operationFlags |= OperationFlag::NoSuchOperation;
	return Value();
}

Value StructValueType::member(Builder *builder, const Value &a, const QString &memberName) const {
	assert(a.valueType() == this);
	QList<StructField>::ConstIterator fieldI = mFieldSearch.value(memberName, mFields.end());
	if (fieldI == mFields.end()) {
		return Value();
	}
	int fieldIndex = fieldI - mFields.begin();

	llvm::Value *structPtr = builder->llvmValue(a);
	return Value(fieldI->valueType(), builder->irBuilder().CreateStructGEP(structPtr, fieldIndex), true);
}

ValueType *StructValueType::memberType(const QString &memberName) const {
	QList<StructField>::ConstIterator fieldI = mFieldSearch.value(memberName, mFields.end());
	if (fieldI != mFields.end()) {
		return fieldI->valueType();
	}
	return 0;
}


StructField::StructField(const QString &name, ValueType *valueType, const CodePoint &cp) :
	mName(name),
	mValueType(valueType),
	mCodePoint(cp) {

}
