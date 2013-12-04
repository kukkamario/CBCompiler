#ifndef ARRAYVALUETYPE_H
#define ARRAYVALUETYPE_H
#include "valuetype.h"
#include "constantvalue.h"

class ArrayValueType : public ValueType {
	public:
		ArrayValueType(ValueType *baseType, int dimensions);
		QString name() const;
		llvm::Type *llvmType() {return mType;}
		CastCost castingCostToOtherValueType(ValueType *to) const;

		/** Casts given value to this ValueType */
		Value cast(Builder *builder, const Value &v) const;

		llvm::Constant *constant(QList<ConstantValue> params) const;
		llvm::Constant *defaultValue() const;

		bool isTypePointer() const{return false;}
		bool isNumber() const{return true;}
		int size() const { return 4; }
	private:
		ValueType *mBaseValueType;
		int mDimensions;

};

#endif // ARRAYVALUETYPE_H
