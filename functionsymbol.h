#ifndef FUNCTIONSYMBOL_H
#define FUNCTIONSYMBOL_H
#include "symbol.h"
#include "function.h"
#include <QList>
class ValueType;
class FunctionSymbol:public Symbol
{
	public:
		enum OverloadSearchError {
			oseNoError,
			oseCannotFindAny,
			oseFoundMultipleOverloads
		};

		FunctionSymbol(const QString &name);
		Type type() const{ return stFunction; }
		QList<Function*> functions() const { return mFunctions;}
		Function *findBestOverload(const QList<ValueType*> &paramTypes, OverloadSearchError *err = 0);
	private:
		QList<Function*> mFunctions;

};

#endif // FUNCTIONSYMBOL_H
