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
		CastCostType castingCostToOtherValueType(ValueType *to) const;
		Value cast(Builder *builder, const Value &v) const;
		llvm::Constant *constant(float f) const;
		llvm::Constant *defaultValue() const;
		bool isTypePointer() const{return false;}
		bool isNumber() const{return true;}
		int size() const { return 4; }
	private:
		Runtime *mRuntime;
};

#endif // FLOATVALUETYPE_H
