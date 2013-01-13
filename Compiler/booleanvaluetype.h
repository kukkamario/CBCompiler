#ifndef BOOLEANVALUETYPE_H
#define BOOLEANVALUETYPE_H
#include "valuetype.h"

class BooleanValueType : public ValueType {
	public:
		BooleanValueType(Runtime *r, llvm::Module *mod);
		QString name() const {return QObject::tr("Boolean");}
		llvm::Type *llvmType() {return mType;}
		/** Calculates cost for casting given ValueType to this ValueType.
		  * If returned cost is over maxCastCost, cast cannot be done. */
		CastCostType castCost(ValueType *to) const;
		Value cast(Builder *builder, const Value &v) const;
		Type type() const{return Byte;}
		llvm::Value *constant(bool t);
		bool isTypePointer() const{return false;}
		bool isNumber() const{return true;}
	private:
		llvm::Type *mType;
};

#endif // BOOLEANVALUETYPE_H
