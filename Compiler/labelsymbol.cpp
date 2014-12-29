#include "labelsymbol.h"

LabelSymbol::LabelSymbol(const std::string &name, const CodePoint &cp):
	Symbol(name, cp),
	mBasicBlock(0)
{
}

std::string LabelSymbol::info() const {
	return "Label " + mName;
}

void LabelSymbol::setBasicBlock(llvm::BasicBlock *bb) {
	mBasicBlock = bb;
}
