#ifndef SYMBOLCOLLECTOR_H
#define SYMBOLCOLLECTOR_H
#include "astvisitor.h"
#include "settings.h"
#include "scope.h"
#include <QObject>
class Runtime;
class ValueType;
class VariableSymbol;
class SymbolCollector : public QObject, protected ast::Visitor {
	Q_OBJECT
	public:
		SymbolCollector(Runtime *runtime, Settings *settings);
		~SymbolCollector();

		bool collect(ast::Program *program, Scope *globalScope, Scope *mainScope);

	private:
		bool actAfter(ast::Global *global);

		bool actBefore(ast::Global *global);
		bool actBefore(ast::Const *c);
		bool actBefore(ast::FunctionDefinition *funcDef);
		bool actBefore(ast::VariableDefinition *def);
		bool actBefore(ast::ArrayInitialization *arrInit);
		bool actBefore(ast::TypeDefinition *typeDef);

		bool createTypeDefinition(ast::Identifier *id);
		bool createFunctionDefinition(ast::FunctionDefinition *funcDef);

		void symbolAlreadyDefinedError(const CodePoint &cp, Symbol *existingSymbol);
		void functionAlreadyDefinedError(const CodePoint &cp, Function *oldFunctionDef);


		VariableSymbol *handleVariableDefinitionAndArrayInitialization(ast::Node *node);
		VariableSymbol *handleVariableDefinition(ast::VariableDefinition *varDef);
		VariableSymbol *handleArrayInitialization(ast::ArrayInitialization *arrayInit);
		QList<VariableSymbol*> handleVariableDefinitionList(ast::Node *node);

		ValueType *resolveValueType(ast::Node *valueType);
		ValueType *basicValueType(ast::BasicType *basicType);
		ValueType *namedValueType(ast::NamedType *namedType);
		ValueType *arrayValueType(ast::ArrayType *arrayType);
		ValueType *defaultValueType();

		int astListSize(ast::Node *node);

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
