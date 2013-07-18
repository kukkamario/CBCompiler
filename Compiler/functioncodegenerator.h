#ifndef FUNCTIONCODEGENERATOR_H
#define FUNCTIONCODEGENERATOR_H
#include <QObject>
#include <QVector>
#include <QStack>

#include "llvm.h"
#include "scope.h"
#include "abstractsyntaxtree.h"
#include "expressioncodegenerator.h"
#include "builder.h"
class QFile;
class CBFunction;
/**
 * @brief The FunctionCodeGenerator class generates llvm-ir for a function. It expects AST and Scope to valid.
 */
class FunctionCodeGenerator: public QObject{
		Q_OBJECT
	public:
		explicit FunctionCodeGenerator(QObject *parent = 0);
		/**
		 * @brief Sets the runtime used for the generation.
		 * @param r Runtime
		 */
		void setRuntime(Runtime *r);

		/**
		 * @brief setFunction
		 * @param func The function in which generateFunctionCode generates the llvm-ir.
		 */
		void setFunction(llvm::Function *func);
		void setScope(Scope *scope);
		void setSetupBasicBlock(llvm::BasicBlock *bb);
		void setBuilder(Builder *builder);

		void generateCBFunction(ast::FunctionDefinition *func, CBFunction *cbFunc);

		/**
		 * @brief generateMainScope generates llvm-IR code for a AST tree.
		 *
		 * llvm::Function where llvm-IR is inserted have to be set using setFunction. The scope should be specificated with setScope.
		 * @param n The block that contains AST.
		 */
		void generateMainScope(ast::Block *n);
		Scope *scope() const {return mScope;}
		llvm::Function *function()const{return mFunction;}
		Runtime *runtime() const {return mRuntime;}
	private:
		void generate(ast::Node *n);
		void generate(ast::IfStatement *n);
		void generate(ast::AssignmentExpression *n);
		void generate(ast::CommandCall *n);
		void generate(ast::RepeatForeverStatement *n);
		void generate(ast::FunctionCallOrArraySubscript *n);
		void generate(ast::Exit *n);
		void generate(ast::Return *n);
		void generate(ast::Block *n);
		void generate(ast::ArrayDefinition *n);
		void generate(ast::ArraySubscriptAssignmentExpression *n);
		void generate(ast::SelectStatement *n);
		void generate(ast::ForEachStatement *n);
		void generate(ast::ForToStatement *n);
		void generate(ast::Goto *n);
		void generate(ast::Gosub *n);
		void generate(ast::RepeatUntilStatement *n);
		void generate(ast::WhileStatement *n);
		void generate(ast::VariableDefinition *n);
		void generate(ast::Redim *n);
		void generate(ast::Label *n);
		void generate(ast::SpecialFunctionCall *n);

		void basicBlockGenerationPass(ast::Block *n);
		void basicBlockGenerationPass(ast::IfStatement *n);
		void basicBlockGenerationPass(ast::ForToStatement *n);
		void basicBlockGenerationPass(ast::ForEachStatement *n);
		void basicBlockGenerationPass(ast::RepeatUntilStatement *n);
		void basicBlockGenerationPass(ast::RepeatForeverStatement *n);
		void basicBlockGenerationPass(ast::Label *n);
		void basicBlockGenerationPass(ast::WhileStatement *n);
		void basicBlockGenerationPass(ast::SelectStatement *n);

		void addBasicBlock();

		void generateLocalVariables();
		void generateDestructors();

		bool isCurrentBBEmpty() const { return mCurrentBasicBlock->getInstList().empty(); }
		void nextBasicBlock();
		void pushExit(const QVector<llvm::BasicBlock*>::ConstIterator &i) { mExitStack.push(i);}
		void pushExit(ast::Node *n) { const QVector<llvm::BasicBlock*>::ConstIterator i = mExitLocations[n]; assert(i != mBasicBlocks.end()); pushExit(i); }
		void popExit() { mExitStack.pop(); }
		llvm::BasicBlock *currentExit() const { assert(!mExitStack.isEmpty()); return *mExitStack.top(); }

		Scope *mScope;
		Builder *mBuilder;
		llvm::Function *mFunction;
		llvm::BasicBlock *mSetupBasicBlock;
		QVector<llvm::BasicBlock*> mBasicBlocks;
		QVector<llvm::BasicBlock*>::ConstIterator mCurrentBasicBlockIt;
		llvm::BasicBlock *mCurrentBasicBlock;
		QMap<ast::Node*, QVector<llvm::BasicBlock*>::ConstIterator> mExitLocations;
		QStack<QVector<llvm::BasicBlock*>::ConstIterator> mExitStack;
		ExpressionCodeGenerator mExprGen;
		Runtime *mRuntime;
		ValueType *mReturnType;
	public slots:

	signals:
		void error(int code, QString msg, int line, QFile *file);
		void warning(int code, QString msg, int line, QFile *file);
};

#endif // FUNCTIONCODEGENERATOR_H
