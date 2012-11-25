#include "scope.h"
#include <QDebug>
#include <QTextStream>
Scope::Scope(const QString &name,Scope *parent):
	mParent(parent),
	mName(name) {
	if (mParent) {
		mParent->addChildScope(this);
	}
}

Scope::~Scope() {
	if (mParent) {
		mParent->removeChildScope(this);
	}
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

void Scope::writeToStream(QTextStream &s) const {
	s << "Scope \"" << mName << "\"\n";
	for (SymbolMap::ConstIterator i = mSymbols.begin(); i != mSymbols.end(); i++) {
		s << "  " << i.value()->info() << '\n';
	}
	s << "\n\n";
	for (QList<Scope*>::ConstIterator i = mChildScopes.begin(); i != mChildScopes.end(); i++) {
		(*i)->writeToStream(s);
		s << "\n\n";
	}
}

void Scope::addChildScope(Scope *s) {
	mChildScopes.append(s);
}



void Scope::removeChildScope(Scope *s) {
	mChildScopes.removeOne(s);
}
