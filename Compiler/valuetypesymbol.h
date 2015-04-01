#ifndef VALUETYPESYMBOL_H
#define VALUETYPESYMBOL_H
#include "symbol.h"
class ValueType;

class ValueTypeSymbol : public Symbol {
	public:
		ValueTypeSymbol(const QString &name, const CodePoint &cp);
		virtual ValueType *valueType() const = 0;
		bool isValueTypeSymbol() const { return true; }
	protected:
};

class DefaultValueTypeSymbol :public ValueTypeSymbol {
	public:
		DefaultValueTypeSymbol(const QString &name, ValueType *valType);
		Type type() const { return stValueType; }
		ValueType *valueType() const { return mValueType; }
		QString info() const;
	private:
		ValueType *mValueType;
};

#endif // VALUETYPESYMBOL_H
