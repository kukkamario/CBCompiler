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
#include "arrayvaluetype.h"
#include "intvaluetype.h"
#include "shortvaluetype.h"
#include "bytevaluetype.h"
#include "typepointervaluetype.h"
#include "booleanvaluetype.h"
#include "liststringjoin.h"
#include "castcostcalculator.h"
#include "cbfunction.h"
#include "structvaluetype.h"

#define CHECK_UNREACHABLE(codePoint) if (checkUnreachable(codePoint)) return;

struct CodeGeneratorError {
		CodeGeneratorError(ErrorCodes::ErrorCode ec) : mErrorCodes(ec) { }
		ErrorCodes::ErrorCode errorCode() const { return mErrorCodes; }
	private:
		ErrorCodes::ErrorCode mErrorCodes;
};

FunctionCodeGenerator::FunctionCodeGenerator(Runtime *runtime, Settings *settings, QObject *parent) :
	QObject(parent),
	mSettings(settings),
	mRuntime(runtime),
	mTypeResolver(runtime)
{
	connect(this, &FunctionCodeGenerator::error, this, &FunctionCodeGenerator::errorOccured);
	connect(&mConstEval, &ConstantExpressionEvaluator::error, this, &FunctionCodeGenerator::error);
	connect(&mConstEval, &ConstantExpressionEvaluator::warning, this, &FunctionCodeGenerator::warning);
	connect(&mTypeResolver, &TypeResolver::error, this, &FunctionCodeGenerator::error);
	connect(&mTypeResolver, &TypeResolver::warning, this, &FunctionCodeGenerator::warning);
}

bool FunctionCodeGenerator::generate(Builder *builder, ast::Node *block, CBFunction *func, Scope *globalScope) {
	mMainFunction = false;
	mUnreachableBasicBlock = false;
	mLocalScope = func->scope();
	mGlobalScope = globalScope;
	mFunction = func->function();
	mValid = true;
	mBuilder = builder;
	mReturnType = func->returnValue();
	mUnresolvedGotos.clear();

	llvm::BasicBlock *firstBasicBlock = llvm::BasicBlock::Create(builder->context(), "firstBB", mFunction);
	mBuilder->setInsertPoint(firstBasicBlock);
	generateAllocas();
	generateFunctionParameterAssignments(func->parameters());

	try {
		block->accept(this);
	}
	catch (CodeGeneratorError ) {
		return false;
	}

	if (!mBuilder->currentBasicBlock()->getTerminator()) {
		generateDestructors();
		if (func->returnValue()) {
			mBuilder->returnValue(func->returnValue(), Value(func->returnValue(), func->returnValue()->defaultValue()));
			emit warning(WarningCodes::wcReturnsDefaultValue, tr("Function might return default value of the return type because the function doesn't end to return statement"), func->codePoint());
		}
		else
			mBuilder->returnVoid();
		}

	resolveGotos();

	return mValid;
}

bool FunctionCodeGenerator::generateMainBlock(Builder *builder, ast::Node *block, llvm::Function *func, Scope *localScope, Scope *globalScope) {
	mLocalScope = localScope;
	mGlobalScope = globalScope;
	mFunction = func;
	mValid = true;
	mUnreachableBasicBlock = false;
	mBuilder = builder;
	mMainFunction = true;
	mReturnType = 0;
	mUnresolvedGotos.clear();

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

	resolveGotos();

	return mValid;
}

void FunctionCodeGenerator::visit(ast::Expression *n) {
	CHECK_UNREACHABLE(n->codePoint());

	Value result = generate(n);
	mBuilder->destruct(result);
}


void FunctionCodeGenerator::visit(ast::Const *n) {
	/*CHECK_UNREACHABLE(n->codePoint());

	Value constVal = generate(n->value());
	if (!constVal.isConstant()) {
		emit error(ErrorCodes::ecNotConstant, tr("Expression isn't constant expression"), n->codePoint());
		throw CodeGeneratorError(ErrorCodes::ecNotConstant);
	}
	Symbol *sym = mLocalScope->find(n->variable()->identifier()->name());
	assert(sym->type() == Symbol::stConstant);
	ConstantSymbol *constant = static_cast<ConstantSymbol*>(sym);
	constant->setValue(constVal.constant());*/
}

void FunctionCodeGenerator::visit(ast::VariableDefinition *n) {
	CHECK_UNREACHABLE(n->codePoint());

	Value ref = generate(n->identifier());
	Value init;
	if (n->value()->type() == ast::Node::ntDefaultValue) {
		init = Value(ref.valueType(), ref.valueType()->defaultValue());
	}
	else {
		init = generate(n->value());
	}

	OperationFlags flags = ValueType::castCostOperationFlags(init.valueType()->castingCostToOtherValueType(ref.valueType()));
	if (operationFlagsContainFatalFlags(flags)) {
		emit error(ErrorCodes::ecCantCastValue, tr("Can't cast %1 to %2").arg(init.valueType()->name(), ref.valueType()->name()), n->identifier()->codePoint());
		return;
	}

	mBuilder->store(ref, ref.valueType()->cast(mBuilder, init));
}

void FunctionCodeGenerator::visit(ast::ArrayInitialization *n) {
	CHECK_UNREACHABLE(n->codePoint());

	Value arr = generate(n->array());
	assert(arr.isReference());

	if (arr.isNormalValue() && arr.valueType()->isArray()) {
		ArrayValueType *valType = static_cast<ArrayValueType*>(arr.valueType());
		QList<Value> params = generateParameterList(n->dimensions());
		if (params.size() != valType->dimensions()) {
			emit error(ErrorCodes::ecArrayDimensionCountDoesntMatch, tr("Invalid number of dimensions %1. The array has %n dimensions.", 0, valType->dimensions()).arg(params.size()), n->codePoint());
			throw CodeGeneratorError(ErrorCodes::ecArrayDimensionCountDoesntMatch);
		}
		int index = 1;
		for (const Value &v : params) {
			if (!(v.valueType() == mRuntime->intValueType()  || v.valueType() == mRuntime->shortValueType() || v.valueType() == mRuntime->byteValueType() || v.valueType() == mRuntime->booleanValueType())) {
				emit error(ErrorCodes::ecArraySubscriptNotInteger, tr("Array initialization parameter %1 is not an integer").arg(index), n->codePoint());
				throw CodeGeneratorError(ErrorCodes::ecArraySubscriptNotInteger);
			}
			index++;

		}
		Value val = valType->constructArray(mBuilder, params);
		valType->assignArray(mBuilder, arr.value(), mBuilder->llvmValue(val));
	} else {
		assert("Shouldn't this be checked else where?" && 0);
	}

}

void FunctionCodeGenerator::visit(ast::FunctionCall *n) {
	CHECK_UNREACHABLE(n->codePoint());
	mBuilder->destruct(generate(n));
}

void FunctionCodeGenerator::visit(ast::IfStatement *n) {
	CHECK_UNREACHABLE(n->codePoint());
	llvm::BasicBlock *condBB = mBuilder->currentBasicBlock();
	Value cond = generate(n->condition());
	llvm::BasicBlock *trueBlock = createBasicBlock("trueBB");
	llvm::BasicBlock *endBlock = createBasicBlock("endIfBB");

	mBuilder->setInsertPoint(trueBlock);
	n->block()->accept(this);
	if (!mUnreachableBasicBlock)
		mBuilder->branch(endBlock);
	mUnreachableBasicBlock = false;

	llvm::BasicBlock *elseBlock = 0;
	if (n->elseBlock()) {
		elseBlock = createBasicBlock("elseBB");
		mBuilder->setInsertPoint(elseBlock);
		n->elseBlock()->accept(this);
		if (!mUnreachableBasicBlock)
			mBuilder->branch(endBlock);
		mUnreachableBasicBlock = false;
	}


	mBuilder->setInsertPoint(condBB);
	if (n->elseBlock()) {
		mBuilder->branch(cond, trueBlock, elseBlock);
	}
	else {
		mBuilder->branch(cond, trueBlock, endBlock);
	}

	mBuilder->setInsertPoint(endBlock);
}

void FunctionCodeGenerator::visit(ast::WhileStatement *n) {
	CHECK_UNREACHABLE(n->codePoint());
	llvm::BasicBlock *condBB = createBasicBlock("whileCondBB");
	llvm::BasicBlock *blockBB = createBasicBlock("whileBodyBB");
	llvm::BasicBlock *wendBB = createBasicBlock("wendBB");

	mBuilder->branch(condBB);
	mBuilder->setInsertPoint(condBB);

	Value cond = generate(n->condition());
	mBuilder->branch(cond, blockBB, wendBB);

	mExitStack.push(wendBB);
	mBuilder->setInsertPoint(blockBB);
	n->block()->accept(this);
	if (!mUnreachableBasicBlock)
		mBuilder->branch(condBB);
	mUnreachableBasicBlock = false;
	mExitStack.pop();

	mBuilder->setInsertPoint(wendBB);
}

void FunctionCodeGenerator::visit(ast::RepeatForeverStatement *n) {
	CHECK_UNREACHABLE(n->codePoint());

	llvm::BasicBlock *block = createBasicBlock("repeatForeverBB");
	llvm::BasicBlock *endBlock = createBasicBlock("repeatForeverEndBB");

	mBuilder->branch(block);
	mBuilder->setInsertPoint(block);
	mExitStack.push(endBlock);
	n->block()->accept(this);
	mExitStack.pop();
	if (!mUnreachableBasicBlock)
		mBuilder->branch(block);
	mUnreachableBasicBlock = false;

	mBuilder->setInsertPoint(endBlock);


}

void FunctionCodeGenerator::visit(ast::RepeatUntilStatement *n) {
	CHECK_UNREACHABLE(n->codePoint());

	llvm::BasicBlock *block = createBasicBlock("repeatUntilBB");
	llvm::BasicBlock *endBlock = createBasicBlock("repeatUntilEndBB");

	mBuilder->branch(block);
	mBuilder->setInsertPoint(block);
	mExitStack.push(endBlock);
	n->block()->accept(this);
	mExitStack.pop();
	Value cond = generate(n->condition());
	if (!mUnreachableBasicBlock) {
		mBuilder->branch(cond, endBlock, block);
	}
	mUnreachableBasicBlock = false;

	mBuilder->setInsertPoint(endBlock);

}

void FunctionCodeGenerator::visit(ast::ForToStatement *n) {
	CHECK_UNREACHABLE(n->codePoint());

	llvm::BasicBlock *condBB = createBasicBlock("ForToCondBB");
	llvm::BasicBlock *blockBB = createBasicBlock("forBodyBB");
	llvm::BasicBlock *endBB = createBasicBlock("endForBB");

	Value value = generate(n->from());
	if (!value.isReference()) {
		emit error(ErrorCodes::ecReferenceRequired, tr("Expression should return a reference to a variable"), n->from()->codePoint());
		return;
	}
	ConstantValue step = 1;
	if (n->step()) {
		step = mConstEval.evaluate(n->step());
		if (!step.isValid()) {
			emit error(ErrorCodes::ecNotConstant, tr("\"Step\" has to be a constant value"), n->step()->codePoint());
			return;
		}
	}

	OperationFlags flags;
	bool positiveStep = ConstantValue::greaterEqual(step, ConstantValue(0), flags).toBool();

	mBuilder->branch(condBB);
	mBuilder->setInsertPoint(condBB);

	Value to = generate(n->to());
	Value cond;
	if (positiveStep) {
		cond = mBuilder->lessEqual(value, to);
	} else {
		cond = mBuilder->greaterEqual(value, to);
	}

	mBuilder->branch(cond, blockBB, endBB);

	mExitStack.push(endBB);
	mBuilder->setInsertPoint(blockBB);
	n->block()->accept(this);

	if (!mUnreachableBasicBlock) {
		mBuilder->store(value, mBuilder->add(value, Value(step, mRuntime)));
		mBuilder->branch(condBB);
	}
	mUnreachableBasicBlock = false;
	mExitStack.pop();
	mBuilder->setInsertPoint(endBB);
}

void FunctionCodeGenerator::visit(ast::ForEachStatement *n) {
	CHECK_UNREACHABLE(n->codePoint());
	Value container = generate(n->container());
	Value var = generate(n->variable());
	assert(var.isReference());

	llvm::BasicBlock *condBB = createBasicBlock("forEachCondBB");
	llvm::BasicBlock *blockBB = createBasicBlock("forEachBlockBB");
	llvm::BasicBlock *endBB = createBasicBlock("endForEachBB");

	ValueType *valueType = container.valueType();
	if (container.isValueType() && valueType->isTypePointer()) { //Type
		if (var.valueType() != valueType) {
			emit error(ErrorCodes::ecInvalidAssignment, tr("The type of the variable \"%1\" doesn't match the container type \"%2\"").arg(var.valueType()->name(), valueType->name()), n->codePoint());
			return;
		}
		TypePointerValueType *typePointerValueType = static_cast<TypePointerValueType*>(valueType);
		TypeSymbol *typeSymbol = typePointerValueType->typeSymbol();

		mBuilder->store(var, mBuilder->firstTypeMember(typeSymbol));
		mBuilder->branch(condBB);

		mBuilder->setInsertPoint(condBB);
		Value cond = mBuilder->typePointerNotNull(var);
		mBuilder->branch(cond, blockBB, endBB);

		mBuilder->setInsertPoint(blockBB);
		n->block()->accept(this);
		if (!mUnreachableBasicBlock) {
			mBuilder->store(var, mBuilder->afterTypeMember(var));
			mBuilder->branch(condBB);
		}
		mUnreachableBasicBlock = false;

		mBuilder->setInsertPoint(endBB);
	}
	else if (container.isNormalValue() && valueType->isArray()) {
		VariableSymbol *varSym = searchVariableSymbol(n->variable());
		ArrayValueType *array = static_cast<ArrayValueType*>(valueType);
		valueType = array->baseType();
		if (var.valueType() != valueType) {
			emit error(ErrorCodes::ecInvalidAssignment, tr("The type of the variable \"%1\" doesn't match the container item type \"%2\"").arg(var.valueType()->name(), valueType->name()), n->codePoint());
			return;
		}

		llvm::Value *alloc = varSym->alloca_();


		llvm::Value *arrayData = array->dataArray(mBuilder, container);
		llvm::Value *arrayDataPtr = mBuilder->irBuilder().CreateAlloca(arrayData->getType());
		llvm::Value *totalSize = array->totalSize(mBuilder, container);
		llvm::Value *arrayEndPtr = mBuilder->irBuilder().CreateGEP(arrayData, totalSize);
		mBuilder->irBuilder().CreateStore(arrayData, arrayDataPtr);
		mBuilder->branch(condBB);

		mBuilder->setInsertPoint(condBB);
		arrayData = mBuilder->irBuilder().CreateLoad(arrayDataPtr);
		llvm::Value *cond = mBuilder->irBuilder().CreateICmpNE(arrayData, arrayEndPtr);
		mBuilder->irBuilder().CreateCondBr(cond, blockBB, endBB);

		mBuilder->setInsertPoint(blockBB);
		arrayData = mBuilder->irBuilder().CreateLoad(arrayDataPtr);
		varSym->setAlloca(arrayData);
		mExitStack.push(endBB);

		n->block()->accept(this);
		if (!mUnreachableBasicBlock) {
			arrayData = mBuilder->irBuilder().CreateGEP(arrayData, mBuilder->irBuilder().getInt32(1));
			mBuilder->irBuilder().CreateStore(arrayData, arrayDataPtr);
			mBuilder->branch(condBB);
		}
		mUnreachableBasicBlock = false;
		mExitStack.pop();

		mBuilder->setInsertPoint(endBB);
		varSym->setAlloca(alloc);
	}
	else {
		emit error(ErrorCodes::ecNotContainer, tr("For-Each-statement requires a container (an array or a type). Can't iterate \"%1\"").arg(valueType->name()), n->container()->codePoint());
		return;
	}
}

void FunctionCodeGenerator::visit(ast::SelectStatement *n) {
	CHECK_UNREACHABLE(n->codePoint());

	Value value = generate(n->variable());
	if (n->cases().size() == 0) {
		if (n->defaultCase()) {
			n->defaultCase()->accept(this);
			return;
		}
		else {
			emit error(ErrorCodes::ecEmptySelect, tr("Empty select statement"), n->codePoint());
			return;
		}
	}


	value.toLLVMValue(mBuilder);
	llvm::BasicBlock *beginBlock = mBuilder->currentBasicBlock();
	llvm::BasicBlock *endBlock = createBasicBlock("selectEndBB");

	bool switchPossible = value.valueType() == mRuntime->intValueType() || value.valueType() == mRuntime->shortValueType() || value.valueType() == mRuntime->byteValueType();
	QList<QPair<Value, llvm::BasicBlock*> > values;
	for (ast::SelectCase *c : n->cases()) {
		Value value = generate(c->value());
		switchPossible &= value.isConstant() && (value.valueType() == mRuntime->intValueType() || value.valueType() == mRuntime->shortValueType() || value.valueType() == mRuntime->byteValueType());
		llvm::BasicBlock *basicBlock = createBasicBlock("caseBB");
		mBuilder->setInsertPoint(basicBlock);
		c->block()->accept(this);
		if (!mUnreachableBasicBlock) {
			mBuilder->branch(endBlock);
		}
		mUnreachableBasicBlock = false;
		values.append(QPair<Value, llvm::BasicBlock*>(value, basicBlock));
	}
	llvm::BasicBlock *defaultBlock = endBlock;
	if (n->defaultCase()) {
		defaultBlock = createBasicBlock("defaultBB");
		mBuilder->setInsertPoint(defaultBlock);
		n->defaultCase()->accept(this);
		if (!mUnreachableBasicBlock) {
			mBuilder->branch(endBlock);
		}
		mUnreachableBasicBlock = false;
	}

	mBuilder->setInsertPoint(beginBlock);
	if (switchPossible) {
		llvm::SwitchInst *switchInst = mBuilder->irBuilder().CreateSwitch(
					mBuilder->llvmValue(mBuilder->toInt(value)),
					defaultBlock,
					values.size());
		for (const QPair<Value, llvm::BasicBlock*> &pairs : values) {
			llvm::ConstantInt *caseVal = llvm::cast<llvm::ConstantInt>(mBuilder->llvmValue(mBuilder->toInt(pairs.first)));
			switchInst->addCase(caseVal, pairs.second);
		}
	}
	else {
		ValueType *valueType = value.valueType();
		int index = 0;
		Value caseTrue;
		llvm::BasicBlock *trueBlock = 0;
		for (QList<QPair<Value, llvm::BasicBlock*> >::ConstIterator i = values.begin(); i != values.end(); i++) {
			if (trueBlock) {
				llvm::BasicBlock *caseCondBB = createBasicBlock("caseCondBB");
				mBuilder->branch(caseTrue, trueBlock, caseCondBB);
				mBuilder->setInsertPoint(caseCondBB);
			}
			OperationFlags opFlags;
			ast::ExpressionNode::Op op = ast::ExpressionNode::opEqual;
			caseTrue = valueType->generateOperation(mBuilder, op, value, i->first, opFlags);
			mBuilder->destruct(i->first);
			if (opFlags.testFlag(OperationFlag::MayLosePrecision)) {
				emit warning(WarningCodes::wcMayLosePrecision, tr("Operation \"%1\" may lose precision with operands of types \"%2\" and \"%3\"").arg(ast::ExpressionNode::opToString(op), valueType->name(), i->first.valueType()->name()), n->cases().value(index)->codePoint());
			}
			if (opFlags.testFlag(OperationFlag::IntegerDividedByZero)) {
				emit error(ErrorCodes::ecIntegerDividedByZero, tr("Integer divided by Zero"), n->cases().value(index)->codePoint());
				return;
			}
			if (opFlags.testFlag(OperationFlag::CastFromString)) {
				emit warning(WarningCodes::wcMayLosePrecision, tr("Automatic cast from a string to a number."), n->cases().value(index)->codePoint());
			}
			if (opFlags.testFlag(OperationFlag::OperandBCantBeCastedToA)) {
				emit error(ErrorCodes::ecCantCastValue, tr("Can't cast \"%1\" to \"%2\"").arg(valueType->name(), i->first.valueType()->name()), n->cases().value(index)->codePoint());
				throw CodeGeneratorError(ErrorCodes::ecCantCastValue);
			}

			if (opFlags.testFlag(OperationFlag::NoSuchOperation)) {
				emit error(ErrorCodes::ecMathematicalOperationOperandTypeMismatch, tr("No operation \"%1\" between operands of types \"%2\" and \"%3\"").arg(ast::ExpressionNode::opToString(op), valueType->name(), i->first.valueType()->name()), n->cases().value(index)->codePoint());
				return;
			}
			trueBlock = i->second;


			index++;
		}
		mBuilder->branch(caseTrue, trueBlock, endBlock);
	}
	mBuilder->setInsertPoint(endBlock);
	mBuilder->destruct(value);
}

void FunctionCodeGenerator::visit(ast::Return *n) {
	CHECK_UNREACHABLE(n->codePoint());
	if (mMainFunction) {
		assert("Implement gosub" && 0);
	}
	else {
		if (n->value()) {
			if (!mReturnType) {
				emit error(ErrorCodes::ecReturnParameterInCommand,
						   tr("Return doesn't take parameters in a command. (if you wish to return a value, define the return type of a function after the parameter list.)"),
						   n->value()->codePoint());
				return;
			}

			Value v = generate(n->value());
			if (v.isReference()) {
				v = mBuilder->load(v);
			}
			generateDestructors();
			ValueType::CastCost cc = v.valueType()->castingCostToOtherValueType(mReturnType);
			OperationFlags opFlags = ValueType::castCostOperationFlags(cc);

			if (opFlags.testFlag(OperationFlag::OperandBCantBeCastedToA)) {
				emit error(ErrorCodes::ecCantCastValue, tr("Can't cast \"%1\" to \"%2\"").arg(v.valueType()->name(), mReturnType->name()), n->value()->codePoint());
				return;
			}

			mBuilder->returnValue(mReturnType, v);
		}
		else {
			if (mReturnType)
				mBuilder->returnValue(mReturnType, mBuilder->defaultValue(mReturnType));
			else
				mBuilder->returnVoid();
		}
	}
	mUnreachableBasicBlock = true;
}

void FunctionCodeGenerator::visit(ast::Goto *n) {
	CHECK_UNREACHABLE(n->codePoint());

	Symbol *sym = mLocalScope->find(n->label()->name());
	if (!sym) {
		emit error(ErrorCodes::ecCantFindSymbol, tr("Can't find label \"%1\"").arg(n->label()->name()), n->label()->codePoint());
		return;
	}
	if (sym->type() != Symbol::stLabel) {
		emit error(ErrorCodes::ecNotLabel, tr("Symbol \"%1\" isn't a label").arg(n->label()->name()), n->label()->codePoint());
		return;
	}

	LabelSymbol *label = static_cast<LabelSymbol*>(sym);
	if (label->basicBlock()) {
		mBuilder->branch(label->basicBlock());
	}
	else {
		mUnresolvedGotos.append(QPair<LabelSymbol*, llvm::BasicBlock*>(label, mBuilder->currentBasicBlock()));
	}
	mUnreachableBasicBlock = true;
}

void FunctionCodeGenerator::visit(ast::Gosub *n) {
	assert("UNIMPLEMENTED FUNCTION" && 0);
}

void FunctionCodeGenerator::visit(ast::Label *n) {
	llvm::BasicBlock *bb = createBasicBlock("label_" + n->name().toStdString());
	if (!mUnreachableBasicBlock) {
		mBuilder->branch(bb);
	}
	mBuilder->setInsertPoint(bb);
	mUnreachableBasicBlock = false;

	Symbol *sym = mLocalScope->find(n->name());
	assert(sym && sym->type() == Symbol::stLabel);

	LabelSymbol *label = static_cast<LabelSymbol*>(sym);
	label->setBasicBlock(bb);
}

void FunctionCodeGenerator::visit(ast::Exit *n) {
	CHECK_UNREACHABLE(n->codePoint());

	if (mExitStack.empty()) {
		emit error(ErrorCodes::ecInvalidExit, tr("Nothing to exit from"), n->codePoint());
		return;
	}

	llvm::BasicBlock *bb = mExitStack.top();
	mBuilder->branch(bb);
	mUnreachableBasicBlock = true;
}

void FunctionCodeGenerator::visit(ast::KeywordFunctionCall *n) {
	CHECK_UNREACHABLE(n->codePoint());
	if (n->keyword() == ast::KeywordFunctionCall::Delete) {
		generate(n);
		return;
	}
	emit warning(WarningCodes::wcUselessLineIgnored, tr("Ignored expression because it doesn't affect anything"), n->codePoint());
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
	assert(symbol);
	switch (symbol->type()) {
		case Symbol::stVariable: {
			VariableSymbol *varSym = static_cast<VariableSymbol*>(symbol);
			return Value(varSym->valueType(), varSym->alloca_(), true);
		}
		case Symbol::stConstant: {
			ConstantSymbol *constant = static_cast<ConstantSymbol*>(symbol);
			return constant->valueType()->cast(mBuilder, Value(constant->value(), mRuntime));
		}
		case Symbol::stFunctionOrCommand: {
			FunctionSymbol *funcSym = static_cast<FunctionSymbol*>(symbol);
			/*if (funcSym->functions().size() == 1) {
				Function *func = funcSym->functions().first();
				return Value(func->functionValueType(), func->function(), false);
			}*/

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
		default:
			assert("Invalid Symbol::Type");
			return Value();
	}
}

Value FunctionCodeGenerator::generate(ast::Expression *n) {
	if (n->associativity() == ast::Expression::LeftToRight) {
		Value op1 = generate(n->firstOperand());
		for (QList<ast::ExpressionNode*>::ConstIterator i = n->operations().begin(); i != n->operations().end(); ++i) {
			ast::ExpressionNode *exprNode = *i;
			if (exprNode->op() == ast::ExpressionNode::opMember) {
				ValueType *valueType = op1.valueType();
				QString memberName;
				ast::Node *memberId = exprNode->operand();
				switch (memberId->type()) {
					case ast::Node::ntIdentifier:
						memberName = memberId->cast<ast::Identifier>()->name();
						break;
					case ast::Node::ntVariable: {
						ast::Variable *var = memberId->cast<ast::Variable>();
						memberName = var->identifier()->name();
						if (var->valueType()->type() != ast::Node::ntDefaultType) {
							ValueType *memberType = valueType->memberType(memberName);
							ValueType *resolvedType = mTypeResolver.resolve(var->valueType());
							if (resolvedType && memberType) {
								if (memberType != resolvedType) {
									emit error(ErrorCodes::ecVariableAlreadyDefinedWithAnotherType, tr("Member (\"%1\") is already defined with type \"%2\"").arg(resolvedType->name(), memberType->name()), memberId->codePoint());
								}
							}
						}
						break;

					}
					default:
						assert("Invalid ast::Node::Type" && 0);
				}

				if (!valueType->hasMember(memberName)) {
					emit error(ErrorCodes::ecCantFindField, tr("Can't find field \"%1\"").arg(memberName), memberId->codePoint());
					throw CodeGeneratorError(ErrorCodes::ecCantFindField);
				}

				op1 = valueType->member(mBuilder, op1, memberName);
			}
			else {
				Value op2 = generate(exprNode->operand());
				assert(op1.isValid());
				assert(op2.isValid());

				ValueType *valueType = op1.valueType();
				OperationFlags opFlags;

				if (op1.isConstant() ^ op2.isConstant()) {
					op1.toLLVMValue(mBuilder);
					op2.toLLVMValue(mBuilder);
				}

				Value result = valueType->generateOperation(mBuilder, exprNode->op(), op1, op2, opFlags);

				if (opFlags.testFlag(OperationFlag::MayLosePrecision)) {
					emit warning(WarningCodes::wcMayLosePrecision, tr("Operation \"%1\" may lose precision with operands of types \"%2\" and \"%3\"").arg(ast::ExpressionNode::opToString(exprNode->op()), valueType->name(), op2.valueType()->name()), exprNode->codePoint());
				}
				if (opFlags.testFlag(OperationFlag::IntegerDividedByZero)) {
					emit error(ErrorCodes::ecIntegerDividedByZero, tr("Integer divided by Zero"), exprNode->codePoint());
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

				if (opFlags.testFlag(OperationFlag::OperandBCantBeCastedToA)) {
					emit error(ErrorCodes::ecCantCastValue, tr("Can't cast \"%1\" to \"%2\"").arg(op2.valueType()->name(), op1.valueType()->name()), exprNode->codePoint());
					throw CodeGeneratorError(ErrorCodes::ecCantCastValue);
				}

				if (opFlags.testFlag(OperationFlag::NoSuchOperation)) {
					emit error(ErrorCodes::ecMathematicalOperationOperandTypeMismatch, tr("No operation \"%1\" between operands of types \"%2\" and \"%3\"").arg(ast::ExpressionNode::opToString(exprNode->op()), valueType->name(), op2.valueType()->name()), exprNode->codePoint());
					throw CodeGeneratorError(ErrorCodes::ecMathematicalOperationOperandTypeMismatch);
					return Value();
				}

				assert(result.isValid());

				op1 = result;
			}
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
				assert(op1.isValid());
				assert(op2.isValid());

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

				op = (*i)->op();
				cp = (*i)->codePoint();

				assert(result.isValid());
				op2 = result;
			} while (i != n->operations().begin());
		}
		Value op1 = generate(n->firstOperand());
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

		if (opFlags.testFlag(OperationFlag::OperandBCantBeCastedToA)) {
			emit error(ErrorCodes::ecCantCastValue, tr("Invalid operation \"%1\". Can't cast \"%3\" to \"%2\".").arg(ast::ExpressionNode::opToString(op), valueType->name(), op2.valueType()->name()), cp);
			throw CodeGeneratorError(ErrorCodes::ecMathematicalOperationOperandTypeMismatch);
		}

		if (opFlags.testFlag(OperationFlag::NoSuchOperation)) {
			emit error(ErrorCodes::ecMathematicalOperationOperandTypeMismatch, tr("No operation \"%1\" between operands of types \"%2\" and \"%3\"").arg(ast::ExpressionNode::opToString(op), valueType->name(), op2.valueType()->name()), cp);
			throw CodeGeneratorError(ErrorCodes::ecMathematicalOperationOperandTypeMismatch);
		}
		assert(result.isValid());

		return result;
	}
}

Value FunctionCodeGenerator::generate(ast::Unary *n) {
	Value val = generate(n->operand());
	OperationFlags flags;
	ValueType *valueType = val.valueType();
	Value result = val.valueType()->generateOperation(mBuilder, n->op(), val, flags);
	if (flags.testFlag(OperationFlag::MayLosePrecision)) {
		emit warning(WarningCodes::wcMayLosePrecision, tr("Operation \"%1\" may lose precision with operand of type \"%2\"").arg(ast::Unary::opToString(n->op()), valueType->name()), n->codePoint());
	}

	if (flags.testFlag(OperationFlag::CastFromString)) {
		emit warning(WarningCodes::wcMayLosePrecision, tr("Automatic cast from a string to a number."), n->codePoint());
	}

	if (flags.testFlag(OperationFlag::NoSuchOperation)) {
		emit error(ErrorCodes::ecMathematicalOperationOperandTypeMismatch, tr("No operation \"%1\" for operand of type \"%2\"").arg(ast::Unary::opToString(n->op()), valueType->name()), n->codePoint());
		throw CodeGeneratorError(ErrorCodes::ecMathematicalOperationOperandTypeMismatch);
	}

	return result;
}

Value FunctionCodeGenerator::generate(ast::FunctionCall *n) {
	Value functionValue = generate(n->function());
	ValueType *valueType = functionValue.valueType();
	QList<Value> paramValues = generateParameterList(n->parameters());

	if (functionValue.isFunctionSelectorValueType()) {
		FunctionSelectorValueType *funcSelector = static_cast<FunctionSelectorValueType*>(valueType);
		QList<Function*> functions = funcSelector->overloads();

		bool isSelector = false;

		QList<ValueType*> paramTypes;
		if (!paramValues.isEmpty() && paramValues.first().isValueType()) {
			isSelector = true;
		}
		for (const Value &val : paramValues) {
			if (isSelector && !val.isValueType()) {
				emit error(ErrorCodes::ecFunctionSelectorParametersShouldBeValueTypes, tr("All parameters of the function selector should be value types"), n->codePoint());
				throw CodeGeneratorError(ErrorCodes::ecFunctionSelectorParametersShouldBeValueTypes);
			}
			paramTypes.append(val.valueType());
		}

		Function *func = findBestOverload(functions, paramTypes, n->isCommand(), n->codePoint());

		assert(func);

		if (isSelector) {
			return Value(func->functionValueType(), func->function(), false);
		}
		else {
			return mBuilder->call(func, paramValues);
		}

	}
	else if (functionValue.isValueType()) {
		//Cast function
		if (paramValues.size() != 1) {
			emit error(ErrorCodes::ecCastFunctionRequiresOneParameter, tr("Cast functions take one parameter"), n->codePoint());
			throw CodeGeneratorError(ErrorCodes::ecCastFunctionRequiresOneParameter);
		}

		Value val = paramValues.first();
		ValueType::CastCost cc = val.valueType()->castingCostToOtherValueType(valueType);
		if (cc == ValueType::ccNoCast) {
			emit error(ErrorCodes::ecCantCastValue, tr("Can't cast \"%1\" to \"%2\"").arg(val.valueType()->name(), valueType->name()), n->codePoint());
			throw CodeGeneratorError(ErrorCodes::ecCantCastValue);
		}

		return valueType->cast(mBuilder, val);
	}
	else {
		if (!valueType->isCallable()) {
			emit error(ErrorCodes::ecCantFindFunction, tr("%1 is not a function and can't be called").arg(valueType->name()), n->codePoint());
			throw CodeGeneratorError(ErrorCodes::ecCantFindFunction);
		}
		FunctionValueType *functionValueType = static_cast<FunctionValueType*>(valueType);

		checkFunctionCallValidity(functionValueType, paramValues, n->codePoint());

		return mBuilder->call(functionValue, paramValues);
	}
}

Value FunctionCodeGenerator::generate(ast::KeywordFunctionCall *n) {
	QList<Value> paramValues = generateParameterList(n->parameters());
	if (n->keyword() == ast::KeywordFunctionCall::ArraySize) {
		if (paramValues.size() == 0 || paramValues.size() > 2) {
			emit error(ErrorCodes::ecWrongNumberOfParameters, tr("ArraySize takes 1 or 2 parameters"), n->codePoint());
			throw CodeGeneratorError(ErrorCodes::ecWrongNumberOfParameters);
		}
		const Value &array = paramValues.first();
		if (!array.valueType()->isArray()) {
			emit error(ErrorCodes::ecNotArray, tr("The first parameter of ArraySize should be an array"), n->codePoint());
			throw CodeGeneratorError(ErrorCodes::ecNotArray);
		}
		ArrayValueType *valType = static_cast<ArrayValueType*>(array.valueType());
		if (valType->dimensions() == 1) {
			if (paramValues.size() != 1) {
				emit error(ErrorCodes::ecWrongNumberOfParameters, tr("ArraySize takes only 1 parameter when first parameter is one dimensional array"), n->codePoint());
				throw CodeGeneratorError(ErrorCodes::ecWrongNumberOfParameters);
			}
			Value result = valType->dimensionSize(mBuilder, array, Value(ConstantValue(0), mRuntime));
			mBuilder->destruct(array);
			return result;
		}

		if (paramValues.size() != 2) {
			emit error(ErrorCodes::ecWrongNumberOfParameters, tr("ArraySize takes 2 parameters (an array and a dimension index) when first parameter is multidimensional array"), n->codePoint());
			throw CodeGeneratorError(ErrorCodes::ecWrongNumberOfParameters);
		}
		Value dimIndex = paramValues.last();
		if (!(dimIndex.valueType() == mRuntime->intValueType() || dimIndex.valueType() == mRuntime->shortValueType() || dimIndex.valueType() == mRuntime->byteValueType() || dimIndex.valueType() == mRuntime->booleanValueType())) {
			emit error(ErrorCodes::ecNotInteger, tr("ArraySize second parameter should be an integer. Given \"%1\"").arg(dimIndex.valueType()->name()), n->codePoint());
			throw CodeGeneratorError(ErrorCodes::ecNotInteger);
		}
		dimIndex = mBuilder->toInt(dimIndex);
		if (dimIndex.isConstant()) {
			int i = dimIndex.constant().toInt();
			if (i < 0 || i >= valType->dimensions()) {
				emit error(ErrorCodes::ecInvalidParameter, tr("Invalid array dimension index %1. Array has %2 dimensions so index should be between 1 and %3").arg(i).arg(valType->dimensions()).arg(valType->dimensions() - 1), n->codePoint());
				throw CodeGeneratorError(ErrorCodes::ecInvalidParameter);
			}
		}
		Value result = valType->dimensionSize(mBuilder, array, dimIndex);
		mBuilder->destruct(array);
		return result;
	}


	if (paramValues.size() != 1) {
		emit error(ErrorCodes::ecWrongNumberOfParameters, tr("This function takes one parameter"), n->codePoint());
		throw CodeGeneratorError(ErrorCodes::ecWrongNumberOfParameters);
	}
	const Value &param = paramValues.first();
	switch (n->keyword()) {
		case ast::KeywordFunctionCall::New: {
			if (!param.isValueType() || !param.valueType()->isTypePointer()) {
				emit error(ErrorCodes::ecNotTypeName, tr("\"New\" is expecting a type or a class as a parameter. Invalid parameter type \"%1\"").arg(param.valueType()->name()), n->codePoint());
				throw CodeGeneratorError(ErrorCodes::ecNotTypeName);
			}
			TypePointerValueType *typePointerValueType = static_cast<TypePointerValueType*>(param.valueType());
			return mBuilder->newTypeMember(typePointerValueType->typeSymbol());
		}

		case ast::KeywordFunctionCall::First: {
			if (!param.isValueType() || !param.valueType()->isTypePointer()) {
				emit error(ErrorCodes::ecNotTypeName, tr("\"First\" is expecting a type as a parameter. Invalid parameter type \"%1\"").arg(param.valueType()->name()), n->codePoint());
				throw CodeGeneratorError(ErrorCodes::ecNotTypeName);
			}
			TypePointerValueType *typePointerValueType = static_cast<TypePointerValueType*>(param.valueType());
			return mBuilder->firstTypeMember(typePointerValueType->typeSymbol());
		}
		case ast::KeywordFunctionCall::Last: {
			if (!param.isValueType() || !param.valueType()->isTypePointer()) {
				emit error(ErrorCodes::ecNotTypeName, tr("\"First\" is expecting a type as a parameter. Invalid parameter type \"%1\"").arg(param.valueType()->name()), n->codePoint());
				throw CodeGeneratorError(ErrorCodes::ecNotTypeName);
			}
			TypePointerValueType *typePointerValueType = static_cast<TypePointerValueType*>(param.valueType());
			return mBuilder->lastTypeMember(typePointerValueType->typeSymbol());
		}

		case ast::KeywordFunctionCall::Before: {
			ValueType *valueType = param.valueType();
			if (!param.isNormalValue() || !valueType->isTypePointer()) {
				emit error(ErrorCodes::ecNotTypePointer, tr("\"Before\" takes a type pointer as a parameter. Invalid parameter type \"%1\"").arg(valueType->name()), n->codePoint());
				throw CodeGeneratorError(ErrorCodes::ecNotTypePointer);
			}
			return mBuilder->beforeTypeMember(param);
		}
		case ast::KeywordFunctionCall::After: {
			ValueType *valueType = param.valueType();
			if (!param.isNormalValue() || !valueType->isTypePointer()) {
				emit error(ErrorCodes::ecNotTypePointer, tr("\"After\" takes a type pointer as a parameter. Invalid parameter type \"%1\"").arg(valueType->name()), n->codePoint());
				throw CodeGeneratorError(ErrorCodes::ecNotTypePointer);
			}
			return mBuilder->afterTypeMember(param);
		}
		case ast::KeywordFunctionCall::Delete:  {
			ValueType *valueType = param.valueType();
			if (!param.isNormalValue() || !valueType->isTypePointer()) {
				emit error(ErrorCodes::ecNotTypePointer, tr("\"Delete\" takes a type pointer as a parameter. Invalid parameter type \"%1\"").arg(valueType->name()), n->codePoint());
				throw CodeGeneratorError(ErrorCodes::ecNotTypePointer);
			}
			mBuilder->deleteTypeMember(param);
			return Value();
		}
		default:
			assert("Invalid ast::KeywordFunctionCall::KeywordFunction" && 0);
	}

	return Value();
}

Value FunctionCodeGenerator::generate(ast::ArraySubscript *n) {
	Value arr = generate(n->array());

	if (arr.isNormalValue() && arr.valueType()->isArray()) {
		ArrayValueType *valType = static_cast<ArrayValueType*>(arr.valueType());
		QList<Value> params = generateParameterList(n->subscript());
		if (params.size() != valType->dimensions()) {
			emit error(ErrorCodes::ecArrayDimensionCountDoesntMatch, tr("Invalid number of dimensions %1. The array has %n dimensions.", 0, valType->dimensions()).arg(params.size()), n->codePoint());
			throw CodeGeneratorError(ErrorCodes::ecArrayDimensionCountDoesntMatch);
		}
		int index = 1;
		for (const Value &v : params) {
			if (!(v.valueType() == mRuntime->intValueType()  || v.valueType() == mRuntime->shortValueType() || v.valueType() == mRuntime->byteValueType() || v.valueType() == mRuntime->booleanValueType())) {
				emit error(ErrorCodes::ecArraySubscriptNotInteger, tr("Array initialization parameter %1 is not an integer").arg(index), n->codePoint());
				throw CodeGeneratorError(ErrorCodes::ecArraySubscriptNotInteger);
			}
			index++;

		}
		return valType->arraySubscript(mBuilder, arr, params);
	} else {
		emit error(ErrorCodes::ecNotArray, tr("Value isn't an array and  it doesn't have subscript operator"), n->codePoint());
		throw CodeGeneratorError(ErrorCodes::ecNotArray);
	}
}

Value FunctionCodeGenerator::generate(ast::Node *n) {
	Value result;
	switch (n->type()) {
		case ast::Node::ntBlock:
			result = generate(n->cast<ast::Block>()); break;
		case ast::Node::ntInteger:
			result = generate(n->cast<ast::Integer>()); break;
		case ast::Node::ntFloat:
			result = generate(n->cast<ast::Float>()); break;
		case ast::Node::ntString:
			result = generate(n->cast<ast::String>()); break;
		case ast::Node::ntIdentifier:
			result = generate(n->cast<ast::Identifier>()); break;
		case ast::Node::ntLabel:
			result = generate(n->cast<ast::Label>()); break;
		case ast::Node::ntList:
			result = generate(n->cast<ast::List>()); break;
		case ast::Node::ntGoto:
			result = generate(n->cast<ast::Goto>()); break;
		case ast::Node::ntGosub:
			result = generate(n->cast<ast::Gosub>()); break;
		case ast::Node::ntReturn:
			result = generate(n->cast<ast::Return>()); break;
		case ast::Node::ntExit:
			result = generate(n->cast<ast::Exit>()); break;


		case ast::Node::ntExpression:
			result = generate(n->cast<ast::Expression>()); break;
		case ast::Node::ntExpressionNode:
			result = generate(n->cast<ast::ExpressionNode>()); break;
		case ast::Node::ntUnary:
			result = generate(n->cast<ast::Unary>()); break;
		case ast::Node::ntArraySubscript:
			result = generate(n->cast<ast::ArraySubscript>()); break;
		case ast::Node::ntFunctionCall:
			result = generate(n->cast<ast::FunctionCall>()); break;
		case ast::Node::ntKeywordFunctionCall:
			result = generate(n->cast<ast::KeywordFunctionCall>()); break;
		case ast::Node::ntDefaultValue:
			result = generate(n->cast<ast::DefaultValue>()); break;
		case ast::Node::ntVariable:
			result = generate(n->cast<ast::Variable>()); break;

		default:
			assert("Invalid ast::Node for generate(ast::Node*)" && 0);
			return Value();
	}
	assert(result.isValid());
	return result;
}

Function *FunctionCodeGenerator::findBestOverload(const QList<Function *> &functions, const QList<ValueType *> &parameters, bool command, const CodePoint &cp) {
	//No overloads
	if (functions.size() == 1) {
		Function *f = functions.first();

		if (!(f->isCommand() == command || (command == false && parameters.size() == 1))) {
			if (command) {
				emit error(ErrorCodes::ecNotCommand, tr("There is no command \"%1\" (but there is a function with same name)").arg(f->name()), cp);
				throw CodeGeneratorError(ErrorCodes::ecNotCommand);
			} else {
				emit error(ErrorCodes::ecCantFindFunction,  tr("There is no function \"%1\" (but there is a command with same name)").arg(f->name()), cp);
				throw CodeGeneratorError(ErrorCodes::ecCantFindFunction);
			}
		}

		if (f->requiredParams() > parameters.size() || parameters.size() > f->paramTypes().size()) {
			emit error(ErrorCodes::ecCantFindFunction, tr("Function doesn't match given parameters. \"%1\" was tried to call with parameters of types (%2)").arg(
						   f->functionValueType()->name(),
						   listStringJoin(parameters, [](ValueType *val) {
				return val->name();
			})), cp);
			throw CodeGeneratorError(ErrorCodes::ecCantFindFunction);
		}

		Function::ParamList::ConstIterator p1i = f->paramTypes().begin();
		CastCostCalculator totalCost;
		for (QList<ValueType*>::ConstIterator p2i = parameters.begin(); p2i != parameters.end(); p2i++) {
			totalCost += (*p2i)->castingCostToOtherValueType(*p1i);
			p1i++;
		}
		if (!totalCost.isCastPossible()) {
			if (command) {
				emit error(ErrorCodes::ecCantFindFunction, tr("Command doesn't match given parameters. \"%1\" was tried to call with parameters of types (%2)").arg(
							   f->functionValueType()->name(),
							   listStringJoin(parameters, [](ValueType *val) {
					return val->name();
				})), cp);
				throw CodeGeneratorError(ErrorCodes::ecCantFindCommand);
			}
			else {
				emit error(ErrorCodes::ecCantFindFunction, tr("Function doesn't match given parameters. \"%1\" was tried to call with parameters of types (%2)").arg(
							   f->functionValueType()->name(),
							   listStringJoin(parameters, [](ValueType *val) {
					return val->name();
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
		if (f->paramTypes().size() >= parameters.size() && f->requiredParams() <= parameters.size() && (f->isCommand() == command || (command == false && parameters.size() == 1))) {
			QList<ValueType*>::ConstIterator p1i = f->paramTypes().begin();
			CastCostCalculator totalCost;
			for (QList<ValueType*>::ConstIterator p2i = parameters.begin(); p2i != parameters.end(); p2i++) {
				totalCost += (*p2i)->castingCostToOtherValueType(*p1i);
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
	if (bestFunc == 0 || !bestCost.isCastPossible()) {
		if (command) {
			emit error(ErrorCodes::ecCantFindCommand, tr("Can't find a command overload which would accept given parameters (%1)").arg(
						   listStringJoin(parameters, [](ValueType *val) {
				return val->name();
			})), cp);
			throw CodeGeneratorError(ErrorCodes::ecCantFindCommand);
		}
		else {
			emit error(ErrorCodes::ecCantFindFunction, tr("Can't find a function overload which would accept given parameters (%1)").arg(
						   listStringJoin(parameters, [](ValueType *val) {
				return val->name();
			})), cp);
			throw CodeGeneratorError(ErrorCodes::ecCantFindFunction);
		}

	}
	if (multiples) {
		QString msg = command ? tr("Found multiple possible command overloads with parameters (%1) and can't choose between them.") : tr("Found multiple possible function overloads with parameters (%1) and can't choose between them.");
		emit error(ErrorCodes::ecMultiplePossibleOverloads,  msg.arg(
					   listStringJoin(parameters, [](ValueType *val) {
			return val->name();
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

void FunctionCodeGenerator::resolveGotos() {
	for (const QPair<LabelSymbol*, llvm::BasicBlock*> &g : mUnresolvedGotos) {
		assert(g.first->basicBlock());
		mBuilder->setInsertPoint(g.second);
		mBuilder->branch(g.first->basicBlock());
	}
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

llvm::BasicBlock *FunctionCodeGenerator::createBasicBlock(const llvm::Twine &name, llvm::BasicBlock *insertBefore) {
	return llvm::BasicBlock::Create(mBuilder->context(), name, mFunction, insertBefore);
}

bool FunctionCodeGenerator::checkUnreachable(CodePoint cp) {
	if (mUnreachableBasicBlock) {
		emit warning(WarningCodes::wcUnreachableCode, tr("Unreachable code"), cp);
		return true;
	}
	return false;
}

void FunctionCodeGenerator::generateFunctionParameterAssignments(const QList<CBFunction::Parameter> &parameters) {
	QList<CBFunction::Parameter>::ConstIterator pi = parameters.begin();
	for (llvm::Function::arg_iterator i = mFunction->arg_begin(); i != mFunction->arg_end(); ++i) {
		mBuilder->store(pi->mVariableSymbol, i);
		pi++;
	}
}

VariableSymbol *FunctionCodeGenerator::searchVariableSymbol(ast::Node *n) {
	ast::Identifier *id = 0;
	switch (n->type()) {
		case ast::Node::ntIdentifier:
			id = n->cast<ast::Identifier>(); break;
		case ast::Node::ntVariable:
			id = n->cast<ast::Variable>()->identifier(); break;
		default:
			assert("ast::Node not variable" && 0);
	}

	Symbol *symbol = mLocalScope->find(id->name());
	assert(symbol && symbol->type() == Symbol::stVariable);
	return static_cast<VariableSymbol*>(symbol);
}

bool FunctionCodeGenerator::checkFunctionCallValidity(FunctionValueType *funcValType, const QList<Value> &parameters, const CodePoint &cp){

	if (funcValType->paramTypes().size() != parameters.size()) {
		emit error(ErrorCodes::ecCantFindFunction, tr("Function doesn't match given parameters. \"%1\" was tried to call with parameters of types (%2)").arg(
					   funcValType->name(),
					   listStringJoin(parameters, [](const Value &val) {
			return val.valueType()->name();
		})), cp);
		throw CodeGeneratorError(ErrorCodes::ecCantFindFunction);
	}

	Function::ParamList::ConstIterator p1i = funcValType->paramTypes().begin();
	CastCostCalculator totalCost;
	for (QList<Value>::ConstIterator p2i = parameters.begin(); p2i != parameters.end(); p2i++) {
		totalCost += p2i->valueType()->castingCostToOtherValueType(*p1i);
		p1i++;
	}
	if (!totalCost.isCastPossible()) {
		emit error(ErrorCodes::ecCantFindFunction, tr("Function doesn't match given parameters. \"%1\" was tried to call with parameters of types (%2)").arg(
					   funcValType->name(),
					   listStringJoin(parameters, [](const Value &val) {
			return val.valueType()->name();
		})), cp);
		throw CodeGeneratorError(ErrorCodes::ecCantFindFunction);
	}
	return true;
}


void FunctionCodeGenerator::errorOccured(int, QString, CodePoint) {
	mValid = false;
}
