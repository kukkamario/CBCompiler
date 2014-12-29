#include "variablesymbol.h"
#include "valuetype.h"

VariableSymbol::VariableSymbol(const std::string &name, ValueType *t, const CodePoint &cp):
	Symbol(name, cp),
	mValueType(t),
	mAlloca(0) {
	assert(t);
}

VariableSymbol::~VariableSymbol() {

}

std::string VariableSymbol::info() const {
	return "Variable \"" + mName + "\" " + mValueType->name();
}

void VariableSymbol::setAlloca(llvm::Value *alloc) {
	 mAlloca = alloc;
	 mAlloca->setName(name());
}
