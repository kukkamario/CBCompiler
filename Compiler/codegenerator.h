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
		bool initialize(const QString &runtimeFile, const Settings &settings);
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
		bool generateFunctions();
		bool generateMainScope(ast::Block *block);
		void createBuilder();

		void addPredefinedConstantSymbols();

		TypeSymbol *findTypeSymbol(const QString &typeName, QFile *f, int line);

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
	signals:
		void error(int code, QString msg, int line, QFile *file);
		void warning(int code, QString msg, int line, QFile *file);
};

#endif // CODEGENERATOR_H
