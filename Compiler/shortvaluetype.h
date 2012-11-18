#ifndef SHORTVALUETYPE_H
#define SHORTVALUETYPE_H
#include "valuetype.h"
class ShortValueType : public ValueType
{
	public:
		ShortValueType(llvm::Module *mod);
		QString name() const {return "Short";}
		Type type() const{return Short;}
		/** Calculates cost for casting given ValueType to this ValueType.
		  * If returned cost is over maxCastCost, cast cannot be done. */
		CastCostType castCost(ValueType *to) const;
		Value cast(const Value &v) const;
	private:
};

#endif // SHORTVALUETYPE_H
