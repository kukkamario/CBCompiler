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

bool StructValueType::containsValueType(const ValueType *valueType) const {
	for (const StructField &f : mFields) {
		if (f.valueType() == valueType) return true;
		if (f.valueType()->isStruct()) {
			StructValueType *structValueType = static_cast<StructValueType*>(f.valueType());
			if (structValueType->containsValueType(valueType)) return true;
		}
	}
	return false;
}

bool StructValueType::containsItself() const {
	return containsValueType(this);
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
	return llvm::Constant::getNullValue(mType);
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


bool StructValueType::generateLLVMType() {
	std::vector<llvm::Type*> body;
	body.reserve(mFields.size());

	for (const StructField &f : mFields) {
		if (f.valueType()->isStruct()) {
			if (!static_cast<StructValueType*>(f.valueType())->isGenerated()) return false;
		}
		body.push_back(f.valueType()->llvmType());
	}

	mStructType = llvm::StructType::create(body, mName.toStdString());
	mType = mStructType;
	return true;
}

bool StructValueType::isGenerated() const {
	return mStructType != 0;
}

Value StructValueType::generateOperation(Builder *builder, int opType, const Value &operand1, const Value &operand2, OperationFlags &operationFlags) const {
	if (operand1.valueType() == this && operand2.valueType() == this) {
		switch (opType) {
			case ast::ExpressionNode::opAssign: {
				if (!operand1.isReference()) {
					operationFlags |= OperationFlag::ReferenceRequired;
					return Value();
				}
				for (int fieldIndex = 0; fieldIndex < mFields.size(); ++fieldIndex) {
					Value f1 = this->field(builder, operand1, fieldIndex);
					Value f2 = this->field(builder, operand2, fieldIndex);
					f1.valueType()->generateOperation(builder, ast::ExpressionNode::opAssign, f1, f2, operationFlags);
					if (operationFlagsContainFatalFlags(operationFlags)) {
						return Value();
					}
				}
				return operand1;
			}
			case ast::ExpressionNode::opEqual: {
				Value result;
				for (int fieldIndex = 0; fieldIndex < mFields.size(); ++fieldIndex) {
					Value f1 = this->field(builder, operand1, fieldIndex);
					Value f2 = this->field(builder, operand2, fieldIndex);
					Value r = f1.valueType()->generateOperation(builder, ast::ExpressionNode::opEqual, f1, f2, operationFlags);
					if (operationFlagsContainFatalFlags(operationFlags)) {
						return Value();
					}
					if (result.isValid()) {
						result = builder->and_(result, r);
					}
					else {
						result = r;
					}
				}
				return result;
			}
			case ast::ExpressionNode::opNotEqual: {
				Value result;
				for (int fieldIndex = 0; fieldIndex < mFields.size(); ++fieldIndex) {
					Value f1 = this->field(builder, operand1, fieldIndex);
					Value f2 = this->field(builder, operand2, fieldIndex);
					Value r = f1.valueType()->generateOperation(builder, ast::ExpressionNode::opNotEqual, f1, f2, operationFlags);
					if (operationFlagsContainFatalFlags(operationFlags)) {
						return Value();
					}
					if (result.isValid()) {
						result = builder->or_(result, r);
					}
					else {
						result = r;
					}
				}
				return result;
			}
		}
	}
	operationFlags |= OperationFlag::NoSuchOperation;
	return Value();
}

void StructValueType::generateDestructor(Builder *builder, const Value &v) {
	for (int fieldIndex = 0; fieldIndex < mFields.size(); ++fieldIndex) {
		Value f = this->field(builder, v, fieldIndex);
		if (f.isReference()) {
			f = Value(f.valueType(), builder->irBuilder().CreateLoad(f.value()), false);
		}
		builder->destruct(f);
	}
}

Value StructValueType::generateLoad(Builder *builder, const Value &var) const {
	assert(var.isReference());
	llvm::Value *ret = llvm::UndefValue::get(llvmType());
	for (int fieldIndex = 0; fieldIndex < mFields.size(); ++fieldIndex) {
		Value f = field(builder, var, fieldIndex);
		Value fLoaded = f.valueType()->generateLoad(builder, f);
		unsigned idx[1];
		idx[0] = fieldIndex;
		ret = builder->irBuilder().CreateInsertValue(ret, fLoaded.value(), idx);
	}
	return Value(var.valueType(), ret, false);
}

Value StructValueType::member(Builder *builder, const Value &a, const QString &memberName) const {
	assert(a.valueType() == this);
	QList<StructField>::ConstIterator fieldI = mFieldSearch.value(memberName, mFields.end());
	if (fieldI == mFields.end()) {
		return Value();
	}
	int fieldIndex = fieldI - mFields.begin();
	return field(builder, a, fieldIndex);
}

ValueType *StructValueType::memberType(const QString &memberName) const {
	QList<StructField>::ConstIterator fieldI = mFieldSearch.value(memberName, mFields.end());
	if (fieldI != mFields.end()) {
		return fieldI->valueType();
	}
	return 0;
}

Value StructValueType::field(Builder *builder, const Value &a, int fieldIndex) const {
	assert(a.valueType() == this);
	if (a.isReference()) {
		return Value(mFields.at(fieldIndex).valueType(),
					 builder->irBuilder().CreateStructGEP(a.value(), fieldIndex),
					 true);
	}
	else {
		unsigned ids[1] = {fieldIndex};
		return Value(mFields.at(fieldIndex).valueType(),
					 builder->irBuilder().CreateExtractValue(a.value(), ids),
					 false);
	}

}


StructField::StructField(const QString &name, ValueType *valueType, const CodePoint &cp) :
	mName(name),
	mValueType(valueType),
	mCodePoint(cp) {

}
