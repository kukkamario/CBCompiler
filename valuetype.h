#ifndef VALUETYPE_H
#define VALUETYPE_H
#include <QString>
class Value;
class ValueType
{
	public:
		typedef unsigned int CastCostType;
		static const CastCostType maxCastCost = 10000;

		ValueType();
		virtual QString name() const = 0;
		/** Calculates cost for casting given ValueType to this ValueType.
		 *If returned cost is over maxCastCost, cast cannot be done. */
		virtual CastCostType castCost(ValueType *from) const = 0;
		virtual Value cast(const Value &v) const = 0;
};

#endif // VALUETYPE_H
