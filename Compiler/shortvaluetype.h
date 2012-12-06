#ifndef SHORTVALUETYPE_H
#define SHORTVALUETYPE_H
#include "valuetype.h"
class ShortValueType : public ValueType
{
	public:
		ShortValueType(Runtime *r, llvm::Module *mod);
		QString name() const {return QObject::tr("Short");}
		Type type() const{return Short;}
		/** Calculates cost for casting given ValueType to this ValueType.
		  * If returned cost is over maxCastCost, cast cannot be done. */
		CastCostType castCost(ValueType *to) const;
		Value cast(llvm::IRBuilder<> *builder, const Value &v) const;
		llvm::Value *constant(quint16 i);
		bool isTypePointer() const{return false;}
		bool isNumber() const{return true;}
	private:
};

#endif // SHORTVALUETYPE_H
