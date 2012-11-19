#include "scope.h"

Scope::Scope(Scope *parent):
	mParent(parent)
{
}

void Scope::addSymbol(Symbol *symbol) {
	assert(mSymbols.find(symbol->name()) == mSymbols.end());
	mSymbols[symbol->name()] = symbol;
}

bool Scope::contains(const QString &symbol) const {
	SymbolMap::const_iterator i = mSymbols.find(symbol);
	if (i != mSymbols.end()) {
		return true;
	}
	if (mParent) return mParent->contains(symbol);
	return false;
}

Symbol *Scope::find(const QString &name) const{
	SymbolMap::const_iterator i = mSymbols.find(name);
	if (i != mSymbols.end()) {
		return i.value();
	}
	if (mParent) return mParent->find(name);
	return 0;
}
