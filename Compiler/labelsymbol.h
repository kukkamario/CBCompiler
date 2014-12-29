#ifndef LABELSYMBOL_H
#define LABELSYMBOL_H
#include "symbol.h"
namespace llvm {
	class BasicBlock;
}
class LabelSymbol : public Symbol
{
	public:
		LabelSymbol(const std::string &name, const CodePoint &cp);
		Type type()const{return stLabel;}
		std::string info()const;
		void setBasicBlock(llvm::BasicBlock *bb);
		llvm::BasicBlock *basicBlock()const{return mBasicBlock;}
	private:
		llvm::BasicBlock *mBasicBlock;
};

#endif // LABELSYMBOL_H
