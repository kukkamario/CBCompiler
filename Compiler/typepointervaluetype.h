#ifndef TYPEPOINTERVALUETYPE_H
#define TYPEPOINTERVALUETYPE_H
#include "valuetype.h"
class TypeSymbol;
class TypePointerValueType : public ValueType {
	public:
		TypePointerValueType(Runtime *r, TypeSymbol *s);
		Type type()const{return TypePointer;}
		QString name()const;
		CastCostType castCost(ValueType *to) const;
		Value cast(llvm::IRBuilder<> *builder, const Value &v) const;
	private:
		TypeSymbol *mTypeSymbol;
};

#endif // TYPEPOINTERVALUETYPE_H
