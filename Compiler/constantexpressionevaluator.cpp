#include "constantexpressionevaluator.h"
#include "errorcodes.h"
ConstantExpressionEvaluator::ConstantExpressionEvaluator() {
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
		case ast::Node::ntFunctionCallOrArraySubscript:
			emit error(ErrorCodes::ecNotConstant, tr("Neither function call nor array subscript is allowed inside constant expression"), mLine, mFile);
			return ConstantValue();
		case ast::Node::ntNew:
			emit error(ErrorCodes::ecNotConstant, tr("\"New\" isn't allowed in constant expression"), mLine, mFile);
			return ConstantValue();
		default:
			emit error(ErrorCodes::ecWTF, tr("Unknown AST node, ConstantExpressionEvaluator::evaluate"), mLine, mFile);
			return ConstantValue();
	}
}

ConstantValue ConstantExpressionEvaluator::evaluate(const ast::Expression *s) {
	ConstantValue first = evaluate(s->mFirst);
	if (!first.isValid()) return ConstantValue();

	for (QList<ast::Operation>::ConstIterator i = s->mRest.begin(); i != s->mRest.end(); i++) {
		ConstantValue second = evaluate(i->mOperand);
		if (!second.isValid()) return ConstantValue();
		ConstantValue result;
		switch (i->mOperator) {
			case ast::opEqual:
				result = ConstantValue::equal(first, second); break;
			case ast::opNotEqual:
				result = ConstantValue::notEqual(first, second); break;
			case ast::opGreater:
				result = ConstantValue::greater(first, second); break;
			case ast::opLess:
				result = ConstantValue::less(first, second); break;
			case ast::opGreaterEqual:
				result = ConstantValue::greaterEqual(first, second); break;
			case ast::opLessEqual:
				result = ConstantValue::lessEqual(first, second); break;
			case ast::opPlus:
				result = ConstantValue::add(first, second); break;
			case ast::opMinus:
				result = ConstantValue::subtract(first, second); break;
			case ast::opMultiply:
				result = ConstantValue::multiply(first, second); break;
			case ast::opDivide:
				result = ConstantValue::divide(first, second); break;
			case ast::opPower:
				result = ConstantValue::power(first, second); break;
			case ast::opMod:
				result = ConstantValue::mod(first, second); break;
			case ast::opShl:
				result = ConstantValue::shl(first, second); break;
			case ast::opShr:
				result = ConstantValue::shr(first, second); break;
			case ast::opSar:
				result = ConstantValue::sar(first, second); break;
			case ast::opAnd:
				result = ConstantValue::and_(first, second); break;
			case ast::opOr:
				result = ConstantValue::or_(first, second); break;
			case ast::opXor:
				result = ConstantValue::xor_(first, second); break;
			default:
				emit error(ErrorCodes::ecWTF, tr("Unknown expression operator, ConstantExpressionEvaluator::evaluate"), mLine, mFile);
				return ConstantValue();
		}
		if (!result.isValid()) {
			emit error(ErrorCodes::ecMathematicalOperationOperandTypeMismatch,
					   tr("Mathematical operation operand type mismatch: No operation \"%1\" between %2 and %3").arg(ast::operatorToString(i->mOperator), first.typeName(), second.typeName()), mLine, mFile);
			return ConstantValue();
		}
		first = result;
	}
	return first;
}

ConstantValue ConstantExpressionEvaluator::evaluate(const ast::Unary *s) {
	ConstantValue operand = evaluate(s->mOperand);
	if (!operand.isValid()) return ConstantValue();
	switch (s->mOperator) {
		case ast::opPlus:
			return ConstantValue::plus(operand);
		case ast::opMinus:
			return ConstantValue::minus(operand);
		case ast::opNot:
			return ConstantValue::not_(operand);
		default:
			assert(0);
			return ConstantValue();
	}
}

ConstantValue ConstantExpressionEvaluator::evaluate(const ast::String *s) {
	return s->mValue;
}

ConstantValue ConstantExpressionEvaluator::evaluate(const ast::Integer *s) {
	return s->mValue;
}

ConstantValue ConstantExpressionEvaluator::evaluate(const ast::Float *s) {
	return s->mValue;
}

ConstantValue ConstantExpressionEvaluator::evaluate(const ast::Variable *s) {
	QMap<QString, ConstantValue>::ConstIterator i = mConstants.find(s->mName);
	if (i == mConstants.end()) {
		emit error(ErrorCodes::ecNotConstant, tr("\"%1\" is not constant").arg(s->mName), mLine, mFile);
		return ConstantValue();
	}
	const ConstantValue &v = i.value();
	if (s->mVarType != ast::Variable::Default) {
		switch (s->mVarType) {
			case ast::Variable::Integer:
				if (v.type() != ValueType::Integer) {
					errorConstantAlreadyDefinedWithAnotherType(s->mName);
					return ConstantValue();
				}
				break;
			case ast::Variable::Float:
				if (v.type() != ValueType::Float) {
					errorConstantAlreadyDefinedWithAnotherType(s->mName);
					return ConstantValue();
				}
				break;
			case ast::Variable::Short:
				if (v.type() != ValueType::Short) {
					errorConstantAlreadyDefinedWithAnotherType(s->mName);
					return ConstantValue();
				}
				break;
			case ast::Variable::Byte:
				if (v.type() != ValueType::Byte) {
					errorConstantAlreadyDefinedWithAnotherType(s->mName);
					return ConstantValue();
				}
				break;
			case ast::Variable::String:
				if (v.type() != ValueType::Byte) {
					errorConstantAlreadyDefinedWithAnotherType(s->mName);
					return ConstantValue();
				}
				break;
			case ast::Variable::TypePtr:
				emit error(ErrorCodes::ecNotConstant, tr("Type pointer can't be constant"), mLine, mFile);
				return ConstantValue();
		}
	}
	return i.value();
}


void ConstantExpressionEvaluator::setConstants(const QMap<QString, ConstantValue> &constants) {
	mConstants = constants;
}

void ConstantExpressionEvaluator::setCodeFile(QFile *f) {
	mFile = f;
}

void ConstantExpressionEvaluator::setCodeLine(int l) {
	mLine = l;
}

void ConstantExpressionEvaluator::errorConstantAlreadyDefinedWithAnotherType(const QString &name) {
	emit error(ErrorCodes::ecVarAlreadyDefinedWithAnotherType, tr("Constant \"%1\" already defined with another type").arg(name), mLine, mFile);
}
