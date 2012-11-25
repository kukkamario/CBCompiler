#ifndef CODEGENERATOR_H
#define CODEGENERATOR_H
#include "scope.h"
#include "abstractsyntaxtree.h"
#include "runtime.h"
#include "stringpool.h"
#include "constantvalue.h"
#include "constantexpressionevaluator.h"
#include "scope.h"
class TypeSymbol;
class CodeGenerator : public QObject{
		Q_OBJECT
	public:
		CodeGenerator();
		bool initialize(const QString &runtimeFile);
		bool generate(ast::Program *program);
		ValueType *valueTypeEnum(ValueType::Type t);
	private:
		bool evaluateConstants(ast::Program *program);
		bool addGlobalsToScope(ast::Program *program);
		bool addTypesToScope(ast::Program *program);
		ValueType::Type valueTypeFromVarType(ast::Variable::VarType t);

		QMap<ValueType::Type, ValueType *> mValueTypeEnum;
		Runtime mRuntime;
		StringPool mStringPool;
		QMap<QString, ConstantValue> mConstants;
		ConstantExpressionEvaluator mConstEval;
		QList<ValueType*> mValueTypes;
		Scope mGlobalScope;
		Scope mMainScope;

		TypeSymbol *findTypeSymbol(const QString &typeName, QFile *f, int line);

	signals:
		void error(int code, QString msg, int line, QFile *file);
		void warning(int code, QString msg, int line, QFile *file);
};

#endif // CODEGENERATOR_H
