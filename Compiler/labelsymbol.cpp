#include "labelsymbol.h"

LabelSymbol::LabelSymbol(const QString &name, const QString &file, int line):
	Symbol(name, file, line),
	mBasicBlock(0)
{
}

QString LabelSymbol::info() const {
	return QStringLiteral("Label ") + mName;
}

void LabelSymbol::setBasicBlock(llvm::BasicBlock *bb) {
	mBasicBlock = bb;
}
