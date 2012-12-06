#ifndef INTVALUETYPE_H
#define INTVALUETYPE_H
#include "valuetype.h"
class IntValueType : public ValueType {
	public:
		IntValueType(Runtime *r, llvm::Module *mod);
		QString name() const {return QObject::tr("Integer");}
		llvm::Type *llvmType() {return mType;}
		CastCostType castCost(ValueType *to) const;

		/** Casts given value to this ValueType */
		Value cast(llvm::IRBuilder<> *builder, const Value &v) const;

		Type type() const{return Integer;}
		llvm::Value *constant(int v);
		bool isTypePointer() const{return false;}
		bool isNumber() const{return true;}
	private:

};

#endif // INTVALUETYPE_H
