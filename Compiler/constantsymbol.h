#ifndef CONSTANTSYMBOL_H
#define CONSTANTSYMBOL_H
#include "symbol.h"
#include "constantvalue.h"
class ConstantSymbol : public Symbol {
	public:
		ConstantSymbol(const QString &symbolName, const ConstantValue &val, const QString &f, int line);
		Type type() const {return stConstant;}
		const ConstantValue &value()const {return mValue;}
		QString info() const;
	private:
		ConstantValue mValue;
};

#endif // CONSTANTSYMBOL_H
