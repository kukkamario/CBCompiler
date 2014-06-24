#ifndef CONSTANTEXPRESSIONEVALUATOR_H
#define CONSTANTEXPRESSIONEVALUATOR_H

#include <QObject>
#include "constantvalue.h"
#include "abstractsyntaxtree.h"
class Scope;
class ConstantExpressionEvaluator : public QObject {
		Q_OBJECT
	public:
		explicit ConstantExpressionEvaluator(QObject *parent = 0);

		Scope *scope() const { return mScope; }
		void setScope(Scope *s) { mScope = s; }

		ConstantValue evaluate(ast::Node *node);
		ConstantValue evaluate(ast::Identifier *node);
		ConstantValue evaluate(ast::Variable *node);
		ConstantValue evaluate(ast::Integer *node);
		ConstantValue evaluate(ast::Float *node);
		ConstantValue evaluate(ast::String *node);
		ConstantValue evaluate(ast::Expression *node);
		ConstantValue evaluate(ast::Unary *node);
	signals:
		void warning(int code, QString msg, CodePoint codePoint);
		void error(int code, QString msg, CodePoint codePoint);
	public slots:

	private:
		Scope *mScope;
};

#endif // CONSTANTEXPRESSIONEVALUATOR_H
