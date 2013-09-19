#ifndef VALUETYPESYMBOL_H
#define VALUETYPESYMBOL_H
#include "symbol.h"
class ValueType;

class ValueTypeSymbol : public Symbol {
	public:
		ValueTypeSymbol(const QString &name, QString f, int line);
		virtual ValueType *valueType() const = 0;
		bool isValueTypeSymbol() const { return true; }
	protected:
};

class DefaultValueTypeSymbol :public ValueTypeSymbol {
	public:
		DefaultValueTypeSymbol(ValueType *valType);
		Type type() const { return stDefaultValueType; }
		ValueType *valueType() const { return mValueType; }
		QString info() const;
	private:
		ValueType *mValueType;
};

#endif // VALUETYPESYMBOL_H
