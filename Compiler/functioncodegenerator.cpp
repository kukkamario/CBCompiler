#include "functioncodegenerator.h"
#include "labelsymbol.h"
#include "arraysymbol.h"
#include "variablesymbol.h"
#include "functionsymbol.h"
#include "builder.h"
#include <QDebug>
#include "errorcodes.h"
#include "warningcodes.h"
#include "stringpool.h"
#include "cbfunction.h"
#include "typepointervaluetype.h"
#include "typesymbol.h"

FunctionCodeGenerator::FunctionCodeGenerator(QObject *parent):
	QObject(parent),
	mCurrentBasicBlock(0),
	mFunction(0),
	mCurrentBasicBlockIt(mBasicBlocks.end()),
	mExprGen(this),
	mBuilder(0),
	mSetupBasicBlock(0),
	mReturnType(0) {
	connect(&mExprGen, SIGNAL(error(int,QString,int,QString)), this, SIGNAL(error(int,QString,int,QString)));
	connect(&mExprGen,SIGNAL(warning(int,QString,int,QString)), this, SIGNAL(warning(int,QString,int,QString)));
}

void FunctionCodeGenerator::setRuntime(Runtime *r) {
	mRuntime = r;
}

void FunctionCodeGenerator::setFunction(llvm::Function *func) {
	mFunction = func;
}

void FunctionCodeGenerator::setScope(Scope *scope) {
	mScope = scope;
	mExprGen.setScope(scope);
}

void FunctionCodeGenerator::setSetupBasicBlock(llvm::BasicBlock *bb) {
	assert(mSetupBasicBlock == 0);
	mSetupBasicBlock = bb;
}

void FunctionCodeGenerator::setBuilder(Builder *builder) {
	mBuilder = builder;
	mExprGen.setBuilder(builder);
}


void FunctionCodeGenerator::generateCBFunction(ast::FunctionDefinition *func, CBFunction *cbFunc) {
	mFunction = cbFunc->function();
	setScope(cbFunc->scope());
	mBasicBlocks.clear();
	mReturnType = cbFunc->returnValue();

	llvm::BasicBlock *start = llvm::BasicBlock::Create(mFunction->getContext(), "Start", mFunction);
	mBasicBlocks.append(start);
	mCurrentBasicBlockIt = mBasicBlocks.begin();
	mCurrentBasicBlock = *mCurrentBasicBlockIt;
	mBuilder->setInsertPoint(start);

	generateLocalVariables();

	basicBlockGenerationPass(&func->mBlock);

	mCurrentBasicBlockIt = mBasicBlocks.begin();
	mCurrentBasicBlock = *mCurrentBasicBlockIt;
	mBuilder->setInsertPoint(start);

	//Store parameters to variables
	QList<CBFunction::Parameter>::ConstIterator pI = cbFunc->parameters().begin();
	for (llvm::Function::arg_iterator argI = mFunction->arg_begin(); argI != mFunction->arg_end(); ++argI) {
		mBuilder->store(pI->mVariableSymbol, argI);
		pI++;
	}

	generate(&func->mBlock);

	mCurrentBasicBlock = mBasicBlocks.last();
	if (!mCurrentBasicBlock->getTerminator()) {
		mBuilder->setInsertPoint(mCurrentBasicBlock); //No return
		generateDestructors();
		mBuilder->returnValue(mReturnType, Value(mReturnType, mReturnType->defaultValue()));
	}
}

void FunctionCodeGenerator::generateMainScope(ast::Block *n) {
	assert(mFunction);
	mReturnType = 0;
	mBasicBlocks.clear();

	qDebug() << "Generating basic blocks";
	mSetupBasicBlock = llvm::BasicBlock::Create(mFunction->getContext(), "Setup", mFunction);
	mBasicBlocks.append(mSetupBasicBlock);

	mBasicBlocks.append(llvm::BasicBlock::Create(mFunction->getContext(), "Start", mFunction));

	basicBlockGenerationPass(n);
	qDebug() << mBasicBlocks.size() << " basic blocks created";

	mCurrentBasicBlockIt = mBasicBlocks.begin();
	mCurrentBasicBlock = *mCurrentBasicBlockIt;

	mBuilder->setInsertPoint(mCurrentBasicBlock);
	qDebug() << "Generating local variables";
	generateLocalVariables();

	nextBasicBlock();
	mBuilder->branch(mCurrentBasicBlock);
	mBuilder->setInsertPoint(mCurrentBasicBlock);

	qDebug() << "Generating code";
	generate(n);
	qDebug() << "Generating destructors";
	generateDestructors();
	qDebug() << "Generating finished";
	mBuilder->irBuilder().CreateRetVoid();

}

void FunctionCodeGenerator::generate(ast::Node *n) {
	switch(n->type()) {
		case ast::Node::ntArrayDefinition:
			return generate((ast::ArrayDefinition*)n);
		case ast::Node::ntArraySubscriptAssignmentExpression:
			return generate((ast::ArraySubscriptAssignmentExpression*)n);
		case ast::Node::ntAssignmentExpression:
			return generate((ast::AssignmentExpression*)n);
		case ast::Node::ntBlock:
			return generate((ast::Block*)n);
		case ast::Node::ntCommandCall:
			return generate((ast::CommandCall*)n);
		case ast::Node::ntExit:
			return generate((ast::Exit*)n);
		case ast::Node::ntForEachStatement:
			return generate((ast::ForEachStatement*)n);
		case ast::Node::ntForToStatement:
			return generate((ast::ForToStatement*)n);
		case ast::Node::ntFunctionCallOrArraySubscript:
			return generate((ast::FunctionCallOrArraySubscript*)n);
		case ast::Node::ntGosub:
			return generate((ast::Gosub*)n);
		case ast::Node::ntGoto:
			return generate((ast::Goto*)n);
		case ast::Node::ntIfStatement:
			return generate((ast::IfStatement*)n);
		case ast::Node::ntRedim:
			return generate((ast::Redim*)n);
		case ast::Node::ntRepeatForeverStatement:
			return generate((ast::RepeatForeverStatement*)n);
		case ast::Node::ntRepeatUntilStatement:
			return generate((ast::RepeatUntilStatement*)n);
		case ast::Node::ntReturn:
			return generate((ast::Return*)n);
		case ast::Node::ntSelectStatement:
			return generate((ast::SelectStatement*)n);
		case ast::Node::ntVariableDefinition:
			return generate((ast::VariableDefinition*)n);
		case ast::Node::ntWhileStatement:
			return generate((ast::VariableDefinition*)n);
		case ast::Node::ntLabel:
			return generate((ast::Label*)n);
		case ast::Node::ntSpecialFunctionCall:
			return generate((ast::SpecialFunctionCall*)n);
		default:
			assert(0);
	}
}

void FunctionCodeGenerator::generate(ast::IfStatement *n) {
	llvm::BasicBlock *ifBB = mCurrentBasicBlock;
	Value cond = mExprGen.generate(n->mCondition);
	assert(cond.isValid());
	//If true
	nextBasicBlock();
	llvm::BasicBlock *ifTrueBB = mCurrentBasicBlock;
	ifTrueBB->setName("If true");

	mBuilder->setInsertPoint(ifTrueBB);
	generate(&n->mIfTrue);
	llvm::BasicBlock *ifTrueEndBB = mCurrentBasicBlock;

	if (n->mElse.isEmpty()) {
		nextBasicBlock();

		//exit a if-statement
		mBuilder->branch(mCurrentBasicBlock);

		//Condition
		mBuilder->setInsertPoint(ifBB);
		mBuilder->branch(cond, ifTrueBB, mCurrentBasicBlock);
	}
	else {
		nextBasicBlock();
		llvm::BasicBlock *elseBB = mCurrentBasicBlock;
		elseBB->setName("Else");
		mBuilder->setInsertPoint(elseBB);
		generate(&n->mElse);

		mCurrentBasicBlockIt++; //EndIf
		mCurrentBasicBlock = *mCurrentBasicBlockIt;

		mBuilder->branch(mCurrentBasicBlock); //Jump over the EndIf

		mBuilder->setInsertPoint(ifBB);
		mBuilder->branch(cond, ifTrueBB, elseBB);

		mBuilder->setInsertPoint(ifTrueEndBB);
		mBuilder->branch(mCurrentBasicBlock);
	}
	mBuilder->setInsertPoint(mCurrentBasicBlock);
	mCurrentBasicBlock->setName("EndIf");
}

void FunctionCodeGenerator::generate(ast::AssignmentExpression *n) {
	Value val = mExprGen.generate(n->mExpression);
	assert(val.isValid());

	if (n->mVariable->type() == ast::Node::ntVariable) {
		Symbol *sym = mScope->find(static_cast<ast::Variable*>(n->mVariable)->mName);
		assert(sym->type() == Symbol::stVariable);
		VariableSymbol *var = static_cast<VariableSymbol*>(sym);
		mBuilder->store(var, val);
		return;
	}
	else if (n->mVariable->type() == ast::Node::ntTypePtrField) {
		ast::TypePtrField *typeField = static_cast<ast::TypePtrField *>(n->mVariable);

		Symbol *sym = mScope->find(typeField->mVariableName);
		assert(sym && sym->type() == Symbol::stVariable);
		VariableSymbol *varSym = static_cast<VariableSymbol*>(sym);

		mBuilder->store(varSym, typeField->mFieldName, val);
		return;
	}
	assert(0); //Should never happen
	return;
}

void FunctionCodeGenerator::generate(ast::CommandCall *n) {
	Symbol *sym = scope()->find(n->mName);
	assert(sym->type() == Symbol::stFunctionOrCommand);

	QList<ValueType*> valueTypes;
	QList<Value> params;
	for (QList<ast::Node*>::ConstIterator i = n->mParams.begin();i != n->mParams.end(); i++) {
		Value p = mExprGen.generate(*i);
		valueTypes.append(p.valueType());
		params.append(p);
	}

	FunctionSymbol *funcSym = static_cast<FunctionSymbol*>(sym);
	Function *command = funcSym->findBestOverload(valueTypes, true);
	assert(command);

	mBuilder->call(command, params);
}

void FunctionCodeGenerator::generate(ast::RepeatForeverStatement *n) {
	pushExit(mExitLocations[n]);
	nextBasicBlock();
	mBuilder->branch(mCurrentBasicBlock);
	mBuilder->setInsertPoint(mCurrentBasicBlock);

	llvm::BasicBlock *repeatBlock = mCurrentBasicBlock;
	repeatBlock->setName("Repeat");
	generate(&n->mBlock);
	mBuilder->branch(repeatBlock);
	nextBasicBlock();
	mBuilder->setInsertPoint(mCurrentBasicBlock);
	mCurrentBasicBlock->setName("Forever");
	popExit();
}

void FunctionCodeGenerator::generate(ast::FunctionCallOrArraySubscript *n) { // Function or command call
	Symbol *sym = scope()->find(n->mName);
	if (sym->type() == Symbol::stArray) {
		emit warning(WarningCodes::wcUselessLineIgnored, tr("Ignored useless array \"%1\" subscript"), n->mLine, n->mFile);
		return;
	}
	assert(sym->type() == Symbol::stFunctionOrCommand);

	QList<ValueType*> valueTypes;
	QList<Value> params;
	for (QList<ast::Node*>::ConstIterator i = n->mParams.begin();i != n->mParams.end(); i++) {
		Value p = mExprGen.generate(*i);
		assert(p.isValid());
		valueTypes.append(p.valueType());
		params.append(p);
	}

	FunctionSymbol *funcSym = static_cast<FunctionSymbol*>(sym);
	Function *func = funcSym->findBestOverload(valueTypes, false); //Find function
	if (!func) {
		func = funcSym->findBestOverload(valueTypes, true); //Find command
	}
	assert(func);

	Value val = mBuilder->call(func, params);
	mBuilder->destruct(val);
	return;
}

void FunctionCodeGenerator::generate(ast::Exit *n) {
	mBuilder->branch(currentExit());
	nextBasicBlock();
	mBuilder->setInsertPoint(mCurrentBasicBlock);
	mCurrentBasicBlock->setName("Useless branch after exit");
	return;
}

void FunctionCodeGenerator::generate(ast::Return *n) {
	if (mReturnType) {
		generateDestructors();
		mBuilder->returnValue(mReturnType, mExprGen.generate(n->mValue));
		nextBasicBlock();
		mBuilder->setInsertPoint(mCurrentBasicBlock);
		return;
	}
	assert("TODO: Implement gosub-return" && 0);
}

void FunctionCodeGenerator::generate(ast::Block *n) {
	for (ast::Block::ConstIterator i = n->begin(); i != n->end(); ++i) {
		ast::Node *s = *i;
		generate(s);
	}
}

void FunctionCodeGenerator::generate(ast::ArrayDefinition *n) {
	Symbol *sym = mScope->find(n->mName);
	assert(sym && sym->type() == Symbol::stArray);
	ArraySymbol *array = static_cast<ArraySymbol*>(sym);

	QList<Value> dimensions = mExprGen.generateParameterList(n->mDimensions);

	array->createGlobalVariables(mBuilder);
	mBuilder->initilizeArray(array, dimensions);
}

void FunctionCodeGenerator::generate(ast::ArraySubscriptAssignmentExpression *n) {
	Symbol *sym = mScope->find(n->mArrayName);
	assert(sym && sym->type() == Symbol::stArray);
	ArraySymbol *array = static_cast<ArraySymbol*>(sym);

	QList<Value> dims = mExprGen.generateParameterList(n->mSubscripts);

	Value value = mExprGen.generate(n->mValue);
	assert(value.isValid());
	mBuilder->store(array, dims, value);
}

void FunctionCodeGenerator::generate(ast::SelectStatement *n) {
	assert(0); return;
}

void FunctionCodeGenerator::generate(ast::ForEachStatement *n) {
	pushExit(n);
	Symbol *containerSym = mScope->find(n->mContainer);
	assert(containerSym);
	if (containerSym->type() == Symbol::stType) {
		TypeSymbol *typeSymbol = static_cast<TypeSymbol*>(containerSym);

		Symbol *sym = mScope->find(n->mVarName);
		assert(sym && sym->type() == Symbol::stVariable);
		VariableSymbol *variableSymbol = static_cast<VariableSymbol*>(sym);

		Value firstTypeMember = mBuilder->firstTypeMember(typeSymbol);
		mBuilder->store(variableSymbol, firstTypeMember);

		nextBasicBlock();
		mBuilder->branch(mCurrentBasicBlock);
		mBuilder->setInsertPoint(mCurrentBasicBlock);
		mCurrentBasicBlock->setName("For-Each condition");

		llvm::BasicBlock *conditionBlock = mCurrentBasicBlock;

		nextBasicBlock();
		mBuilder->setInsertPoint(mCurrentBasicBlock);
		llvm::BasicBlock *block = mCurrentBasicBlock;

		generate(&n->mBlock);

		//Next
		mBuilder->store(variableSymbol, mBuilder->afterTypeMember(mBuilder->load(variableSymbol)));
		mBuilder->branch(conditionBlock);

		nextBasicBlock();
		mCurrentBasicBlock->setName("After For-Each Next");

		//Create condition
		mBuilder->setInsertPoint(conditionBlock);
		Value cond = mBuilder->typePointerNotNull(mBuilder->load(variableSymbol));
		mBuilder->branch(cond, block, mCurrentBasicBlock);

		mBuilder->setInsertPoint(mCurrentBasicBlock);
	}
	else if (containerSym->type() == Symbol::stArray) {
		assert("STUB" && 0);
		ArraySymbol *array = static_cast<ArraySymbol*>(containerSym);
	}
	popExit();
}

void FunctionCodeGenerator::generate(ast::ForToStatement *n) {
	pushExit(mExitLocations[n]);

	Symbol *sym = mScope->find(n->mVarName);
	assert(sym);
	assert(sym->type() == Symbol::stVariable);
	VariableSymbol *var = (VariableSymbol*)sym;
	mBuilder->store(var, mExprGen.generate(n->mFrom));
	nextBasicBlock();
	llvm::BasicBlock *start = mCurrentBasicBlock;
	mBuilder->branch(mCurrentBasicBlock);
	mBuilder->setInsertPoint(mCurrentBasicBlock);

	mCurrentBasicBlock->setName("For-To condition");
	Value val = mBuilder->load(var);
	Value add = mExprGen.generate(n->mTo);
	assert(add.isValid());
	Value cond = mBuilder->lessEqual(val, add);
	assert(cond.isValid());
	//Branch

	nextBasicBlock();
	llvm::BasicBlock *ifTrue = mCurrentBasicBlock;
	mBuilder->setInsertPoint(mCurrentBasicBlock);
	mCurrentBasicBlock->setName("For-To block");
	generate(&n->mBlock);

	val = mBuilder->load(var);
	Value step;
	if (n->mStep) {
		step = mExprGen.generate(n->mStep);
	}
	else {
		step = Value(ConstantValue(1), mRuntime);
	}
	
	assert(step.isValid());
	val = mBuilder->add(val, step);
	mBuilder->store(var, val);
	mBuilder->branch(start);

	nextBasicBlock();

	//Branch
	mBuilder->setInsertPoint(start);
	mBuilder->branch(cond, ifTrue, mCurrentBasicBlock);

	mBuilder->setInsertPoint(mCurrentBasicBlock);
	mCurrentBasicBlock->setName("after Next");
	popExit();
}

void FunctionCodeGenerator::generate(ast::Goto *n) {
	Symbol *sym = mScope->find(n->mLabel);
	assert(sym);
	assert(sym->type() == Symbol::stLabel);
	LabelSymbol *label = static_cast<LabelSymbol*>(sym);
	mBuilder->branch(label->basicBlock());

	nextBasicBlock();
	mBuilder->setInsertPoint(mCurrentBasicBlock);
}

void FunctionCodeGenerator::generate(ast::Gosub *n) {
	assert(0); return;
}

void FunctionCodeGenerator::generate(ast::RepeatUntilStatement *n) {
	pushExit(mExitLocations[n]);
	nextBasicBlock();
	mBuilder->branch(mCurrentBasicBlock);
	mBuilder->setInsertPoint(mCurrentBasicBlock);
	mCurrentBasicBlock->setName("While");
	llvm::BasicBlock *start = mCurrentBasicBlock;
	generate(&n->mBlock);

	Value cond = mExprGen.generate(n->mCondition);
	assert(cond.isValid());
	nextBasicBlock();
	mBuilder->branch(cond, mCurrentBasicBlock, start);
	mBuilder->setInsertPoint(mCurrentBasicBlock);
	mCurrentBasicBlock->setName("Wend");
	popExit();
}

void FunctionCodeGenerator::generate(ast::WhileStatement *n) {
	pushExit(mExitLocations[n]);
	nextBasicBlock();
	mBuilder->branch(mCurrentBasicBlock);
	mBuilder->setInsertPoint(mCurrentBasicBlock);
	mCurrentBasicBlock->setName("While condition");
	llvm::BasicBlock *start = mCurrentBasicBlock;
	Value cond = mExprGen.generate(n->mCondition);
	//
	nextBasicBlock();
	llvm::BasicBlock *ifTrue = mCurrentBasicBlock;
	mBuilder->setInsertPoint(mCurrentBasicBlock);
	mCurrentBasicBlock->setName("While block");
	generate(&n->mBlock);
	mBuilder->branch(start);
	nextBasicBlock();

	mBuilder->setInsertPoint(start);
	mBuilder->branch(cond, ifTrue, mCurrentBasicBlock); //While check
	mBuilder->setInsertPoint(mCurrentBasicBlock);
	mCurrentBasicBlock->setName("Wend");
	popExit();

	return;
}

void FunctionCodeGenerator::generate(ast::VariableDefinition *n) {
	//Empty?
	return;
}

void FunctionCodeGenerator::generate(ast::Redim *n) {
	assert(0); return;
}

void FunctionCodeGenerator::generate(ast::Label *n) {
	nextBasicBlock();
	mBuilder->branch(mCurrentBasicBlock);
	mCurrentBasicBlock->setName(("LABEL " + n->mName).toStdString());
	mBuilder->setInsertPoint(mCurrentBasicBlock);
	return;
}

void FunctionCodeGenerator::generate(ast::SpecialFunctionCall *n) {
	mExprGen.generate(n);
}

void FunctionCodeGenerator::basicBlockGenerationPass(ast::Block *n) {
	for (ast::Block::ConstIterator i = n->begin(); i != n->end(); ++i) {
		ast::Node *s = *i;
		assert(s);
		switch(s->type()) {
			case ast::Node::ntIfStatement:
				basicBlockGenerationPass((ast::IfStatement*)s); break;
			case ast::Node::ntWhileStatement:
				basicBlockGenerationPass((ast::WhileStatement*)s); break;
			case ast::Node::ntRepeatForeverStatement:
				basicBlockGenerationPass((ast::RepeatForeverStatement*)s); break;
			case ast::Node::ntRepeatUntilStatement:
				basicBlockGenerationPass((ast::RepeatUntilStatement*)s); break;
			case ast::Node::ntForToStatement:
				basicBlockGenerationPass((ast::ForToStatement*)s); break;
			case ast::Node::ntForEachStatement:
				basicBlockGenerationPass((ast::ForEachStatement*)s); break;
			case ast::Node::ntLabel:
				basicBlockGenerationPass((ast::Label*)s); break;
			case ast::Node::ntGosub:
			case ast::Node::ntGoto:
			case ast::Node::ntExit:
			case ast::Node::ntReturn:
				addBasicBlock(); break;
			case ast::Node::ntSelectStatement:
				basicBlockGenerationPass((ast::SelectStatement*)s); break;
			default: continue;
		}
	}
}

void FunctionCodeGenerator::basicBlockGenerationPass(ast::IfStatement *n) {
	addBasicBlock(); //If true
	basicBlockGenerationPass(&n->mIfTrue);
	if (!n->mElse.isEmpty()) {
		addBasicBlock(); //Else
		basicBlockGenerationPass(&n->mElse);
	}

	addBasicBlock(); //EndIf
}

void FunctionCodeGenerator::basicBlockGenerationPass(ast::ForToStatement *n) {
	addBasicBlock();
	addBasicBlock();
	basicBlockGenerationPass(&n->mBlock);
	addBasicBlock();
	mExitLocations.insert(n, mCurrentBasicBlockIt);
}

void FunctionCodeGenerator::basicBlockGenerationPass(ast::ForEachStatement *n) {
	addBasicBlock();
	addBasicBlock();
	basicBlockGenerationPass(&n->mBlock);
	addBasicBlock();
	mExitLocations.insert(n, mCurrentBasicBlockIt);
}

void FunctionCodeGenerator::basicBlockGenerationPass(ast::RepeatUntilStatement *n) {
	addBasicBlock();
	basicBlockGenerationPass(&n->mBlock);
	addBasicBlock();
	mExitLocations.insert(n, mCurrentBasicBlockIt);
}

void FunctionCodeGenerator::basicBlockGenerationPass(ast::RepeatForeverStatement *n) {
	addBasicBlock();
	basicBlockGenerationPass(&n->mBlock);
	addBasicBlock();
	mExitLocations.insert(n, mCurrentBasicBlockIt);
}

void FunctionCodeGenerator::basicBlockGenerationPass(ast::Label *n) {
	addBasicBlock();
	Symbol *sym = scope()->find(n->mName);
	assert(sym && sym->type() == Symbol::stLabel);
	LabelSymbol *label = static_cast<LabelSymbol*>(sym);
	label->setBasicBlock(mCurrentBasicBlock);
}

void FunctionCodeGenerator::basicBlockGenerationPass(ast::WhileStatement *n) {
	addBasicBlock();
	addBasicBlock();
	basicBlockGenerationPass(&n->mBlock);
	addBasicBlock();
	mExitLocations.insert(n, mCurrentBasicBlockIt);
}

void FunctionCodeGenerator::basicBlockGenerationPass(ast::SelectStatement *n) {
	assert("STUB" && 0);
}


void FunctionCodeGenerator::addBasicBlock() {
	mCurrentBasicBlock = llvm::BasicBlock::Create(mFunction->getContext(), "BasicBlock", mFunction);
	assert(mCurrentBasicBlock);
	mBasicBlocks.append(mCurrentBasicBlock);
	mCurrentBasicBlockIt = mBasicBlocks.end();
	--mCurrentBasicBlockIt;
}

void FunctionCodeGenerator::generateLocalVariables() {
	for (Scope::ConstIterator i = mScope->begin(); i != mScope->end(); ++i) {
		Symbol *sym = i.value();
		if (sym->type() == Symbol::stVariable) {
			mBuilder->construct(static_cast<VariableSymbol*>(sym));
		}
	}
}

void FunctionCodeGenerator::generateDestructors() {
	for (Scope::ConstIterator i = mScope->begin(); i != mScope->end(); ++i) {
		Symbol *sym = i.value();
		if (sym->type() == Symbol::stVariable) {
			mBuilder->destruct(static_cast<VariableSymbol*>(sym));
		}
	}
}

void FunctionCodeGenerator::nextBasicBlock() {
	++mCurrentBasicBlockIt;
	assert(mCurrentBasicBlockIt != mBasicBlocks.end());
	mCurrentBasicBlock = *mCurrentBasicBlockIt;
}
