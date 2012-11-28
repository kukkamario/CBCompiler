#include "arraysymbol.h"

ArraySymbol::ArraySymbol(const QString &name, ValueType *valType, int dim, QFile *file, int line):
	Symbol(name, file, line),
	mValueType(valType),
	mDimensions(dim)
{
}
