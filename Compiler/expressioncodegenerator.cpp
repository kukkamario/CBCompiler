#include "expressioncodegenerator.h"
#include "builder.h"
#include "scope.h"
#include "constantsymbol.h"
#include "errorcodes.h"
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
	switch(n->type()) {
		case ast::Node::ntExpression:
			return generate((ast::Expression*)n);
		case ast::Node::ntFloat:
			return generate((ast::Float*)n);
		case ast::Node::ntFunctionCallOrArraySubscript:
			return generate((ast::FunctionCallOrArraySubscript*)n);
		case ast::Node::ntInteger:
			return generate((ast::Integer*)n);
		case ast::Node::ntNew:
			return generate((ast::New*)n);
		case ast::Node::ntString:
			return generate((ast::String*)n);
		case ast::Node::ntTypePtrField:
			return generate((ast::TypePtrField*)n);
		case ast::Node::ntUnary:
			return generate((ast::Unary*)n);
		case ast::Node::ntVariable:
			return generate((ast::Variable*)n);
	}
	assert(0);
	return Value();
}

Value ExpressionCodeGenerator::generate(ast::New *n) {
	assert(0);
	return Value();
}

Value ExpressionCodeGenerator::generate(ast::Unary *n) {
	Value val = generate(n->mOperand);
	switch (n->mOperator) {
		case ast::opMinus:
			return mBuilder->minus(Value(val.constant(), mBuilder->runtime()));
		case ast::opNot:
			return mBuilder->not_(Value(val.constant(), mBuilder->runtime()));
		case ast::opPlus:
			return mBuilder->plus(Value(val.constant(), mBuilder->runtime()));
		default:
			assert(0);
			return Value();
	}


}

Value ExpressionCodeGenerator::generate(ast::Variable *n) {
	Symbol *sym = mScope->find(n->mName);
	assert(sym->type() == Symbol::stVariable || sym->type() == Symbol::stConstant);
	if (sym->type() == Symbol::stConstant) {
		ConstantSymbol *c = static_cast<ConstantSymbol*>(sym);
		return Value(c->value(), mBuilder->runtime());
	}
	else {
		VariableSymbol *var = static_cast<VariableSymbol*>(sym);
		return mBuilder->load(var);
	}
}

Value ExpressionCodeGenerator::generate(ast::TypePtrField *n) {
	assert(0);
	return Value();
}

Value ExpressionCodeGenerator::generate(ast::FunctionCallOrArraySubscript *n) {
	assert(0);
	return Value();
}

Value ExpressionCodeGenerator::generate(ast::Float *n) {
	return Value(ConstantValue(n->mValue), mBuilder->runtime());
}

Value ExpressionCodeGenerator::generate(ast::Integer *n) {
	return Value(ConstantValue(n->mValue), mBuilder->runtime());
}

Value ExpressionCodeGenerator::generate(ast::String *n) {
	return Value(ConstantValue(n->mValue), mBuilder->runtime());
}

Value ExpressionCodeGenerator::generate(ast::Expression *n) {
	Value first = generate(n->mFirst);
	if (!first.isValid()) return Value();


	for (QList<ast::Operation>::ConstIterator i = n->mRest.begin(); i != n->mRest.end(); i++) {
		Value second = generate(i->mOperand);
		if (!second.isValid()) return Value();
		Value result;
		switch (i->mOperator) {
			case ast::opEqual:
				result = mBuilder->equal(first, second); break;
			case ast::opNotEqual:
				result = mBuilder->notEqual(first, second); break;
			case ast::opGreater:
				result = mBuilder->greater(first, second); break;
			case ast::opLess:
				result = mBuilder->less(first, second); break;
			case ast::opGreaterEqual:
				result = mBuilder->greaterEqual(first, second); break;
			case ast::opLessEqual:
				result = mBuilder->lessEqual(first, second); break;
			case ast::opPlus:
				result = mBuilder->add(first, second); break;
			case ast::opMinus:
				result = mBuilder->subtract(first, second); break;
			case ast::opMultiply:
				result = mBuilder->multiply(first, second); break;
			case ast::opDivide:
				result = mBuilder->divide(first, second); break;
			case ast::opPower:
				result = mBuilder->power(first, second); break;
			case ast::opMod:
				result = mBuilder->mod(first, second); break;
			case ast::opShl:
				result = mBuilder->shl(first, second); break;
			case ast::opShr:
				result = mBuilder->shr(first, second); break;
			case ast::opSar:
				result = mBuilder->sar(first, second); break;
			case ast::opAnd:
				result = mBuilder->and_(first, second); break;
			case ast::opOr:
				result = mBuilder->or_(first, second); break;
			case ast::opXor:
				result = mBuilder->xor_(first, second); break;
			default:
				assert(0);
				return Value();
		}
		assert(result.isValid());
		first = result;
	}
	return first;
}