#ifndef GENERICCLASSVALUETYPE_H
#define GENERICCLASSVALUETYPE_H
#include "valuetype.h"

class GenericClassValueType : public ValueType {
	public:
		GenericClassValueType(llvm::Type *genericClassValueType, Runtime *runtime);
		virtual QString name() const;
		virtual CastCost castingCostToOtherValueType(const ValueType *to) const;
		virtual Value cast(Builder *builder, const Value &v) const;
		bool isTypePointer() const{return false;}
		bool isNumber() const{return false;}
		bool isClass() const { return true; }
		llvm::Constant *defaultValue() const;
		int size() const;
		bool isNamedValueType() const { return false; }
	protected:
};

#endif // GENERICCLASSVALUETYPE_H
