#ifndef FUNCTIONSYMBOL_H
#define FUNCTIONSYMBOL_H
#include "symbol.h"
#include "function.h"
#include <QList>

class FunctionSelectorValueType;

class FunctionSymbol:public Symbol {
	public:
		FunctionSymbol(const QString &name);
		Type type() const{ return stFunctionOrCommand; }
		void addFunction(Function *func);
		Function *exactMatch(const Function::ParamList &params) const;

		QList<Function*> functions() const { return mFunctions;}

		QString info() const;

		FunctionSelectorValueType *functionSelector() const;
	private:
		QList<Function*> mFunctions;
		mutable FunctionSelectorValueType *mSelector;

};

#endif // FUNCTIONSYMBOL_H
