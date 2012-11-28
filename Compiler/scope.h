#ifndef SCOPE_H
#define SCOPE_H
#include "symbol.h"
#include <QMap>
class QTextStream;
class Scope
{
	public:
		Scope(const QString &name, Scope *parent = 0);
		~Scope();
		void addSymbol(Symbol *symbol);
		bool contains(const QString &symbol) const;
		Symbol *find(const QString &name) const;
		void writeToStream(QTextStream &s) const;
		Scope *parent() const {return mParent;}
	private:
		void addChildScope(Scope *s);
		void removeChildScope(Scope *s);
		typedef QMap<QString, Symbol*> SymbolMap;
		QList<Scope*> mChildScopes;
		SymbolMap mSymbols;
		Scope *mParent;
		QString mName;
};

#endif // SCOPE_H
