#include "valuetypesymbol.h"
#include "valuetype.h"

ValueTypeSymbol::ValueTypeSymbol(const QString &name, QString f, int line) :
	Symbol(name, f, line){
}


DefaultValueTypeSymbol::DefaultValueTypeSymbol(ValueType *valType) :
	ValueTypeSymbol(valType->name(), QString(), 0),
	mValueType(valType) {
}

QString DefaultValueTypeSymbol::info() const {
	return QString("ValueType symbol: ") + name();
}
