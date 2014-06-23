#ifndef CONSTANTSYMBOL_H
#define CONSTANTSYMBOL_H
#include "symbol.h"
#include "constantvalue.h"

class ConstantSymbol : public Symbol {
	public:
		ConstantSymbol(const QString &symbolName, ValueType *valueType, const CodePoint &cp);
		ConstantSymbol(const QString &symbolName, ValueType *valueType, const ConstantValue &c, const CodePoint &cp);
		Type type() const {return stConstant; }
		void setValue(const ConstantValue &v) { mValue = v; }
		const ConstantValue &value()const {return mValue;}
		bool autoValueType() const { return mValueType == 0; }
		ValueType *valueType() const { return mValueType; }
		QString info() const;
	private:
		ConstantValue mValue;
		ValueType *mValueType;
};

#endif // CONSTANTSYMBOL_H
