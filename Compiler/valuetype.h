#ifndef VALUETYPE_H
#define VALUETYPE_H
#include <QString>
class Value;
namespace llvm {
	class Type;
	class Module;
}
class ValueType {
	public:
		enum Type {
			Integer,
			Float,
			String,
			Short,
			Byte,
			TypePointer
		};

		typedef unsigned int CastCostType;
		static const CastCostType maxCastCost = 10000;

		ValueType();
		virtual QString name() const = 0;
		virtual llvm::Type *llvmType() = 0;
		virtual Type type() const = 0;
		/** Calculates cost for casting this ValueType to given ValueType.
		 *If returned cost is over maxCastCost, cast cannot be done. */
		virtual CastCostType castCost(ValueType *to) const = 0;
		virtual Value cast(const Value &v) const = 0;
};

#endif // VALUETYPE_H
