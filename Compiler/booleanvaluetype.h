#ifndef BOOLEANVALUETYPE_H
#define BOOLEANVALUETYPE_H
#include "valuetype.h"


class BooleanValueType : public ValueType {
	public:
		BooleanValueType(Runtime *r, llvm::Module *mod);
		QString name() const {return "boolean";}
		BasicType basicType() const { return Boolean; }

		CastCost castingCostToOtherValueType(ValueType *to) const;
		Value cast(Builder *builder, const Value &v) const;
		llvm::Constant *constant(bool t) const;

		bool isNamedValueType() const { return true; }

		llvm::Constant* defaultValue() const;

		bool isTypePointer() const{return false;}

		Value generateOperation(Builder *builder, int opType, const Value &operand1, const Value &operand2, OperationFlags &operationFlags) const;
		Value generateOperation(Builder *builder, int opType, const Value &operand, OperationFlags &operationFlags) const;

		bool isNumber() const{return true;}

		bool valueCanBeConstant() const { return true; }

		int size() const { return 1; }
	private:
};

#endif // BOOLEANVALUETYPE_H
