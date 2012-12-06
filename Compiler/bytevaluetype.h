#ifndef BYTEVALUETYPE_H
#define BYTEVALUETYPE_H
#include "valuetype.h"
class ByteValueType : public ValueType {
	public:
		ByteValueType(Runtime *r, llvm::Module *mod);
		QString name() const {return QObject::tr("Byte");}
		llvm::Type *llvmType() {return mType;}
		Type type() const{return Byte;}
		/** Calculates cost for casting given ValueType to this ValueType.
		  * If returned cost is over maxCastCost, cast cannot be done. */
		CastCostType castCost(ValueType *to) const;
		Value cast(llvm::IRBuilder<> *builder, const Value &v) const;
		llvm::Value *constant(quint8 i);
		bool isTypePointer() const{return false;}
		bool isNumber() const{return true;}
	private:
		llvm::Type *mType;
};

#endif // BYTEVALUETYPE_H
