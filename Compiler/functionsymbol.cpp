#include "functionsymbol.h"
#include "valuetype.h"
#include "functionselectorvaluetype.h"
#include "functionvaluetype.h"

FunctionSymbol::FunctionSymbol(const std::string &name):
	Symbol(name, CodePoint()),
	mSelector(0)
{
}

void FunctionSymbol::addFunction(Function *func) {
	mFunctions.push_back(func);
}

Function *FunctionSymbol::exactMatch(const Function::ParamList &params) const{
	for (std::vector<Function*>::const_iterator i = mFunctions.begin(); i != mFunctions.end(); i++) {
		Function *func = *i;
		if (func->paramTypes() == params) return func;
	}
	return 0;
}


std::string FunctionSymbol::info() const {
	std::string str("Function " + mName);
	if (mFunctions.size() > 1) str += ", " + boost::lexical_cast<std::string>(mFunctions.size()) + " overloads";
	return str;
}

FunctionSelectorValueType *FunctionSymbol::functionSelector() const {
	if (!mSelector) {
		assert(!mFunctions.empty());
		mSelector = new FunctionSelectorValueType(mFunctions.front()->functionValueType()->runtime(), mFunctions);
	}

	return mSelector;
}

