#ifndef BYTEVALUETYPE_H
#define BYTEVALUETYPE_H
#include "valuetype.h"
class ByteValueType : public ValueType {
	public:
		ByteValueType(llvm::Module *mod);
		QString name() const {return "Byte";}
		llvm::Type *llvmType() {return mType;}
		Type type() const{return Byte;}
		/** Calculates cost for casting given ValueType to this ValueType.
		  * If returned cost is over maxCastCost, cast cannot be done. */
		CastCostType castCost(ValueType *to) const;
		Value cast(const Value &v) const;
	private:
		llvm::Type *mType;
};

#endif // BYTEVALUETYPE_H
