#ifndef CONSTANTEXPRESSIONEVALUATOR_H
#define CONSTANTEXPRESSIONEVALUATOR_H

#include <QObject>
#include "constantvalue.h"
#include "abstractsyntaxtree.h"
#include "errorhandler.h"
class Scope;
class ConstantExpressionEvaluator {
	public:
		explicit ConstantExpressionEvaluator(ErrorHandler *errorHandler);

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


	private:
		void warning(int code, std::string msg, CodePoint codePoint);
		void error(int code, std::string msg, CodePoint codePoint);

		Scope *mScope;
		ErrorHandler *mErrorHandler;
};

#endif // CONSTANTEXPRESSIONEVALUATOR_H
