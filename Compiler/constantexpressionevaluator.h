#ifndef CONSTANTEXPRESSIONEVALUATOR_H
#define CONSTANTEXPRESSIONEVALUATOR_H
#include "stringpool.h"
#include "value.h"
#include "constantvalue.h"
#include "valuetype.h"
#include "abstractsyntaxtree.h"
#include "scope.h"
#include "constantsymbol.h"

class ConstantExpressionEvaluator : public QObject {
		Q_OBJECT
	public:
		ConstantExpressionEvaluator();
		ConstantValue evaluate(const ast::Node *s);
		ConstantValue evaluate(const ast::Expression *s);
		ConstantValue evaluate(const ast::Unary *s);
		ConstantValue evaluate(const ast::String *s);
		ConstantValue evaluate(const ast::Integer *s);
		ConstantValue evaluate(const ast::Float *s);
		ConstantValue evaluate(const ast::Variable *s);

		void setRuntime(Runtime *runtime);
		void setGlobalScope(Scope *globalScope);
	private:
		Runtime *mRuntime;
		Scope* mGlobalScope;
		void errorConstantAlreadyDefinedWithAnotherType(const QString &name, const CodePoint &firstDef, const CodePoint &secondDef);
		bool checkConstantType(ConstantSymbol *symbol, ast::Node *givenType);
	signals:
		void error(int code, QString msg, const CodePoint &cp);
		void warning(int code, QString msg, const CodePoint &cp);
};

#endif // CONSTANTEXPRESSIONEVALUATOR_H
