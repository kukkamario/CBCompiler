#ifndef SYMBOLCOLLECTOR_H
#define SYMBOLCOLLECTOR_H
#include "astvisitor.h"
#include "settings.h"
#include "scope.h"
class SymbolCollector : protected ast::Visitor {
	public:
		SymbolCollector(Settings *settings);
		~SymbolCollector();

		void collect(ast::Program *program, Scope *globalScope, Scope *mainScope);

	private:
		bool actBefore(ast::Global *global);
		bool actAfter(ast::Global *global);

		bool actBefore(ast::Const *c);

		bool actBefore(ast::FunctionDefinition *funcDef);

		bool actBefore(ast::VariableDefinition *def);

		bool actBefore(ast::ArrayInitialization *arrInit);

		Settings *mSettings;
		Scope *mGlobalScope;
		Scope *mMainScope;
		Scope *mCurrentScope;
};

#endif // SYMBOLCOLLECTOR_H
