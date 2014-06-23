#ifndef ARRAYVALUETYPE_H
#define ARRAYVALUETYPE_H
#include "valuetype.h"
#include "constantvalue.h"

class ArrayValueType : public ValueType {
	public:
		ArrayValueType(ValueType *baseType, llvm::Type *llvmType, int dimensions);
		QString name() const;
		llvm::Type *llvmType() {return mType;}
		CastCost castingCostToOtherValueType(const ValueType *to) const;

		/** Casts given value to this ValueType */
		Value cast(Builder *builder, const Value &v) const;

		llvm::Constant *constant(QList<ConstantValue> params) const;
		llvm::Constant *defaultValue() const;

		bool isTypePointer() const{return false;}
		bool isNumber() const{return true;}
		bool isNamedValueType() const { return false; }
		int size() const;
		llvm::PointerType *llvmType() const {return llvm::cast<llvm::PointerType>(mType);}
	private:
		ValueType *mBaseValueType;
		int mDimensions;

};

#endif // ARRAYVALUETYPE_H
