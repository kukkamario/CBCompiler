#include "constantsymbol.h"

ConstantSymbol::ConstantSymbol(const std::string &symbolName, ValueType *valueType, const CodePoint &cp):
	Symbol(symbolName, cp),
	mValueType(valueType) {

}

ConstantSymbol::ConstantSymbol(const std::string &symbolName, ValueType *valueType, const ConstantValue &c, const CodePoint &cp):
	Symbol(symbolName, cp),
	mValue(c),
	mValueType(valueType)
	{

}

std::string ConstantSymbol::info() const {
	return "Constant " + mName + " = " + mValue.typeName() + " " + mValue.valueInfo();
}
