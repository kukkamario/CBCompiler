#include "functioncodegenerator.h"
#include "functionsymbol.h"
#include "variablesymbol.h"
#include "labelsymbol.h"
#include "typesymbol.h"
#include "valuetypesymbol.h"
#include "constantsymbol.h"
#include "errorcodes.h"
#include "warningcodes.h"
#include "runtime.h"
#include "functionselectorvaluetype.h"
#include "functionvaluetype.h"
#include "liststringjoin.h"
#include "castcostcalculator.h"
#include "cbfunction.h"

struct CodeGeneratorError {
		CodeGeneratorError(ErrorCodes::ErrorCode ec) : mErrorCodes(ec) { }
		ErrorCodes::ErrorCode errorCode() const { return mErrorCodes; }
	private:
		ErrorCodes::ErrorCode mErrorCodes;
};

FunctionCodeGenerator::FunctionCodeGenerator(Runtime *runtime, Settings *settings, QObject *parent) :
	QObject(parent),
	mSettings(settings),
	mRuntime(runtime)
{
	connect(this, &FunctionCodeGenerator::error, this, &FunctionCodeGenerator::errorOccured);
}

bool FunctionCodeGenerator::generate(Builder *builder, ast::Node *block, CBFunction *func, Scope *globalScope) {

	mLocalScope = func->scope();
	mGlobalScope = globalScope;
	mFunction = func->function();
	mValid = true;
	mBuilder = builder;

	llvm::BasicBlock *firstBasicBlock = llvm::BasicBlock::Create(builder->context(), "firstBB", mFunction);
	mBuilder->setInsertPoint(firstBasicBlock);
	generateAllocas();

	try {
		block->accept(this);
	}
	catch (CodeGeneratorError ) {
		return false;
	}

	if (!mBuilder->currentBasicBlock()->getTerminator()) {
		generateDestructors();
		mBuilder->returnValue(func->returnValue(), Value(func->returnValue(), func->returnValue()->defaultValue()));
		emit warning(WarningCodes::wcReturnsDefaultValue, tr("Function might return default value of the return type because the function doesn't end to return statement"), func->codePoint());
	}

	return mValid;
}

bool FunctionCodeGenerator::generateMainBlock(Builder *builder, ast::Node *block, llvm::Function *func, Scope *localScope, Scope *globalScope) {
	mLocalScope = localScope;
	mGlobalScope = globalScope;
	mFunction = func;
	mValid = true;
	mBuilder = builder;

	llvm::BasicBlock *firstBasicBlock = llvm::BasicBlock::Create(builder->context(), "firstBB", func);
	mBuilder->setInsertPoint(firstBasicBlock);
	if (!generateAllocas()) return false;

	try {
		block->accept(this);
	}
	catch (CodeGeneratorError ) {
		return false;
	}
	generateDestructors();
	mBuilder->returnVoid();

	return mValid;
}

void FunctionCodeGenerator::visit(ast::Expression *n) {
	Value result = generate(n);
	mBuilder->destruct(result);
}


void FunctionCodeGenerator::visit(ast::Const *n) {
	Value constVal = generate(n->value());
	if (!constVal.isConstant()) {
		emit error(ErrorCodes::ecNotConstant, tr("Expression isn't constant expression"), n->codePoint());
		throw CodeGeneratorError(ErrorCodes::ecNotConstant);
	}
	Symbol *sym = mLocalScope->find(n->variable()->identifier()->name());
	assert(sym->type() == Symbol::stConstant);
	ConstantSymbol *constant = static_cast<ConstantSymbol*>(sym);
	constant->setValue(constVal.constant());
}

void FunctionCodeGenerator::visit(ast::VariableDefinition *n) {
	n->identifier();
}

void FunctionCodeGenerator::visit(ast::ArrayInitialization *n) {

}

void FunctionCodeGenerator::visit(ast::FunctionCall *n) {
	mBuilder->destruct(generate(n));
}


Value FunctionCodeGenerator::generate(ast::Integer *n) {
	return Value(ConstantValue(n->value()), mRuntime);
}


Value FunctionCodeGenerator::generate(ast::String *n) {
	return Value(ConstantValue(n->value()), mRuntime);
}

Value FunctionCodeGenerator::generate(ast::Float *n) {
	return Value(ConstantValue(n->value()), mRuntime);
}

Value FunctionCodeGenerator::generate(ast::Variable *n) {
	return generate(n->identifier());
}

Value FunctionCodeGenerator::generate(ast::Identifier *n) {
	Symbol *symbol = mLocalScope->find(n->name());
	switch (symbol->type()) {
		case Symbol::stVariable: {
			VariableSymbol *varSym = static_cast<VariableSymbol*>(symbol);
			return Value(varSym->valueType(), varSym->alloca_(), true);
		}
		case Symbol::stConstant: {
			ConstantSymbol *constant = static_cast<ConstantSymbol*>(symbol);
			return Value(constant->value(), mRuntime);
		}
		case Symbol::stFunctionOrCommand: {
			FunctionSymbol *funcSym = static_cast<FunctionSymbol*>(symbol);
			if (funcSym->functions().size() == 1) {
				Function *func = funcSym->functions().first();
				return Value(func->functionValueType(), func->function(), false);
			}

			return Value(funcSym->functionSelector());
		}
		case Symbol::stLabel:
			emit error(ErrorCodes::ecNotVariable, tr("Symbol \"%1\" is a label defined [%2]").arg(symbol->name(), symbol->codePoint().toString()), n->codePoint());
			throw CodeGeneratorError(ErrorCodes::ecNotVariable);
			return Value();
		case Symbol::stValueType: {
			ValueTypeSymbol *valTySymbol = static_cast<ValueTypeSymbol*>(symbol);
			return Value(valTySymbol->valueType());
		}
		case Symbol::stType: {
			TypeSymbol *type = static_cast<TypeSymbol*>(symbol);
			return Value(type->valueType());
		}
	}
}

Value FunctionCodeGenerator::generate(ast::Expression *n) {
	if (n->associativity() == ast::Expression::LeftToRight) {
		Value op1 = generate(n->firstOperand());
		for (QList<ast::ExpressionNode*>::ConstIterator i = n->operations().begin(); i != n->operations().end(); ++i) {
			ast::ExpressionNode *exprNode = *i;
			Value op2 = generate(exprNode->operand());

			ValueType *valueType = op1.valueType();
			OperationFlags opFlags;

			if (op1.isConstant() ^ op2.isConstant()) {
				op1.toLLVMValue(mBuilder);
				op2.toLLVMValue(mBuilder);
			}

			Value result = valueType->generateOperation(mBuilder, exprNode->op(), op1, op2, opFlags);

			if (opFlags.testFlag(OperationFlag::MayLosePrecision)) {
				emit warning(WarningCodes::wcMayLosePrecision, tr("Operation \"%1\" may lose precision with operands of types \"%2\" and \"%3\"").arg(ast::ExpressionNode::opToString(exprNode->op()), valueType->name(), op2.valueType()->name()), exprNode->codePoint());
				return Value();
			}
			if (opFlags.testFlag(OperationFlag::IntegerDividedByZero)) {
				emit error(ErrorCodes::ecIntegerDividedByZero, tr("Integer divided by zero"), exprNode->codePoint());
				throw CodeGeneratorError(ErrorCodes::ecIntegerDividedByZero);
				return Value();
			}

			if (opFlags.testFlag(OperationFlag::ReferenceRequired)) {
				emit error(ErrorCodes::ecInvalidAssignment, tr("You can only assign a value to a reference"), exprNode->codePoint());
				throw CodeGeneratorError(ErrorCodes::ecInvalidAssignment);
				return Value();
			}
			if (opFlags.testFlag(OperationFlag::CastFromString)) {
				emit warning(WarningCodes::wcMayLosePrecision, tr("Automatic cast from a string to a number."), exprNode->codePoint());
			}

			if (opFlags.testFlag(OperationFlag::NoSuchOperation)) {
				emit error(ErrorCodes::ecMathematicalOperationOperandTypeMismatch, tr("No operation \"%1\" between operands of types \"%2\" and \"%3\"").arg(ast::ExpressionNode::opToString(exprNode->op()), valueType->name(), op2.valueType()->name()), exprNode->codePoint());
				throw CodeGeneratorError(ErrorCodes::ecMathematicalOperationOperandTypeMismatch);
				return Value();
			}

			assert(op1.isValid());

			mBuilder->destruct(op1);
			mBuilder->destruct(op2);

			op1 = result;
		}
		return op1;
	}
	else {
		QList<ast::ExpressionNode*>::ConstIterator i = --n->operations().end();
		Value op2 = generate((*i)->operand());
		ast::ExpressionNode::Op op = (*i)->op();
		CodePoint cp = (*i)->codePoint();
		OperationFlags opFlags;
		if (i != n->operations().begin()) {
			do {
				--i;
				Value op1 = generate((*i)->operand());
				ValueType *valueType = op1.valueType();

				if (op1.isConstant() ^ op2.isConstant()) {
					op1.toLLVMValue(mBuilder);
					op2.toLLVMValue(mBuilder);
				}

				Value result = valueType->generateOperation(mBuilder, op, op1, op2, opFlags);

				if (opFlags.testFlag(OperationFlag::MayLosePrecision)) {
					emit warning(WarningCodes::wcMayLosePrecision, tr("Operation \"%1\" may lose precision with operands of types \"%2\" and \"%3\"").arg(ast::ExpressionNode::opToString(op), valueType->name(), op2.valueType()->name()), cp);
				}
				if (opFlags.testFlag(OperationFlag::IntegerDividedByZero)) {
					emit error(ErrorCodes::ecIntegerDividedByZero, tr("Integer divided by zero"), cp);
					throw CodeGeneratorError(ErrorCodes::ecIntegerDividedByZero);
				}

				if (opFlags.testFlag(OperationFlag::ReferenceRequired)) {
					emit error(ErrorCodes::ecInvalidAssignment, tr("You can only assign a value to a reference"), cp);
					throw CodeGeneratorError(ErrorCodes::ecInvalidAssignment);
				}
				if (opFlags.testFlag(OperationFlag::CastFromString)) {
					emit warning(WarningCodes::wcMayLosePrecision, tr("Automatic cast from a string to a number."), cp);
				}

				if (opFlags.testFlag(OperationFlag::NoSuchOperation)) {
					emit error(ErrorCodes::ecMathematicalOperationOperandTypeMismatch, tr("No operation \"%1\" between operands of types \"%2\" and \"%3\"").arg(ast::ExpressionNode::opToString(op), valueType->name(), op2.valueType()->name()), cp);
					throw CodeGeneratorError(ErrorCodes::ecMathematicalOperationOperandTypeMismatch);
				}

				mBuilder->destruct(op1);
				mBuilder->destruct(op2);

				op = (*i)->op();
				cp = (*i)->codePoint();
				op2 = result;
			} while (i != n->operations().begin());
		}
		Value op1 = generate(n->firstOperand());
		ValueType *valueType = op1.valueType();
		Value result = valueType->generateOperation(mBuilder, op, op1, op2, opFlags);

		if (opFlags.testFlag(OperationFlag::MayLosePrecision)) {
			emit warning(WarningCodes::wcMayLosePrecision, tr("Operation \"%1\" may lose precision with operands of types \"%2\" and \"%3\"").arg(ast::ExpressionNode::opToString(op), valueType->name(), op2.valueType()->name()), cp);
		}
		if (opFlags.testFlag(OperationFlag::IntegerDividedByZero)) {
			emit error(ErrorCodes::ecIntegerDividedByZero, tr("Integer divided by zero"), cp);
			throw CodeGeneratorError(ErrorCodes::ecIntegerDividedByZero);
		}

		if (opFlags.testFlag(OperationFlag::ReferenceRequired)) {
			emit error(ErrorCodes::ecInvalidAssignment, tr("You can only assign a value to a reference"), cp);
			throw CodeGeneratorError(ErrorCodes::ecInvalidAssignment);
		}
		if (opFlags.testFlag(OperationFlag::CastFromString)) {
			emit warning(WarningCodes::wcMayLosePrecision, tr("Automatic cast from a string to a number."), cp);
		}

		if (opFlags.testFlag(OperationFlag::NoSuchOperation)) {
			emit error(ErrorCodes::ecMathematicalOperationOperandTypeMismatch, tr("No operation \"%1\" between operands of types \"%2\" and \"%3\"").arg(ast::ExpressionNode::opToString(op), valueType->name(), op2.valueType()->name()), cp);
			throw CodeGeneratorError(ErrorCodes::ecMathematicalOperationOperandTypeMismatch);
		}

		return result;
	}
}

Value FunctionCodeGenerator::generate(ast::FunctionCall *n) {
	Value functionValue = generate(n->function());
	ValueType *valueType = functionValue.valueType();
	if (!valueType->isCallable()) {
		emit error(ErrorCodes::ecCantFindFunction, tr("%1 is not a function and can't be called").arg(valueType->name()), n->codePoint());
		throw CodeGeneratorError(ErrorCodes::ecCantFindFunction);
	}
	QList<Value> paramValues = generateParameterList(n->parameters());

	if (valueType->isFunctionSelector()) {
		FunctionSelectorValueType *funcSelector = static_cast<FunctionSelectorValueType*>(valueType);
		QList<Function*> functions = funcSelector->overloads();

		Function *func = findBestOverload(functions, paramValues, n->isCommand(), n->codePoint());
		assert(func);

		return mBuilder->call(func, paramValues);
	}
	else {
		FunctionValueType *functionValueType = static_cast<FunctionValueType*>(valueType);
		llvm::Value *llvmFunction = functionValue.value();
		return mBuilder->call(functionValueType, llvmFunction, paramValues);
	}
}


Value FunctionCodeGenerator::generate(ast::KeywordFunctionCall *n) {

}

Value FunctionCodeGenerator::generate(ast::ArraySubscript *n) {

}



Value FunctionCodeGenerator::generate(ast::WhileStatement *n) {

}

Value FunctionCodeGenerator::generate(ast::RepeatForeverStatement *n) {

}

Value FunctionCodeGenerator::generate(ast::RepeatUntilStatement *n) {

}

Value FunctionCodeGenerator::generate(ast::ForToStatement *n)
{

}

Value FunctionCodeGenerator::generate(ast::ForEachStatement *n)
{

}

Value FunctionCodeGenerator::generate(ast::SelectStatement *n) {

}

Value FunctionCodeGenerator::generate(ast::SelectCase *n)
{

}

Value FunctionCodeGenerator::generate(ast::Const *n) {

}

Value FunctionCodeGenerator::generate(ast::Node *n) {
	switch (n->type()) {
		case ast::Node::ntBlock:
			return generate(n->cast<ast::Block>());
		case ast::Node::ntInteger:
			return generate(n->cast<ast::Integer>());
		case ast::Node::ntFloat:
			return generate(n->cast<ast::Float>());
		case ast::Node::ntString:
			return generate(n->cast<ast::String>());
		case ast::Node::ntIdentifier:
			return generate(n->cast<ast::Identifier>());
		case ast::Node::ntLabel:
			return generate(n->cast<ast::Label>());
		case ast::Node::ntList:
			return generate(n->cast<ast::List>());
		case ast::Node::ntGoto:
			return generate(n->cast<ast::Goto>());
		case ast::Node::ntGosub:
			return generate(n->cast<ast::Gosub>());
		case ast::Node::ntReturn:
			return generate(n->cast<ast::Return>());
		case ast::Node::ntExit:
			return generate(n->cast<ast::Exit>());


		case ast::Node::ntExpression:
			return generate(n->cast<ast::Expression>());
		case ast::Node::ntExpressionNode:
			return generate(n->cast<ast::ExpressionNode>());
		case ast::Node::ntUnary:
			return generate(n->cast<ast::Unary>());
		case ast::Node::ntArraySubscript:
			return generate(n->cast<ast::ArraySubscript>());
		case ast::Node::ntFunctionCall:
			return generate(n->cast<ast::FunctionCall>());
		case ast::Node::ntKeywordFunctionCall:
			return generate(n->cast<ast::KeywordFunctionCall>());
		case ast::Node::ntDefaultValue:
			return generate(n->cast<ast::DefaultValue>());
		case ast::Node::ntVariable:
			return generate(n->cast<ast::Variable>());

		default:
			assert("Invalid ast::Node for generate(ast::Node*)" && 0);
			return Value();
	}
}

Function *FunctionCodeGenerator::findBestOverload(const QList<Function *> &functions, const QList<Value> &parameters, bool command, const CodePoint &cp) {
	//No overloads
	if (functions.size() == 1) {
		Function *f = functions.first();

		if (f->isCommand() != command) {
			if (command) {
				emit error(ErrorCodes::ecNotCommand, tr("There is no command \"%1\" (but there is a function with same name)").arg(f->name()), cp);
				throw CodeGeneratorError(ErrorCodes::ecNotCommand);
			} else {
				emit error(ErrorCodes::ecCantFindFunction,  tr("There is no function \"%1\" (but there is a command with same name)").arg(f->name()), cp);
				throw CodeGeneratorError(ErrorCodes::ecCantFindFunction);
			}
		}

		if (f->requiredParams() > parameters.size()) {
			emit error(ErrorCodes::ecCantFindFunction, tr("Function doesn't match given parameters. \"%1\" was tried to call with parameters of types (%2)").arg(
						   f->functionValueType()->name(),
						   listStringJoin(parameters, [](const Value &val) {
				return val.valueType()->name();
			})), cp);
			throw CodeGeneratorError(ErrorCodes::ecCantFindFunction);
		}

		Function::ParamList::ConstIterator p1i = f->paramTypes().begin();
		CastCostCalculator totalCost;
		for (QList<Value>::ConstIterator p2i = parameters.begin(); p2i != parameters.end(); p2i++) {
			totalCost += p2i->valueType()->castingCostToOtherValueType(*p1i);
			p1i++;
		}
		if (!totalCost.isCastPossible()) {
			if (command) {
				emit error(ErrorCodes::ecCantFindFunction, tr("Command doesn't match given parameters. \"%1\" was tried to call with parameters of types (%2)").arg(
							   f->functionValueType()->name(),
							   listStringJoin(parameters, [](const Value &val) {
					return val.valueType()->name();
				})), cp);
				throw CodeGeneratorError(ErrorCodes::ecCantFindCommand);
			}
			else {
				emit error(ErrorCodes::ecCantFindFunction, tr("Function doesn't match given parameters. \"%1\" was tried to call with parameters of types (%2)").arg(
							   f->functionValueType()->name(),
							   listStringJoin(parameters, [](const Value &val) {
					return val.valueType()->name();
				})), cp);
				throw CodeGeneratorError(ErrorCodes::ecCantFindFunction);
			}
		}
		return f;
	}
	bool multiples = false;
	Function *bestFunc = 0;
	CastCostCalculator bestCost = CastCostCalculator::maxCastCost();

	for (QList<Function*>::ConstIterator fi = functions.begin(); fi != functions.end(); fi++) {
		Function *f = *fi;
		if (f->paramTypes().size() >= parameters.size() && f->requiredParams() <= parameters.size() && f->isCommand() == command) {
			QList<ValueType*>::ConstIterator p1i = f->paramTypes().begin();
			CastCostCalculator totalCost;
			for (QList<Value>::ConstIterator p2i = parameters.begin(); p2i != parameters.end(); p2i++) {
				totalCost += p2i->valueType()->castingCostToOtherValueType(*p1i);
				p1i++;
			}
			if (totalCost == bestCost) {
				multiples = true;
				continue;
			}
			if (totalCost < bestCost) {
				bestFunc = f;
				bestCost = totalCost;
				multiples = false;
				continue;
			}
		}
	}
	if (bestFunc == 0) {
		if (command) {
			emit error(ErrorCodes::ecCantFindCommand, tr("Can't find a command overload which would accept given parameters (%1)").arg(
						   listStringJoin(parameters, [](const Value &val) {
				return val.valueType()->name();
			})), cp);
			throw CodeGeneratorError(ErrorCodes::ecCantFindCommand);
		}
		else {
			emit error(ErrorCodes::ecCantFindFunction, tr("Can't find a function overload which would accept given parameters (%1)").arg(
						   listStringJoin(parameters, [](const Value &val) {
				return val.valueType()->name();
			})), cp);
			throw CodeGeneratorError(ErrorCodes::ecCantFindFunction);
		}

	}
	if (multiples) {
		QString msg = command ? tr("Found multiple possible command overloads with parameters (%1) and can't choose between them.") : tr("Found multiple possible function overloads with parameters (%1) and can't choose between them.");
		emit error(ErrorCodes::ecMultiplePossibleOverloads,  msg.arg(
					   listStringJoin(parameters, [](const Value &val) {
			return val.valueType()->name();
		})), cp);
		throw CodeGeneratorError(ErrorCodes::ecMultiplePossibleOverloads);
	}
	return bestFunc;
}

QList<Value> FunctionCodeGenerator::generateParameterList(ast::Node *n) {
	QList<Value> result;
	if (n->type() == ast::Node::ntList) {
		for (ast::ChildNodeIterator i = n->childNodesBegin(); i != n->childNodesEnd(); i++) {
			result.append(generate(*i));
		}
	}
	else {
		result.append(generate(n));
	}
	return result;
}

bool FunctionCodeGenerator::generateAllocas() {
	for (Scope::Iterator i = mLocalScope->begin(); i != mLocalScope->end(); ++i) {
		Symbol *symbol = *i;
		if (symbol->type() == Symbol::stVariable) {
			VariableSymbol *varSymbol = static_cast<VariableSymbol*>(symbol);
			mBuilder->construct(varSymbol);
		}
	}
	return true;
}

void FunctionCodeGenerator::generateDestructors() {
	for (Scope::Iterator i = mLocalScope->begin(); i != mLocalScope->end(); ++i) {
		Symbol *symbol = *i;
		if (symbol->type() == Symbol::stVariable) {
			VariableSymbol *varSymbol = static_cast<VariableSymbol*>(symbol);
			mBuilder->destruct(varSymbol);
		}
	}
}


void FunctionCodeGenerator::errorOccured(int, QString, CodePoint) {
	mValid = false;
}
