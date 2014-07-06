#include "labelsymbol.h"

LabelSymbol::LabelSymbol(const QString &name, const CodePoint &cp):
	Symbol(name, cp),
	mBasicBlock(0)
{
}

QString LabelSymbol::info() const {
	return QStringLiteral("Label ") + mName;
}

void LabelSymbol::setBasicBlock(llvm::BasicBlock *bb) {
	mBasicBlock = bb;
}
