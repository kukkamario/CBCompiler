#include "constantexpressionevaluator.h"
#include "errorcodes.h"
#include "warningcodes.h"
#include "scope.h"
#include "constantsymbol.h"

ConstantExpressionEvaluator::ConstantExpressionEvaluator(QObject *parent) :
	QObject(parent)
{
}

ConstantValue ConstantExpressionEvaluator::evaluate(ast::Node *n) {
	switch (n->type()) {
		case ast::Node::ntInteger:
			return evaluate(n->cast<ast::Integer>());
		case ast::Node::ntFloat:
			return evaluate(n->cast<ast::Float>());
		case ast::Node::ntString:
			return evaluate(n->cast<ast::String>());
		case ast::Node::ntIdentifier:
			return evaluate(n->cast<ast::Identifier>());

		case ast::Node::ntExpression:
			return evaluate(n->cast<ast::Expression>());
		case ast::Node::ntUnary:
			return evaluate(n->cast<ast::Unary>());
		case ast::Node::ntFunctionCall:
			return evaluate(n->cast<ast::FunctionCall>());
		case ast::Node::ntVariable:
			return evaluate(n->cast<ast::Variable>());
		default:
			emit error(ErrorCodes::ecNotConstant, tr("This expression isn't constant expression"), n->codePoint());
			return ConstantValue();
	}
}

ConstantValue ConstantExpressionEvaluator::evaluate(ast::Expression *node) {
	ConstantValue op1 = evaluate(node->firstOperand());
	if (!op1.isValid()) return ConstantValue();

	for (ast::ExpressionNode *operation : node->operations()) {
		ConstantValue op2 = evaluate(operation->operand());
		if (!op2.isValid()) return ConstantValue();
		ConstantValue result;
		OperationFlags flags;
		switch (operation->op()) {
			case ast::ExpressionNode::opAdd:
				result = ConstantValue::add(op1, op2, flags); break;
			case ast::ExpressionNode::opSubtract:
				result = ConstantValue::subtract(op1, op2, flags); break;
			case ast::ExpressionNode::opMultiply:
				result = ConstantValue::multiply(op1, op2, flags); break;
			case ast::ExpressionNode::opDivide:
				result = ConstantValue::add(op1, op2, flags); break;
			case ast::ExpressionNode::opMod:
				result = ConstantValue::mod(op1, op2, flags); break;
			case ast::ExpressionNode::opPower:
				result = ConstantValue::power(op1, op2, flags); break;
			case ast::ExpressionNode::opShl:
				result = ConstantValue::shl(op1, op2, flags); break;
			case ast::ExpressionNode::opSar:
				result = ConstantValue::sar(op1, op2, flags); break;
			case ast::ExpressionNode::opShr:
				result = ConstantValue::shr(op1, op2, flags); break;
			case ast::ExpressionNode::opXor:
				result = ConstantValue::xor_(op1, op2, flags); break;
			case ast::ExpressionNode::opOr:
				result = ConstantValue::or_(op1, op2, flags); break;
			case ast::ExpressionNode::opAnd:
				result = ConstantValue::and_(op1, op2, flags); break;
			case ast::ExpressionNode::opEqual:
				result = ConstantValue::equal(op1, op2, flags); break;
			case ast::ExpressionNode::opNotEqual:
				result = ConstantValue::notEqual(op1, op2, flags); break;
			case ast::ExpressionNode::opGreater:
				result = ConstantValue::greater(op1, op2, flags); break;
			case ast::ExpressionNode::opGreaterEqual:
				result = ConstantValue::greaterEqual(op1, op2, flags); break;
			case ast::ExpressionNode::opLess:
				result = ConstantValue::less(op1, op2, flags); break;
			case ast::ExpressionNode::opLessEqual:
				result = ConstantValue::lessEqual(op1, op2, flags); break;
			default:
				emit error(ErrorCodes::ecOperationNotAllowedInConstantExpression, tr("Operation %1 is not allowed in constant").arg(ast::ExpressionNode::opToString(operation->op())), operation->codePoint());
				return ConstantValue();
		}
		if (flags.testFlag(OperationFlag::IntegerDividedByZero)) {
			emit error(ErrorCodes::ecIntegerDividedByZero, tr("Integer divided by Zero"), operation->codePoint());
		}
		if (flags.testFlag(OperationFlag::MayLosePrecision)) {
			emit warning(WarningCodes::wcMayLosePrecision, tr("Values may lose precision during operation"), operation->codePoint());
		}
		if (flags.testFlag(OperationFlag::NoSuchOperation)) {
			emit error(ErrorCodes::ecMathematicalOperationOperandTypeMismatch, tr("No operator %1 between operands %2 and %3").arg(ast::ExpressionNode::opToString(operation->op()), op1.valueInfo(), op2.valueInfo()), operation->codePoint());
		}
		if (operationFlagsContainFatalFlags(flags)) {
			return ConstantValue();
		}
		op1 = result;
	}
	return op1;
}

ConstantValue ConstantExpressionEvaluator::evaluate(ast::Unary *node) {
	ConstantValue value = evaluate(node->operand());
	if (!value.isValid()) return ConstantValue();
	ConstantValue result;
	OperationFlags flags;
	switch(node->op()) {
		case ast::Unary::opNegative:
			result = ConstantValue::minus(value, flags); break;
		case ast::Unary::opPositive:
			result = ConstantValue::plus(value, flags); break;
		case ast::Unary::opNot:
			result = ConstantValue::not_(value, flags); break;
		default:
			assert("Invalid ast::Unary::Op" && 0);
	}
	if (flags.testFlag(OperationFlag::MayLosePrecision)) {
		emit warning(WarningCodes::wcMayLosePrecision, tr("Values may lose precision during operation"), node->codePoint());
	}
	if (flags.testFlag(OperationFlag::NoSuchOperation)) {
		emit error(ErrorCodes::ecMathematicalOperationOperandTypeMismatch, tr("Invalid operation %1 for value %2").arg(ast::Unary::opToString(node->op()), result.valueInfo()), node->codePoint());
	}
	if (operationFlagsContainFatalFlags(flags)) {
		return ConstantValue();
	}

	return result;
}

ConstantValue ConstantExpressionEvaluator::evaluate(ast::Identifier *node) {
	Symbol *sym = mScope->find(node->name());
	assert(sym);
	if (sym->type() != Symbol::stConstant) {
		emit error(ErrorCodes::ecNotConstant, tr("Symbol \"%1\" isn't a constant").arg(node->name()), node->codePoint());
		return ConstantValue();
	}

	ConstantSymbol *constant = static_cast<ConstantSymbol*>(sym);
	return constant->value();
}

ConstantValue ConstantExpressionEvaluator::evaluate(ast::Variable *node) {
	return evaluate(node->identifier());
}

ConstantValue ConstantExpressionEvaluator::evaluate(ast::Integer *node) {
	return node->value();
}

ConstantValue ConstantExpressionEvaluator::evaluate(ast::Float *node) {
	return node->value();
}

ConstantValue ConstantExpressionEvaluator::evaluate(ast::String *node) {
	return node->value();
}
