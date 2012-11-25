#include "typepointervaluetype.h"
#include "typesymbol.h"
#include "value.h"

TypePointerValueType::TypePointerValueType(Runtime *r, TypeSymbol *s):
	ValueType(r),
	mTypeSymbol(s){
	mType = s->llvmType();
}

QString TypePointerValueType::name() const {
	return mTypeSymbol->name();
}

ValueType::CastCostType TypePointerValueType::castCost(ValueType *to) const {
	if (to == this) return 0;
	return maxCastCost;
}

Value TypePointerValueType::cast(llvm::IRBuilder<> *builder, const Value &v) const {
	if (v.valueType() != this) { //TODO: NULL
		return Value();
	}
	return v;
}
