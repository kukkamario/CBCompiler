#ifndef SYMBOLCOLLECTOR_H
#define SYMBOLCOLLECTOR_H
#include "astvisitor.h"
#include "settings.h"
#include "scope.h"
#include <QObject>
class Runtime;
class SymbolCollector : public QObject, protected ast::Visitor {
	Q_OBJECT
	public:
		SymbolCollector(Runtime *runtime, Settings *settings);
		~SymbolCollector();

		void collect(ast::Program *program, Scope *globalScope, Scope *mainScope);

	private:
		bool actAfter(ast::Global *global);

		bool actBefore(ast::Global *global);
		bool actBefore(ast::Const *c);
		bool actBefore(ast::FunctionDefinition *funcDef);
		bool actBefore(ast::VariableDefinition *def);
		bool actBefore(ast::ArrayInitialization *arrInit);
		bool actBefore(ast::TypeDefinition *typeDef);

		bool createTypeDefinition(ast::Identifier *id);

		void symbolAlreadyDefinedError(const CodePoint &cp, Symbol *existingSymbol);

		Settings *mSettings;
		Scope *mGlobalScope;
		Scope *mMainScope;
		Scope *mCurrentScope;
		Runtime *mRuntime;

	signals:
		void warning(int code, QString msg, CodePoint codePoint);
		void error(int code, QString msg, CodePoint codePoint);
};

#endif // SYMBOLCOLLECTOR_H
