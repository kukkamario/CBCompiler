#include "constantexpressionevaluator.h"
#include "errorcodes.h"
#include "runtime.h"
#include "valuetype.h"
#include "intvaluetype.h"
#include "floatvaluetype.h"
#include "stringvaluetype.h"
#include "bytevaluetype.h"
#include "shortvaluetype.h"


ConstantExpressionEvaluator::ConstantExpressionEvaluator() :
	mGlobalScope(0),
	mRuntime(0) {
}

ConstantValue ConstantExpressionEvaluator::evaluate(const ast::Node *s) {
	switch(s->type()) {
		case ast::Node::ntInteger:
			return evaluate((ast::Integer*)s);
		case ast::Node::ntFloat:
			return evaluate((ast::Float*)s);
		case ast::Node::ntString:
			return evaluate((ast::String*)s);
		case ast::Node::ntVariable:
			return evaluate((ast::Variable*)s);
		case ast::Node::ntExpression:
			return evaluate((ast::Expression*)s);
		case ast::Node::ntUnary:
			return evaluate((ast::Integer*)s);

		//Errors
		case ast::Node::ntFunctionCall:
		case ast::Node::ntArraySubscript:
			emit error(ErrorCodes::ecNotConstant, tr("Neither function call nor array subscript is allowed inside constant expression"), s->codePoint());
			return ConstantValue();
		default:
			emit error(ErrorCodes::ecWTF, tr("Unknown AST node, ConstantExpressionEvaluator::evaluate"), s->codePoint());
			return ConstantValue();
	}
}

ConstantValue ConstantExpressionEvaluator::evaluate(const ast::Expression *s) {
	ast::ChildNodeIterator i = s->childNodesBegin();
	ConstantValue first = evaluate(*i);
	//ast::Expression::Associativity associativity = s->associativity();
	//FIXME: Associativity? (LeftToRight is only valid currently)

	if (!first.isValid()) return ConstantValue();
	for (++i ; i != s->childNodesEnd(); ++i) {
		ast::ExpressionNode *exprNode = static_cast<ast::ExpressionNode*>(*i);
		ConstantValue second = evaluate(exprNode->operand());
		if (!second.isValid()) return ConstantValue();
		ConstantValue result;
		switch (exprNode->op()) {
			case ast::ExpressionNode::opAssign:
				emit error(ErrorCodes::ecNotConstant, tr("Assign isn't allowed in a constant expression"), exprNode->codePoint());
			case ast::ExpressionNode::opEqual:
				result = ConstantValue::equal(first, second); break;
			case ast::ExpressionNode::opNotEqual:
				result = ConstantValue::notEqual(first, second); break;
			case ast::ExpressionNode::opGreater:
				result = ConstantValue::greater(first, second); break;
			case ast::ExpressionNode::opLess:
				result = ConstantValue::less(first, second); break;
			case ast::ExpressionNode::opGreaterEqual:
				result = ConstantValue::greaterEqual(first, second); break;
			case ast::ExpressionNode::opLessEqual:
				result = ConstantValue::lessEqual(first, second); break;
			case ast::ExpressionNode::opAdd:
				result = ConstantValue::add(first, second); break;
			case ast::ExpressionNode::opSubtract:
				result = ConstantValue::subtract(first, second); break;
			case ast::ExpressionNode::opMultiply:
				result = ConstantValue::multiply(first, second); break;
			case ast::ExpressionNode::opDivide:
				result = ConstantValue::divide(first, second); break;
			case ast::ExpressionNode::opPower:
				result = ConstantValue::power(first, second); break;
			case ast::ExpressionNode::opMod:
				result = ConstantValue::mod(first, second); break;
			case ast::ExpressionNode::opShl:
				result = ConstantValue::shl(first, second); break;
			case ast::ExpressionNode::opShr:
				result = ConstantValue::shr(first, second); break;
			case ast::ExpressionNode::opSar:
				result = ConstantValue::sar(first, second); break;
			case ast::ExpressionNode::opAnd:
				result = ConstantValue::and_(first, second); break;
			case ast::ExpressionNode::opOr:
				result = ConstantValue::or_(first, second); break;
			case ast::ExpressionNode::opXor:
				result = ConstantValue::xor_(first, second); break;
			default:
				emit error(ErrorCodes::ecWTF, tr("Unknown expression operator, ConstantExpressionEvaluator::evaluate"), exprNode->codePoint());
				return ConstantValue();
		}
		if (!result.isValid()) {
			emit error(ErrorCodes::ecMathematicalOperationOperandTypeMismatch,
					   tr("Mathematical operation operand type mismatch: No operation \"%1\" between %2 and %3").arg(ast::ExpressionNode::opToString(exprNode->op()), first.typeName(), second.typeName()), exprNode->codePoint());
			return ConstantValue();
		}
		first = result;
	}
	return first;
}

ConstantValue ConstantExpressionEvaluator::evaluate(const ast::Unary *s) {
	ConstantValue operand = evaluate(s->mOperand);
	if (!operand.isValid()) return ConstantValue();
	switch (s->op()) {
		case ast::Unary::opPositive:
			return ConstantValue::plus(operand);
		case ast::Unary::opNegative:
			return ConstantValue::minus(operand);
		case ast::Unary::opNot:
			return ConstantValue::not_(operand);
		default:
			assert(0 && "Invalid ast::Unary::Op");
			return ConstantValue();
	}
}

ConstantValue ConstantExpressionEvaluator::evaluate(const ast::String *s) {
	return s->value();
}

ConstantValue ConstantExpressionEvaluator::evaluate(const ast::Integer *s) {
	return s->value();
}

ConstantValue ConstantExpressionEvaluator::evaluate(const ast::Float *s) {
	return s->value();
}

ConstantValue ConstantExpressionEvaluator::evaluate(const ast::Variable *s) {

	Symbol *sym = mGlobalScope->find(s->identifier()->name());
	if (sym->type() != Symbol::stConstant) {
		emit error(ErrorCodes::ecNotConstant, tr("\"%1\" is not constant").arg(s->identifier()->name()), s->codePoint());
		return ConstantValue();
	}
	if (checkConstantType(static_cast<ConstantSymbol*>(sym), s->valueType())) {
		return v;
	}
	else {
		return ConstantValue();
	}
	return v;
}

void ConstantExpressionEvaluator::setRuntime(Runtime *runtime) {
	mRuntime = runtime;
}

void ConstantExpressionEvaluator::setGlobalScope(Scope *globalScope) {
	mGlobalScope = globalScope;
}



void ConstantExpressionEvaluator::errorConstantAlreadyDefinedWithAnotherType(const QString &name) {
	emit error(ErrorCodes::ecVarAlreadyDefinedWithAnotherType, tr("Constant \"%1\" already defined with another type").arg(name), mLine, mFile);
}

bool ConstantExpressionEvaluator::checkConstantType(ConstantSymbol *sym, ast::Node *givenType) {
	const ConstantValue &v = sym->value();
	if (givenType->type() == ast::Node::ntDefaultType) {
		return true;
	}
	if (givenType->type() == ast::Node::ntBasicType) {
		ast::BasicType *bt = static_cast<ast::BasicType*>(givenType);
		switch(bt->valueType()) {
			case ast::BasicType::Integer:
				if (v.type() != ConstantValue::Integer) {
					errorConstantAlreadyDefinedWithAnotherType(s->identifier()->name(), sym->codePoint(), s->codePoint());
					return false;
				}
				break;
			case ast::BasicType::Float:
				if (v.type() != ConstantValue::Float) {
					errorConstantAlreadyDefinedWithAnotherType(s->identifier()->name(), sym->codePoint(), s->codePoint());
					return false;
				}
				break;
			case ast::BasicType::String:
				if (v.type() != ConstantValue::String) {
					errorConstantAlreadyDefinedWithAnotherType(s->identifier()->name(), sym->codePoint(), s->codePoint());
					return false;
				}
				break;
			default:
				assert(0 && "Invalid ast::BasicType");
		}
		return true;
	}
	emit error(ErrorCodes::ecVarAlreadyDefinedWithAnotherType, tr("Please, don't use \"As\" with constants... FIXME"));
	return false;
}
