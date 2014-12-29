#ifndef SHORTVALUETYPE_H
#define SHORTVALUETYPE_H
#include "valuetype.h"
class ShortValueType : public ValueType
{
	public:
		ShortValueType(Runtime *r, llvm::Module *mod);
		std::string name() const {return "short";}
		BasicType basicType() const{return Short;}
		/** Calculates cost for casting given ValueType to this ValueType.
		  * If returned cost is over maxCastCost, cast cannot be done. */
		CastCost castingCostToOtherValueType(const ValueType *to) const;
		Value cast(Builder *builder, const Value &v) const;
		llvm::Constant *constant(uint16_t i) const;
		llvm::Constant *defaultValue() const;

		Value generateOperation(Builder *builder, int opType, const Value &operand1, const Value &operand2, OperationFlags &operationFlags) const;
		Value generateOperation(Builder *builder, int opType, const Value &operand, OperationFlags &operationFlags) const;

		bool isTypePointer() const{return false;}
		bool isNumber() const{return true;}
		bool isNamedValueType() const { return true; }
		int size() const { return 2; }
	private:
};

#endif // SHORTVALUETYPE_H
