#include "scope.h"
#include <QDebug>
#include <QTextStream>
#include <assert.h>

Scope::Scope(const std::string &name,Scope *parent):
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

bool Scope::contains(const std::string &symbol) const {
	SymbolMap::const_iterator i = mSymbols.find(symbol);
	if (i != mSymbols.end()) {
		return true;
	}
	if (mParent) return mParent->contains(symbol);
	return false;
}

Symbol *Scope::find(const std::string &name) const{
	SymbolMap::const_iterator i = mSymbols.find(name);
	if (i != mSymbols.end()) {
		return i->second;
	}
	if (mParent) return mParent->find(name);
	return 0;
}

Symbol *Scope::findOnlyThisScope(const std::string &name) const {
	SymbolMap::const_iterator i = mSymbols.find(name);
	if (i != mSymbols.end()) {
		return i->second;
	}
	return 0;
}

void Scope::writeToStream(std::basic_ostream<char> &s) const {
	s << "Scope \"" << mName << "\"\n";
	for (SymbolMap::const_iterator i = mSymbols.begin(); i != mSymbols.end(); i++) {
		s << "  " << i->second->info() << '\n';
	}
	s << "\n\n";
	for (std::vector<Scope*>::const_iterator i = mChildScopes.begin(); i != mChildScopes.end(); i++) {
		(*i)->writeToStream(s);
		s << "\n\n";
	}
}

void Scope::setParent(Scope *parent) {
	if (mParent) {
		mParent->removeChildScope(this);
	}
	mParent = parent;
	mParent->addChildScope(this);
}

void Scope::addChildScope(Scope *s) {
	mChildScopes.push_back(s);
}



void Scope::removeChildScope(Scope *s) {
	mChildScopes.erase(std::remove(mChildScopes.begin(), mChildScopes.end(), s));
}
