#ifndef LABELSYMBOL_H
#define LABELSYMBOL_H
#include "symbol.h"
namespace llvm {
	class BasicBlock;
}
class LabelSymbol : public Symbol
{
	public:
		LabelSymbol(const QString &name, QFile *file, int line);
		Type type()const{return stLabel;}
		QString info()const;
		void setBasicBlock(llvm::BasicBlock *bb);
		llvm::BasicBlock *basicBlock()const{return mBasicBlock;}
	private:
		llvm::BasicBlock *mBasicBlock;
};

#endif // LABELSYMBOL_H
