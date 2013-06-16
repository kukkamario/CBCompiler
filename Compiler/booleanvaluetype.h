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
		QString name() const {return QObject::tr("Boolean");}
		/**
		 * @brief The type of ValueType.
		 * @see ValueType::Type
		 */
		Type type() const{return Boolean;}

		/**
		 * @brief llvm::Type of ValueType
		 * @return llvm::Type pointer
		 */
		llvm::Type *llvmType() {return mType;}

		/** Calculates cost for casting given ValueType to this ValueType.
		  * If returned cost is over or equal to maxCastCost, cast cannot be done. */
		CastCostType castCost(ValueType *to) const;
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
	private:
		llvm::Type *mType;
};

#endif // BOOLEANVALUETYPE_H
