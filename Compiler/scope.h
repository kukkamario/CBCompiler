#ifndef SCOPE_H
#define SCOPE_H
#include "symbol.h"
#include <map>
class QTextStream;

/**
 * @brief The Scope class
 */
class Scope {
	public:
		typedef std::map<std::string, Symbol*>::iterator iterator;
		typedef std::map<std::string, Symbol*>::const_iterator const_iterator;
		Scope(const std::string &name, Scope *parent = 0);
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
		bool contains(const std::string &symbol) const;

		/**
		 * @brief contains Searches for a symbol with a given name from this scope and all parent scopes.
		 * @param symbol The name of the symbol
		 * @return Pointer to the symbol found or a null pointer.
		 */
		Symbol *find(const std::string &name) const;

		Symbol *findOnlyThisScope(const std::string &name) const;

		/**
		 * @brief writeToStream Writes the scope to a stream for a debugging purposes.
		 * @param s Stream
		 */
		void writeToStream(std::basic_ostream<char> &s) const;

		/**
		 * @brief setParent Sets the parent scope
		 * @param parent New parent scope
		 */
		void setParent(Scope *parent);
		Scope *parent() const {return mParent;}

		iterator begin() {return mSymbols.begin();}
		const_iterator begin() const {return mSymbols.begin();}
		iterator end() {return mSymbols.end();}
		const_iterator end() const {return mSymbols.end();}
	private:
		void addChildScope(Scope *s);
		void removeChildScope(Scope *s);
		typedef std::map<std::string, Symbol*> SymbolMap;
		std::vector<Scope*> mChildScopes;
		SymbolMap mSymbols;
		Scope *mParent;
		std::string mName;
};

#endif // SCOPE_H
