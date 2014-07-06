#include "functionsymbol.h"
#include "valuetype.h"
#include "functionselectorvaluetype.h"
#include "functionvaluetype.h"

FunctionSymbol::FunctionSymbol(const QString &name):
	Symbol(name, CodePoint()),
	mSelector(0)
{
}

void FunctionSymbol::addFunction(Function *func) {
	mFunctions.append(func);
}

Function *FunctionSymbol::exactMatch(const Function::ParamList &params) const{
	for (QList<Function*>::ConstIterator i = mFunctions.begin(); i != mFunctions.end(); i++) {
		Function *func = *i;
		if (func->paramTypes() == params) return func;
	}
	return 0;
}


QString FunctionSymbol::info() const {
	QString str("Function " + mName);
	if (mFunctions.count() > 1) str += ", " + QString::number(mFunctions.count()) + " overloads";
	return str;
}

FunctionSelectorValueType *FunctionSymbol::functionSelector() const {
	if (!mSelector) {
		assert(!mFunctions.empty());
		mSelector = new FunctionSelectorValueType(mFunctions.first()->functionValueType()->runtime(), mFunctions);
	}

	return mSelector;
}

