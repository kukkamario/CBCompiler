#ifndef CODEGENERATOR_H
#define CODEGENERATOR_H
#include "scope.h"
#include "abstractsyntaxtree.h"
#include "runtime.h"
#include "stringpool.h"
#include "constantvalue.h"
#include "symbolcollector.h"
#include "functioncodegenerator.h"
#include "settings.h"
class CBFunction;
class TypeSymbol;
class CodeGenerator : public QObject{
		Q_OBJECT
	public:
		CodeGenerator(QObject *parent = 0);
		bool initialize(const Settings &settings);
		bool generate(ast::Program *program);
		bool createExecutable(const QString &path);
	private:
		bool addRuntimeFunctions();
		bool generateFunctions(const QList<ast::FunctionDefinition*> &functions);
		bool checkMainScope(ast::Program *program);
		bool checkFunctions();
		bool calculateConstants(ast::Program *program);
		bool generateGlobalVariables();
		bool generateFunctionDefinitions(const QList<ast::FunctionDefinition*> &functions);
		bool generateMainScope(ast::Block *block);
		void generateInitializers();
		void generateStringLiterals();
		void generateTypeInitializers();
		void createBuilder();

		void addPredefinedConstantSymbols();

		bool generateTypesAndStructs(ast::Program *program);

		Settings mSettings;
		Runtime mRuntime;
		StringPool mStringPool;
		SymbolCollector mSymbolCollector;
		Scope mGlobalScope;
		Scope mMainScope;
		FunctionCodeGenerator mFuncCodeGen;
		QMap<ast::FunctionDefinition *, CBFunction *> mCBFunctions;
		Builder *mBuilder;

		llvm::BasicBlock *mInitializationBlock;
	signals:
		void error(int code, QString msg, CodePoint cp);
		void warning(int code, QString msg, CodePoint cp);
};

#endif // CODEGENERATOR_H
