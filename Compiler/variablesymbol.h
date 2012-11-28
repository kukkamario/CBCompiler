#ifndef VARIABLESYMBOL_H
#define VARIABLESYMBOL_H
#include <QString>
#include "symbol.h"
class ValueType;
namespace llvm {
	class Value;
}
class VariableSymbol : public Symbol{
	public:
		VariableSymbol(const QString &name, ValueType *t, QFile *f, int line);
		Type type()const {return stVariable;}
		QString info() const;
		void setAlloca(llvm::Value *alloc) {mAlloca = alloc;}
		llvm::Value *alloca()const {return mAlloca;}
		ValueType *valueType()const{return mValueType;}
	private:
		ValueType *mValueType;
		llvm::Value *mAlloca;
};

#endif // VARIABLESYMBOL_H
