#ifndef FUNCTIONCODEGENERATOR_H
#define FUNCTIONCODEGENERATOR_H

#include <QObject>
#include "astvisitor.h"
#include "settings.h"
#include "scope.h"
#include "value.h"
#include "builder.h"

class CBFunction;

class FunctionCodeGenerator : public QObject, protected ast::Visitor {
		Q_OBJECT
	public:
		FunctionCodeGenerator(Runtime *runtime, Settings *settings, QObject *parent = 0);
		bool generate(Builder *builder, ast::Node *block, CBFunction *func, Scope *globalScope);
		bool generateMainBlock(Builder *builder, ast::Node *block, llvm::Function *func, Scope *localScope, Scope *globalScope);

	private:
		void visit(ast::Expression *n);
		void visit(ast::Const *n);
		void visit(ast::VariableDefinition *n);
		void visit(ast::ArrayInitialization *n);
		void visit(ast::FunctionCall *n);

		Value generate(ast::Integer *n);
		Value generate(ast::String *n);
		Value generate(ast::Float *n);
		Value generate(ast::Variable *n);
		Value generate(ast::Identifier *n);
		Value generate(ast::Expression *n);
		Value generate(ast::FunctionCall *n);
		Value generate(ast::KeywordFunctionCall *n);
		Value generate(ast::ArraySubscript *n);

		Value generate(ast::WhileStatement *n);
		Value generate(ast::RepeatForeverStatement *n);
		Value generate(ast::RepeatUntilStatement *n);
		Value generate(ast::ForToStatement *n);
		Value generate(ast::ForEachStatement *n);
		Value generate(ast::SelectStatement *n);
		Value generate(ast::SelectCase *n);
		Value generate(ast::Const *n);
		Value generate(ast::Node *n);

		Function *findBestOverload(const QList<Function*> &functions, const QList<Value> &parameters, bool command, const CodePoint &cp);
		QList<Value> generateParameterList(ast::Node *n);

		bool generateAllocas();
		void generateDestructors();


		Settings *mSettings;
		Scope *mLocalScope;
		Scope *mGlobalScope;
		Runtime *mRuntime;
		Builder *mBuilder;
		llvm::Function *mFunction;
		bool mValid;
	signals:
		void warning(int code, QString msg, CodePoint codePoint);
		void error(int code, QString msg, CodePoint codePoint);
	private slots:
		void errorOccured(int, QString, CodePoint);
};

#endif // FUNCTIONCODEGENERATOR_H
