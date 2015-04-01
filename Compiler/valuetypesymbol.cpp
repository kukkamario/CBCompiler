#include "valuetypesymbol.h"
#include "valuetype.h"

ValueTypeSymbol::ValueTypeSymbol(const QString &name, const CodePoint &cp) :
	Symbol(name, cp){
}


DefaultValueTypeSymbol::DefaultValueTypeSymbol(const QString &name, ValueType *valType) :
	ValueTypeSymbol(name, CodePoint()),
	mValueType(valType) {
}

QString DefaultValueTypeSymbol::info() const {
	return QString("ValueType symbol: ") + name();
}
