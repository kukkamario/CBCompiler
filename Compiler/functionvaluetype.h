#ifndef FUNCTIONVALUETYPE_H
#define FUNCTIONVALUETYPE_H
#include "valuetype.h"
class Function;
class FunctionValueType : public ValueType {
	public:
		FunctionValueType(Runtime *r, ValueType *retType, const std::vector<ValueType*> &paramList);
		~FunctionValueType() {}
		std::string name() const;
		virtual bool isNamedValueType() const { return false; }
		llvm::Type *llvmType() const {return mType;}
		bool isTypePointer() const { return false; }
		bool isNumber() const { return false; }
		virtual llvm::Constant *defaultValue() const;

		virtual int size() const;

		virtual CastCost castingCostToOtherValueType(const ValueType *to) const;

		virtual Value cast(Builder *, const Value &v) const;
		
		virtual bool isCallable() const { return true; }
		
		const std::vector<ValueType*> &paramTypes() const { return mParamTypes; }
		ValueType *returnType() const { return mReturnType; }

		Value generateLoad(Builder *builder, const Value &var) const;
		Value generateOperation(Builder *builder, int opType, const Value &operand1, const Value &operand2, OperationFlags &operationFlags) const;
	protected:
		ValueType *mReturnType;
		std::vector<ValueType*> mParamTypes;
};

#endif // FUNCTIONVALUETYPE_H
