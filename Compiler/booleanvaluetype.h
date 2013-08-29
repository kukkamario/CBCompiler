#ifndef BOOLEANVALUETYPE_H
#define BOOLEANVALUETYPE_H
#include "valuetype.h"

/**
 * @brief The BooleanValueType class is only used internally to remove useless casts. Acts like IntegerValueType.
 *
 * @see IntegerValueType
 */
class BooleanValueType : public ValueType {
	public:
		BooleanValueType(Runtime *r, llvm::Module *mod);
		QString name() const {return "boolean";}
		/**
		 * @brief The type of ValueType.
		 * @see ValueType::Type
		 */
		eType type() const{return Boolean;}

		/** Calculates cost for casting given ValueType to this ValueType.
		  * If returned cost is over or equal to maxCastCost, cast cannot be done. */
		CastCostType castingCostToOtherValueType(ValueType *to) const;
		Value cast(Builder *builder, const Value &v) const;
		llvm::Constant *constant(bool t) const;

		llvm::Constant* defaultValue() const;

		/**
		 * @brief Is ValueType a type pointer.
		 * @return false
		 */
		bool isTypePointer() const{return false;}

		/**
		 * @brief Is ValueType a number.
		 * @return true
		 */
		bool isNumber() const{return true;}

		bool valueCanBeConstant() const { return true; }

		int size() const { return 1; }
	private:
};

#endif // BOOLEANVALUETYPE_H
