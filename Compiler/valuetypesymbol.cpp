#include "valuetypesymbol.h"
#include "valuetype.h"

ValueTypeSymbol::ValueTypeSymbol(const std::string &name, const CodePoint &cp) :
	Symbol(name, cp){
}


DefaultValueTypeSymbol::DefaultValueTypeSymbol(ValueType *valType) :
	ValueTypeSymbol(valType->name(), CodePoint()),
	mValueType(valType) {
}

std::string DefaultValueTypeSymbol::info() const {
	return std::string("ValueType symbol: ") + name();
}
