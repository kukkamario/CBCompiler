#ifndef CUSTOMVALUETYPE_H
#define CUSTOMVALUETYPE_H
#include "valuetype.h"

class CustomValueType : public ValueType {
	public:
		CustomValueType(const std::string &name, llvm::Type *type, Runtime *r);
		std::string name() const { return mName; }
		CastCost castingCostToOtherValueType(const ValueType *to) const;
		Value cast(Builder *builder, const Value &v) const;
		llvm::Constant *defaultValue() const;
		bool isTypePointer() const { return false; }
		bool isNamedValueType() const { return true; }
		bool isNumber() const { return false; }
		int size() const;
		virtual Value generateOperation(Builder *builder, int opType, const Value &operand1, const Value &operand2, OperationFlags &operationFlags) const;
	protected:
		std::string mName;
};


#endif // CUSTOMVALUETYPE_H
