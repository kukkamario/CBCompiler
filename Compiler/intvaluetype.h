#ifndef INTVALUETYPE_H
#define INTVALUETYPE_H
#include "valuetype.h"
class IntValueType : public ValueType
{
	public:
		IntValueType(llvm::Module *mod);
		QString name() const {return "Integer";}
		llvm::Type *llvmType() {return mType;}
		Type type() const{return Integer;}
		/** Calculates cost for casting given ValueType to this ValueType.
		  * If returned cost is over maxCastCost, cast cannot be done. */
		CastCostType castCost(ValueType *to) const;
		Value cast(const Value &v) const;
	private:
		llvm::Type *mType;
};

#endif // INTVALUETYPE_H
