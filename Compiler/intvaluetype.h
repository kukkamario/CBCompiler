#ifndef INTVALUETYPE_H
#define INTVALUETYPE_H
#include "valuetype.h"
class IntValueType : public ValueType {
	public:
		IntValueType(Runtime *r, llvm::Module *mod);
		QString name() const {return "integer";}
		llvm::Type *llvmType() {return mType;}
		CastCost castingCostToOtherValueType(ValueType *to) const;

		/** Casts given value to this ValueType */
		Value cast(Builder *builder, const Value &v) const;

		BasicType basicType() const{return Integer;}
		llvm::Constant *constant(int v) const;
		llvm::Constant *defaultValue() const;

		Value generateOperation(Builder *builder, int opType, const Value &operand1, const Value &operand2, OperationFlags &operationFlags) const;
		Value generateOperation(Builder *builder, int opType, const Value &operand, OperationFlags &operationFlags) const;

		bool isTypePointer() const{return false;}
		bool isNumber() const{return true;}
		int size() const { return 4; }
	private:

};

#endif // INTVALUETYPE_H
