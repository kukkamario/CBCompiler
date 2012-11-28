#ifndef ARRAYSYMBOL_H
#define ARRAYSYMBOL_H
#include "symbol.h"
class ValueType;
class ArraySymbol : public Symbol {
	public:
		ArraySymbol(const QString &name, ValueType *valType, int dim, QFile *file, int line);
		int dimensions() const {return mDimensions;}
		ValueType *valueType()const {return mValueType;}
	private:
		ValueType *mValueType;
		int mDimensions;
};

#endif // ARRAYSYMBOL_H
