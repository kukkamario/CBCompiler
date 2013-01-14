#ifndef SCOPE_H
#define SCOPE_H
#include "symbol.h"
#include <QMap>
class QTextStream;


class Scope {
	public:
		typedef QMap<QString, Symbol*>::Iterator Iterator;
		typedef QMap<QString, Symbol*>::ConstIterator ConstIterator;
		Scope(const QString &name, Scope *parent = 0);
		~Scope();
		void addSymbol(Symbol *symbol);
		bool contains(const QString &symbol) const;
		Symbol *find(const QString &name) const;
		void writeToStream(QTextStream &s) const;
		void setParent(Scope *parent);
		Scope *parent() const {return mParent;}

		Iterator begin() {return mSymbols.begin();}
		ConstIterator begin() const {return mSymbols.begin();}
		Iterator end() {return mSymbols.end();}
		ConstIterator end() const {return mSymbols.end();}
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
