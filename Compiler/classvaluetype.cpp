#include "classvaluetype.h"
#include "builder.h"
#include "operationflags.h"
#include "booleanvaluetype.h"
#include "abstractsyntaxtree.h"
#include "genericclassvaluetype.h"

ClassValueType::ClassValueType(const QString &name, const CodePoint &cp, Runtime *runtime) :
	ValueType(runtime),
	mName(name),
	mCodePoint(cp),
	mStructType(0) {

}

ClassValueType::ClassValueType(const QString &name, const CodePoint &cp, const QList<ClassField> &fields, Runtime *runtime) :
	ValueType(runtime),
	mName(name),
	mCodePoint(cp),
	mStructType(0) {
	setFields(fields);
}

ClassValueType::~ClassValueType() {

}

QString ClassValueType::name() const {
	return mName;
}

ValueType::CastCost ClassValueType::castingCostToOtherValueType(const ValueType *to) const {
	if (to == this) return ccNoCost;
	if (to == mRuntime->genericClassValueType()) return ccCastToBigger;
	return ccNoCast;
}

Value ClassValueType::cast(Builder *builder, const Value &v) const {
	if (v.valueType() == this) return v;
	return Value();
}

llvm::Constant *ClassValueType::defaultValue() const {
	return llvm::ConstantPointerNull::get(llvm::cast<llvm::PointerType>(mType));
}

int ClassValueType::size() const {
	return mRuntime->dataLayout().getPointerSize();
}

void ClassValueType::setFields(const QList<ClassField> &fields) {
	mFields = fields;
	for (QList<ClassField>::Iterator i = mFields.begin(); i != mFields.end(); ++i) {
		mFieldSearch[i->name()] = i;
	}
}

void ClassValueType::createOpaqueType(Builder *builder) {
	mStructType = llvm::StructType::create(builder->context(), mName.toStdString());
	mType = mStructType->getPointerTo();
}

void ClassValueType::generateLLVMType() {
	std::vector<llvm::Type*> body;
	body.reserve(mFields.size());

	for (const ClassField &f : mFields) {
		body.push_back(f.valueType()->llvmType());
	}

	mStructType->setBody(body);
}

Value ClassValueType::generateOperation(Builder *builder, int opType, const Value &operand1, const Value &operand2, OperationFlags &operationFlags) const {
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

Value ClassValueType::member(Builder *builder, const Value &a, const QString &memberName) const {
	assert(a.valueType() == this);
	QList<ClassField>::ConstIterator fieldI = mFieldSearch.value(memberName, mFields.end());
	if (fieldI == mFields.end()) {
		return Value();
	}
	int fieldIndex = fieldI - mFields.begin();

	llvm::Value *structPtr = builder->llvmValue(a);
	return Value(fieldI->valueType(), builder->irBuilder().CreateStructGEP(structPtr, fieldIndex), true);
}

ValueType *ClassValueType::memberType(const QString &memberName) const {
	QList<ClassField>::ConstIterator fieldI = mFieldSearch.value(memberName, mFields.end());
	if (fieldI != mFields.end()) {
		return fieldI->valueType();
	}
	return 0;
}


ClassField::ClassField(const QString &name, ValueType *valueType, const CodePoint &cp) :
	mName(name),
	mValueType(valueType),
	mCodePoint(cp) {

}
