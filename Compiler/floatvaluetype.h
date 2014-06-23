#ifndef FLOATVALUETYPE_H
#define FLOATVALUETYPE_H
#include "valuetype.h"
class Runtime;
class FloatValueType : public ValueType {
	public:
		FloatValueType(Runtime *runtime, llvm::Module *mod);
		QString name() const {return "float";}
		BasicType type()const { return Float; }
		/** Calculates cost for casting given ValueType to this ValueType.
		  * If returned cost is over maxCastCost, cast cannot be done. */
		CastCost castingCostToOtherValueType(const ValueType *to) const;
		Value cast(Builder *builder, const Value &v) const;
		llvm::Constant *constant(float f) const;
		llvm::Constant *defaultValue() const;

		Value generateOperation(Builder *builder, int opType, const Value &operand1, const Value &operand2, OperationFlags &operationFlags) const;
		Value generateOperation(Builder *builder, int opType, const Value &operand, OperationFlags &operationFlags) const;

		bool isTypePointer() const{return false;}
		bool isNumber() const{return true;}
		bool isNamedValueType() const { return true; }
		int size() const { return 4; }
	private:
		Runtime *mRuntime;
};

#endif // FLOATVALUETYPE_H
