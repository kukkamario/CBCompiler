#ifndef SCOPE_H
#define SCOPE_H
#include "symbol.h"
#include <QMap>
class QTextStream;

/**
 * @brief The Scope class
 */
class Scope {
	public:
		typedef QMap<QString, Symbol*>::Iterator Iterator;
		typedef QMap<QString, Symbol*>::ConstIterator ConstIterator;
		Scope(const QString &name, Scope *parent = 0);
		~Scope();
		/**
		 * @brief addSymbol Adds symbol to the scope
		 * @param symbol Pointer to the symbol
		 */
		void addSymbol(Symbol *symbol);

		/**
		 * @brief contains Searches for a symbol with a given name from this scope and all parent scopes.
		 * @param symbol The name of the symbol
		 * @return True, if symbol is found and false otherwise
		 */
		bool contains(const QString &symbol) const;

		/**
		 * @brief contains Searches for a symbol with a given name from this scope and all parent scopes.
		 * @param symbol The name of the symbol
		 * @return Pointer to the symbol found or a null pointer.
		 */
		Symbol *find(const QString &name) const;

		/**
		 * @brief writeToStream Writes the scope to a stream for a debugging purposes.
		 * @param s Stream
		 */
		void writeToStream(QTextStream &s) const;

		/**
		 * @brief setParent Sets the parent scope
		 * @param parent New parent scope
		 */
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
