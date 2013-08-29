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
		void setCodeFile(const QString &f);
		void setCodeLine(int l);
	private:
		Runtime *mRuntime;
		Scope* mGlobalScope;
		QString mFile;
		int mLine;
		void errorConstantAlreadyDefinedWithAnotherType(const QString &name);
	signals:
		void error(int code, QString msg, int line, const QString &file);
		void warning(int code, QString msg, int line, const QString &file);
};

#endif // CONSTANTEXPRESSIONEVALUATOR_H
