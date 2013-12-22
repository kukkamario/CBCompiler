#include "valuetypesymbol.h"
#include "valuetype.h"

ValueTypeSymbol::ValueTypeSymbol(const QString &name, const CodePoint &cp) :
	Symbol(name, cp){
}


DefaultValueTypeSymbol::DefaultValueTypeSymbol(ValueType *valType) :
	ValueTypeSymbol(valType->name(), CodePoint()),
	mValueType(valType) {
}

QString DefaultValueTypeSymbol::info() const {
	return QString("ValueType symbol: ") + name();
}
