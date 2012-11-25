#include "variablesymbol.h"

VariableSymbol::VariableSymbol(const QString &name, ValueType *t, QFile *f, int line):
	Symbol(name, f, line),
	mValueType(t),
	mAlloca(0)
{
}
