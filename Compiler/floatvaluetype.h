#ifndef FLOATVALUETYPE_H
#define FLOATVALUETYPE_H
#include "valuetype.h"
class FloatValueType : public ValueType {
	public:
		FloatValueType(llvm::Module *mod);
		QString name() const {return "Float";}
		Type type()const{return Float;}
		/** Calculates cost for casting given ValueType to this ValueType.
		  * If returned cost is over maxCastCost, cast cannot be done. */
		CastCostType castCost(ValueType *to) const;
		Value cast(const Value &v) const;
	private:
};

#endif // FLOATVALUETYPE_H
