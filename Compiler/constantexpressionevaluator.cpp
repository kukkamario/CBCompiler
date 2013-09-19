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
		case ast::Node::ntFunctionCallOrArraySubscript:
			emit error(ErrorCodes::ecNotConstant, tr("Neither function call nor array subscript is allowed inside constant expression"), mLine, mFile);
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

	Symbol *sym = mGlobalScope->find(s->mName);
	if (sym->type() != Symbol::stConstant) {
		emit error(ErrorCodes::ecNotConstant, tr("\"%1\" is not constant").arg(s->mName), mLine, mFile);
		return ConstantValue();
	}
	const ConstantValue &v = static_cast<ConstantSymbol*>(sym)->value();
	if (!s->mTypeName.isEmpty()) {
		switch(v.type()) {
			case ValueType::Integer:
				if (s->mTypeName != mRuntime->intValueType()->name()) {
					errorConstantAlreadyDefinedWithAnotherType(s->mName);
					return ConstantValue();
				}
				break;
			case ValueType::Float:
				if (s->mTypeName != mRuntime->floatValueType()->name()) {
					errorConstantAlreadyDefinedWithAnotherType(s->mName);
					return ConstantValue();
				}
				break;
			case ValueType::Short:
				if (s->mTypeName != mRuntime->shortValueType()->name()) {
					errorConstantAlreadyDefinedWithAnotherType(s->mName);
					return ConstantValue();
				}
				break;
			case ValueType::Byte:
				if (s->mTypeName != mRuntime->byteValueType()->name()) {
					errorConstantAlreadyDefinedWithAnotherType(s->mName);
					return ConstantValue();
				}
				break;
			case ValueType::String:
				if (s->mTypeName != mRuntime->stringValueType()->name()) {
					errorConstantAlreadyDefinedWithAnotherType(s->mName);
					return ConstantValue();
				}
				break;
			default:
				assert("Invalid constant value");
		}
	}
	return v;
}

void ConstantExpressionEvaluator::setRuntime(Runtime *runtime) {
	mRuntime = runtime;
}

void ConstantExpressionEvaluator::setGlobalScope(Scope *globalScope) {
	mGlobalScope = globalScope;
}


void ConstantExpressionEvaluator::setCodeFile(const QString &f) {
	mFile = f;
}

void ConstantExpressionEvaluator::setCodeLine(int l) {
	mLine = l;
}

void ConstantExpressionEvaluator::errorConstantAlreadyDefinedWithAnotherType(const QString &name) {
	emit error(ErrorCodes::ecVarAlreadyDefinedWithAnotherType, tr("Constant \"%1\" already defined with another type").arg(name), mLine, mFile);
}
