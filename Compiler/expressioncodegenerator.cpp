#include "expressioncodegenerator.h"
#include "builder.h"
#include "scope.h"
ExpressionCodeGenerator::ExpressionCodeGenerator(QObject *parent):
	QObject(parent),
	mBuilder(0),
	mScope(0)
{
}

void ExpressionCodeGenerator::setBuilder(Builder *b) {
	mBuilder = b;
}

void ExpressionCodeGenerator::setScope(Scope *s) {
	mScope = s;
}

Value ExpressionCodeGenerator::generate(ast::Node *n) {
}

Value ExpressionCodeGenerator::generate(ast::Unary *n) {
	Value val = generate(n->mOperand);
	if (val.isConstant()) {
		switch (n->mOperator) {
			case ast::opMinus:
				return ConstantValue::minus(val.constant());
			case ast::opNot:
				return ConstantValue::not_(val.constant());
			case ast::opPlus:
				return ConstantValue::plus(val.constant());
			default:
				assert(0);
		}
	}


}

Value ExpressionCodeGenerator::generate(ast::FunctionCallOrArraySubscript *n) {
}

Value ExpressionCodeGenerator::generate(ast::Float *n) {
	return Value(ConstantValue(n->mValue));
}

Value ExpressionCodeGenerator::generate(ast::Integer *n) {
	return Value(ConstantValue(n->mValue));
}

Value ExpressionCodeGenerator::generate(ast::String *n) {
	return Value(ConstantValue(n->mValue));
}
