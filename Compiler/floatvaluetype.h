#ifndef FLOATVALUETYPE_H
#define FLOATVALUETYPE_H
#include "valuetype.h"
class Runtime;
class FloatValueType : public ValueType {
	public:
		FloatValueType(Runtime *runtime, llvm::Module *mod);
		QString name() const {return QObject::tr("Float");}
		Type type()const{return Float;}
		/** Calculates cost for casting given ValueType to this ValueType.
		  * If returned cost is over maxCastCost, cast cannot be done. */
		CastCostType castCost(ValueType *to) const;
		Value cast(Builder *builder, const Value &v) const;
		llvm::Value *constant(float f);
		bool isTypePointer() const{return false;}
		bool isNumber() const{return true;}
	private:
		Runtime *mRuntime;
};

#endif // FLOATVALUETYPE_H
