#ifndef TYPEVALUETYPE_H
#define TYPEVALUETYPE_H
#include "valuetype.h"

class TypeValueType : public ValueType {
	public:
		TypeValueType(Runtime *r, llvm::Type *type);
		QString name() const { return QObject::tr("Type"); }
		llvm::Type *llvmType() { return mType; }
		eType type() const { return Type; }
		/** Calculates cost for casting given ValueType to this ValueType.
		  * If returned cost is over maxCastCost, cast cannot be done. */
		CastCostType castingCostToOtherValueType(ValueType *to) const;
		Value cast(Builder *, const Value &v) const;
		llvm::Constant *defaultValue() const;

		bool isTypePointer() const{return false;}
		bool isNumber() const{return false;}
		int size() const;
};

#endif // TYPEVALUETYPE_H
