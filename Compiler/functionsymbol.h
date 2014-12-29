#ifndef FUNCTIONSYMBOL_H
#define FUNCTIONSYMBOL_H
#include "symbol.h"
#include "function.h"
#include <vector>

class FunctionSelectorValueType;

class FunctionSymbol:public Symbol {
	public:
		FunctionSymbol(const std::string &name);
		Type type() const{ return stFunctionOrCommand; }
		void addFunction(Function *func);
		Function *exactMatch(const Function::ParamList &params) const;

		std::vector<Function*> functions() const { return mFunctions;}

		std::string info() const;

		FunctionSelectorValueType *functionSelector() const;
	private:
		std::vector<Function*> mFunctions;
		mutable FunctionSelectorValueType *mSelector;

};

#endif // FUNCTIONSYMBOL_H
