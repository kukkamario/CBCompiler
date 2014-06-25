#ifndef SYMBOLCOLLECTOR_H
#define SYMBOLCOLLECTOR_H
#include "astvisitor.h"
#include "settings.h"
#include "scope.h"
#include "typeresolver.h"
#include "cbfunction.h"
#include "constantexpressionevaluator.h"

#include <QObject>

class Runtime;
class ValueType;
class VariableSymbol;
class FunctionSymbol;
class ConstantSymbol;
class SymbolCollector : public QObject, protected ast::Visitor {
	Q_OBJECT
	public:
		SymbolCollector(Runtime *runtime, Settings *settings);
		~SymbolCollector();

		bool collect(ast::Program *program, Scope *globalScope, Scope *mainScope);


		CBFunction *functionByDefinition(ast::FunctionDefinition *def) const;

	private:
		void visit(ast::Global *c);
		void visit(ast::Const *c);
		void visit(ast::Dim *c);
		void visit(ast::Variable *c);
		void visit(ast::Label *c);
		void visit(ast::Identifier *c);

		void visit(ast::ExpressionNode *c);

		bool createTypeDefinition(ast::Identifier *id);
		bool createTypeFields(ast::TypeDefinition *def);
		bool createFunctionDefinition(ast::FunctionDefinition *funcDef);

		void symbolAlreadyDefinedError(const CodePoint &cp, Symbol *existingSymbol);
		void functionAlreadyDefinedError(const CodePoint &cp, Function *oldFunctionDef);

		QList<VariableSymbol*> variableDefinitionList(ast::Node *node, Scope *scope);
		QList<CBFunction::Parameter> functionParameterList(ast::Node *node, Scope *scope);

		ValueType *resolveValueType(ast::Node *valueType);

		VariableSymbol *variableDefinition(ast::Node *def, Scope *scope);
		VariableSymbol *variableDefinition(ast::VariableDefinition *def, Scope *scope);
		VariableSymbol *variableDefinition(ast::ArrayInitialization *def, Scope *scope);

		VariableSymbol *addVariableSymbol(ast::Identifier *identifier, ValueType *type, Scope *scope);
		ConstantSymbol *addConstantSymbol(ast::Identifier *identifier, ValueType *type, Scope *scope);

		int astListSize(ast::Node *node);

		Settings *mSettings;
		Scope *mGlobalScope;
		Scope *mMainScope;
		Scope *mCurrentScope;
		Runtime *mRuntime;
		TypeResolver mTypeResolver;
		bool mValid;
		ConstantExpressionEvaluator mConstEval;

		QMap<ast::FunctionDefinition*, CBFunction*> mFunctions;
	private slots:
		void errorOccured(int, QString, CodePoint);
	signals:
		void warning(int code, QString msg, CodePoint codePoint);
		void error(int code, QString msg, CodePoint codePoint);
};

#endif // SYMBOLCOLLECTOR_H
