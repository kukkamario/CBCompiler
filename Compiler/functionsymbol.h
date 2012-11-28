#ifndef FUNCTIONSYMBOL_H
#define FUNCTIONSYMBOL_H
#include "symbol.h"
#include "function.h"
#include <QList>


class FunctionSymbol:public Symbol {
	public:
		FunctionSymbol(const QString &name);
		Type type() const{ return stFunctionOrCommand; }
		void addFunction(Function *func);
		QList<Function*> functions() const { return mFunctions;}
		Function *findBestOverload(const QList<ValueType*> &paramTypes, bool command = false, OverloadSearchError *err = 0);
		QString info() const;
	private:
		QList<Function*> mFunctions;

};

#endif // FUNCTIONSYMBOL_H
