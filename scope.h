#ifndef SCOPE_H
#define SCOPE_H
#include "symbol.h"
#include <QMap>
class Scope
{
	public:
		Scope(Scope *parent = 0);
		void addSymbol(Symbol *symbol);
		bool contains(const QString &symbol) const;
		Symbol *find(const QString &name) const;
	private:
		typedef QMap<QString, Symbol*> SymbolMap;
		SymbolMap mSymbols;
		Scope *mParent;
};

#endif // SCOPE_H
