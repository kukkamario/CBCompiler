#ifndef FUNCTIONCODEGENERATOR_H
#define FUNCTIONCODEGENERATOR_H

#include <QObject>
#include "astvisitor.h"
#include "settings.h"
#include "scope.h"
#include "value.h"
#include "builder.h"
#include "constantexpressionevaluator.h"

class CBFunction;
class LabelSymbol;

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

		void visit(ast::IfStatement *n);
		void visit(ast::WhileStatement *n);
		void visit(ast::RepeatForeverStatement *n);
		void visit(ast::RepeatUntilStatement *n);
		void visit(ast::ForToStatement *n);
		void visit(ast::ForEachStatement *n);
		void visit(ast::SelectStatement *n);
		void visit(ast::SelectCase *n);
		void visit(ast::Return *n);
		void visit(ast::Goto *n);
		void visit(ast::Gosub *n);
		void visit(ast::Label *n);
		void visit(ast::Exit *n);

		Value generate(ast::Integer *n);
		Value generate(ast::String *n);
		Value generate(ast::Float *n);
		Value generate(ast::Variable *n);
		Value generate(ast::Identifier *n);
		Value generate(ast::Expression *n);
		Value generate(ast::Unary *n);
		Value generate(ast::FunctionCall *n);
		Value generate(ast::KeywordFunctionCall *n);
		Value generate(ast::ArraySubscript *n);
		Value generate(ast::Node *n);

		Function *findBestOverload(const QList<Function*> &functions, const QList<Value> &parameters, bool command, const CodePoint &cp);
		QList<Value> generateParameterList(ast::Node *n);
		void resolveGotos();

		bool generateAllocas();
		void generateDestructors();

		llvm::BasicBlock *createBasicBlock(const llvm::Twine &name = llvm::Twine(), llvm::BasicBlock *insertBefore = 0);

		bool checkUnreachable(CodePoint cp);

		Settings *mSettings;
		Scope *mLocalScope;
		Scope *mGlobalScope;
		Runtime *mRuntime;
		Builder *mBuilder;
		llvm::Function *mFunction;
		ConstantExpressionEvaluator mConstEval;
		ValueType *mReturnType;
		bool mMainFunction;
		bool mUnreachableBasicBlock;

		QList<QPair<LabelSymbol*, llvm::BasicBlock*> > mUnresolvedGotos;
		QStack<llvm::BasicBlock*> mExitStack;

		bool mValid;
	signals:
		void warning(int code, QString msg, CodePoint codePoint);
		void error(int code, QString msg, CodePoint codePoint);
	private slots:
		void errorOccured(int, QString, CodePoint);
};

#endif // FUNCTIONCODEGENERATOR_H
