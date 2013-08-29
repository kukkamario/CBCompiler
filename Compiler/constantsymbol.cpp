#include "constantsymbol.h"

ConstantSymbol::ConstantSymbol(const QString &symbolName, const ConstantValue &val, const QString &f, int line):
	Symbol(symbolName, f, line),
	mValue(val) {
}

QString ConstantSymbol::info() const {
	QString str("Constant %1 = %2 %3");

	str = str.arg(mName, mValue.typeName(), mValue.valueInfo());
	return str;
}
