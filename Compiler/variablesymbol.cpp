#include "variablesymbol.h"
#include "valuetype.h"

VariableSymbol::VariableSymbol(const QString &name, ValueType *t, const CodePoint &cp):
	Symbol(name, cp),
	mValueType(t),
	mAlloca(0) {
	assert(t);
}

VariableSymbol::~VariableSymbol() {

}

QString VariableSymbol::info() const {
	return QString("Variable \"%1\" %2").arg(mName, mValueType->name());
}

void VariableSymbol::setAlloca(llvm::Value *alloc) {
	 mAlloca = alloc;
	 mAlloca->setName(name().toStdString());
}
