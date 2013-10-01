#ifndef CODEGENERATOR_H
#define CODEGENERATOR_H
#include "scope.h"
#include "abstractsyntaxtree.h"
#include "runtime.h"
#include "stringpool.h"
#include "constantvalue.h"
#include "constantexpressionevaluator.h"
#include "symbolcollectortypechecker.h"
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
		bool addFunctions(const QList<ast::FunctionDefinition*> &functions);
		bool checkMainScope(ast::Program *program);
		bool checkFunctions();
		bool calculateConstants(ast::Program *program);
		bool addGlobalsToScope(ast::Program *program);
		bool addTypesToScope(ast::Program *program);
		void generateFunctions();
		void generateMainScope(ast::Block *block);
		void generateInitializers();
		void generateStringLiterals();
		void generateTypeInitializers();
		void createBuilder();
		void addValueTypesToGlobalScope();

		void addPredefinedConstantSymbols();

		ValueType *findValueType(const QString &valueTypeName, int line = 0, QString file = QString());

		TypeSymbol *findTypeSymbol(const QString &typeName, const QString &f, int line);

		Settings mSettings;
		Runtime mRuntime;
		StringPool mStringPool;
		ConstantExpressionEvaluator mConstEval;
		SymbolCollectorTypeChecker mTypeChecker;
		QList<ValueType*> mValueTypes;
		Scope mGlobalScope;
		Scope mMainScope;
		FunctionCodeGenerator mFuncCodeGen;
		QMap<ast::FunctionDefinition *, CBFunction *> mCBFunctions;
		Builder *mBuilder;
		QList<TypeSymbol*> mTypes;

		llvm::BasicBlock *mInitializationBlock;
	signals:
		void error(int code, QString msg, int line, const QString &file);
		void warning(int code, QString msg, int line, const QString &file);
};

#endif // CODEGENERATOR_H
