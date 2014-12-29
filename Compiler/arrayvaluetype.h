#ifndef ARRAYVALUETYPE_H
#define ARRAYVALUETYPE_H
#include "valuetype.h"
#include "constantvalue.h"

class ArrayValueType : public ValueType {
	public:
		ArrayValueType(ValueType *baseType, llvm::Type *llvmType, int dimensions);
		std::string name() const;
		llvm::Type *llvmType() const {return mType;}
		CastCost castingCostToOtherValueType(const ValueType *to) const;

		/** Casts given value to this ValueType */
		Value cast(Builder *builder, const Value &v) const;

		llvm::Constant *constant(std::vector<ConstantValue> params) const;
		llvm::Constant *defaultValue() const;

		bool isTypePointer() const{return false;}
		bool isArray() const { return true; }
		bool isNumber() const{return true;}
		bool isNamedValueType() const { return false; }
		int size() const;

		Value generateOperation(Builder *builder, int opType, const Value &operand1, const Value &operand2, OperationFlags &operationFlags) const;
		void generateDestructor(Builder *builder, const Value &value);
		Value generateLoad(Builder *builder, const Value &var) const;
		Value dimensionSize(Builder *builder, const Value &array, const Value &dimNum);

		void assignArray(Builder *builder, llvm::Value *var, llvm::Value *array);
		Value constructArray(Builder *builder, const std::vector<Value> &dims);
		Value arraySubscript(Builder *builder, const Value &array, const std::vector<Value> &dims);
		void refArray(Builder *builder, llvm::Value *array) const;
		void destructArray(Builder *builder, llvm::Value *array);

		llvm::Value *dataArray(Builder *builder, const Value &array);
		llvm::Value *totalSize(Builder *builder, const Value &array);

		int dimensions() const { return mDimensions; }
		ValueType *baseType() const { return mBaseValueType; }
	private:
		ValueType *mBaseValueType;
		llvm::Function *mConstructFunction;
		int mDimensions;

};

#endif // ARRAYVALUETYPE_H
