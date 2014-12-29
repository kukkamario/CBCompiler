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
#include "errorhandler.h"

class CBFunction;
class TypeSymbol;
class CodeGenerator {
	public:
		CodeGenerator(ErrorHandler *errorHandler);
		bool initialize(Settings *settings);
		bool generate(ast::Program *program);
		bool createExecutable(const std::string &path);
	private:
		bool addRuntimeFunctions();
		bool generateFunctions(const std::vector<ast::FunctionDefinition*> &functions);
		bool checkMainScope(ast::Program *program);
		bool checkFunctions();
		bool calculateConstants(ast::Program *program);
		bool generateGlobalVariables();
		bool generateFunctionDefinitions(const std::vector<ast::FunctionDefinition*> &functions);
		bool generateMainScope(ast::Block *block);
		void generateInitializers();
		void generateStringLiterals();
		void generateTypeInitializers();
		void createBuilder();

		void addPredefinedConstantSymbols();

		bool generateTypesAndStructes(ast::Program *program);


		Settings *mSettings;
		Runtime mRuntime;
		StringPool mStringPool;
		SymbolCollector mSymbolCollector;
		Scope mGlobalScope;
		Scope mMainScope;
		FunctionCodeGenerator mFuncCodeGen;
		std::map<ast::FunctionDefinition *, CBFunction *> mCBFunctions;
		Builder *mBuilder;
		ErrorHandler *mErrorHandler;

		llvm::BasicBlock *mInitializationBlock;
	signals:
		void error(int code, std::string msg, CodePoint cp);
		void warning(int code, std::string msg, CodePoint cp);
};

#endif // CODEGENERATOR_H
