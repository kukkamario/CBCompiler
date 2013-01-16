#include "functioncodegenerator.h"
#include "labelsymbol.h"
#include "arraysymbol.h"
#include "variablesymbol.h"
#include "functionsymbol.h"
#include "builder.h"
#include <QDebug>
FunctionCodeGenerator::FunctionCodeGenerator(QObject *parent):
	QObject(parent),
	mCurrentBasicBlock(0),
	mIsMainScope(false),
	mFunction(0),
	mCurrentBasicBlockIt(mBasicBlocks.end()),
	mCurrentExit(mBasicBlocks.end()),
	mExprGen(this),
	mBuilder(0),
	mSetupBasicBlock(0){
	connect(&mExprGen, SIGNAL(error(int,QString,int,QFile*)), this, SIGNAL(error(int,QString,int,QFile*)));
	connect(&mExprGen,SIGNAL(warning(int,QString,int,QFile*)), this, SIGNAL(warning(int,QString,int,QFile*)));
}

void FunctionCodeGenerator::setRuntime(Runtime *r) {
	mRuntime = r;
}

void FunctionCodeGenerator::setFunction(llvm::Function *func) {
	mFunction = func;
}

void FunctionCodeGenerator::setIsMainScope(bool t) {
	mIsMainScope = t;
}

void FunctionCodeGenerator::setScope(Scope *scope) {
	mScope = scope;
	mExprGen.setScope(scope);
}

void FunctionCodeGenerator::setSetupBasicBlock(llvm::BasicBlock *bb) {
	assert(mSetupBasicBlock == 0);
	mSetupBasicBlock = bb;
}

bool FunctionCodeGenerator::generateFunctionCode(ast::Block *n) {
	assert(mFunction);

	if (!mBuilder) {
		mBuilder = new Builder(mFunction->getContext());
		mExprGen.setBuilder(mBuilder);
		mBuilder->setRuntime(mRuntime);
	}

	if (!mSetupBasicBlock) {
		mSetupBasicBlock = llvm::BasicBlock::Create(mFunction->getContext(), "First basic block", mFunction);
	}
	mBasicBlocks.append(mSetupBasicBlock);
	mCurrentBasicBlock = mSetupBasicBlock;
	mCurrentBasicBlockIt = mBasicBlocks.begin();
	qDebug() << "Generating basic blocks";

	if (!basicBlockGenerationPass(n)) return false;

	mCurrentBasicBlock = mBasicBlocks.first();
	mCurrentBasicBlockIt = mBasicBlocks.begin();

	mBuilder->setInsertPoint(mCurrentBasicBlock);
	qDebug() << "Generating local variables";
	if (!generateLocalVariables()) return false;

	qDebug() << "Generating code";
	if (!generate(n)) return false;
	qDebug() << "Generating destructors";
	if (!generateDestructors()) return false;
	qDebug() << "Generating finished";

	if (mIsMainScope) mBuilder->irBuilder().CreateRetVoid();
	return true;
}

bool FunctionCodeGenerator::generate(ast::Node *n) {
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
	}
	assert(0);
	return false;
}

bool FunctionCodeGenerator::generate(ast::IfStatement *n) {
	llvm::BasicBlock *ifBB = mCurrentBasicBlock;
	Value cond = mExprGen.generate(n->mCondition);

	//If true
	mCurrentBasicBlockIt++;
	mCurrentBasicBlock = *mCurrentBasicBlockIt;
	llvm::BasicBlock *ifTrueBB = mCurrentBasicBlock;

	mBuilder->setInsertPoint(ifTrueBB);
	if (!generate(&n->mIfTrue)) return false;

	if (n->mElse.isEmpty()) {
		mCurrentBasicBlockIt++;
		mCurrentBasicBlock = *mCurrentBasicBlockIt;

		//exit a if-statement
		mBuilder->branch(mCurrentBasicBlock);

		//Condition
		mBuilder->setInsertPoint(ifBB);
		mBuilder->branch(cond, ifTrueBB, mCurrentBasicBlock);
	}
	else {
		mCurrentBasicBlockIt++;
		mCurrentBasicBlock = *mCurrentBasicBlockIt;
		llvm::BasicBlock *elseBB = mCurrentBasicBlock;
		mBuilder->setInsertPoint(elseBB);
		if (!generate(&n->mElse)) return false;

		mCurrentBasicBlockIt++; //EndIf
		mCurrentBasicBlock = *mCurrentBasicBlockIt;

		mBuilder->branch(mCurrentBasicBlock); //Jump over the EndIf

		mBuilder->setInsertPoint(ifBB);
		mBuilder->branch(cond, ifTrueBB, elseBB);

		mBuilder->setInsertPoint(ifTrueBB);
		mBuilder->branch(mCurrentBasicBlock);
	}
	mBuilder->setInsertPoint(mCurrentBasicBlock);
	return true;
}

bool FunctionCodeGenerator::generate(ast::AssignmentExpression *n) {
	Value val = mExprGen.generate(n->mExpression);
	if (!val.isValid()) return false;

	if (n->mVariable->type() == ast::Node::ntVariable) {
		Symbol *sym = mScope->find(static_cast<ast::Variable*>(n->mVariable)->mName);
		assert(sym->type() == Symbol::stVariable);
		VariableSymbol *var = static_cast<VariableSymbol*>(sym);
		mBuilder->store(var, val);
		return true;
	}
	else if (n->mVariable->type() == ast::Node::ntTypePtrField) {
		assert(0);
	}
	else {
		assert(0); //Should never happen
	}
	return false;
}

bool FunctionCodeGenerator::generate(ast::CommandCall *n) {
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

	command->call(mBuilder, params);
	return true;
}

bool FunctionCodeGenerator::generate(ast::RepeatForeverStatement *n) {
	assert(0); return false;
}

bool FunctionCodeGenerator::generate(ast::FunctionCallOrArraySubscript *n) {
	assert(0); return false;
}

bool FunctionCodeGenerator::generate(ast::Exit *n) {
	assert(0); return false;
}

bool FunctionCodeGenerator::generate(ast::Return *n) {
	assert(0); return false;
}

bool FunctionCodeGenerator::generate(ast::Block *n) {
	for (ast::Block::ConstIterator i = n->begin(); i != n->end(); ++i) {
		ast::Node *s = *i;
		if (!generate(s)) return false;
	}
	return true;
}

bool FunctionCodeGenerator::generate(ast::ArrayDefinition *n) {
	assert(0); return false;
}

bool FunctionCodeGenerator::generate(ast::ArraySubscriptAssignmentExpression *n) {
	assert(0); return false;
}

bool FunctionCodeGenerator::generate(ast::SelectStatement *n) {
	assert(0); return false;
}

bool FunctionCodeGenerator::generate(ast::ForEachStatement *n) {
	assert(0); return false;
}

bool FunctionCodeGenerator::generate(ast::ForToStatement *n) {
	assert(0); return false;
}

bool FunctionCodeGenerator::generate(ast::Goto *n) {
	Symbol *sym = mScope->find(n->mLabel);
	assert(sym);
	assert(sym->type() == Symbol::stLabel);
	LabelSymbol *label = static_cast<LabelSymbol*>(sym);
	mBuilder->branch(label->basicBlock());

	mCurrentBasicBlockIt++;
	mCurrentBasicBlock = *mCurrentBasicBlockIt;
	return true;
}

bool FunctionCodeGenerator::generate(ast::Gosub *n) {
	assert(0); return false;
}

bool FunctionCodeGenerator::generate(ast::RepeatUntilStatement *n) {
	assert(0); return false;
}

bool FunctionCodeGenerator::generate(ast::WhileStatement *n) {
	assert(0); return false;
}

bool FunctionCodeGenerator::generate(ast::VariableDefinition *n) {
	//Empty?
	return true;
}

bool FunctionCodeGenerator::generate(ast::Redim *n) {
	assert(0); return false;
}

bool FunctionCodeGenerator::generate(ast::Label *n) {
	mCurrentBasicBlockIt++;
	mBuilder->branch(*mCurrentBasicBlockIt);
	mCurrentBasicBlock = *mCurrentBasicBlockIt;
	mBuilder->setInsertPoint(mCurrentBasicBlock);
	return true;
}

bool FunctionCodeGenerator::basicBlockGenerationPass(ast::Block *n) {
	bool valid = true;
	for (ast::Block::ConstIterator i = n->begin(); i != n->end(); ++i) {
		ast::Node *s = *i;
		assert(s);
		bool v = true;
		switch(s->type()) {
			case ast::Node::ntIfStatement:
				v = basicBlockGenerationPass((ast::IfStatement*)s); break;
			case ast::Node::ntWhileStatement:
				v = basicBlockGenerationPass((ast::WhileStatement*)s); break;
			case ast::Node::ntRepeatForeverStatement:
				v = basicBlockGenerationPass((ast::RepeatForeverStatement*)s); break;
			case ast::Node::ntRepeatUntilStatement:
				v = basicBlockGenerationPass((ast::RepeatUntilStatement*)s); break;
			case ast::Node::ntForToStatement:
				v = basicBlockGenerationPass((ast::ForToStatement*)s); break;
			case ast::Node::ntForEachStatement:
				v = basicBlockGenerationPass((ast::ForEachStatement*)s); break;
			case ast::Node::ntLabel:
				v = basicBlockGenerationPass((ast::Label*)s); break;
			case ast::Node::ntGosub:
			case ast::Node::ntGoto:
				addBasicBlock(); break;
			case ast::Node::ntSelectStatement:
				v = basicBlockGenerationPass((ast::SelectStatement*)s); break;

		}
		if (!v) valid = false;
	}

	return valid;
}

bool FunctionCodeGenerator::basicBlockGenerationPass(ast::IfStatement *n) {
	addBasicBlock(); //If true
	bool valid = basicBlockGenerationPass(&n->mIfTrue);
	if (!n->mElse.isEmpty()) {
		addBasicBlock(); //Else
		if (!basicBlockGenerationPass(&n->mElse)) valid = false;
	}

	addBasicBlock(); //EndIf

	return valid;
}

bool FunctionCodeGenerator::basicBlockGenerationPass(ast::ForToStatement *n) {
	addBasicBlock();
	bool valid = basicBlockGenerationPass(&n->mBlock);
	addBasicBlock();
	mExitLocations.insert(n, mCurrentBasicBlockIt);
	return valid;
}

bool FunctionCodeGenerator::basicBlockGenerationPass(ast::ForEachStatement *n) {
	addBasicBlock();
	bool valid = basicBlockGenerationPass(&n->mBlock);
	addBasicBlock();
	mExitLocations.insert(n, mCurrentBasicBlockIt);
	return valid;
}

bool FunctionCodeGenerator::basicBlockGenerationPass(ast::RepeatUntilStatement *n) {
	addBasicBlock();
	bool valid = basicBlockGenerationPass(&n->mBlock);
	addBasicBlock();
	mExitLocations.insert(n, mCurrentBasicBlockIt);
	return valid;
}

bool FunctionCodeGenerator::basicBlockGenerationPass(ast::RepeatForeverStatement *n) {
	addBasicBlock();
	bool valid = basicBlockGenerationPass(&n->mBlock);
	addBasicBlock();
	mExitLocations.insert(n, mCurrentBasicBlockIt);
	return valid;
}

bool FunctionCodeGenerator::basicBlockGenerationPass(ast::Label *n) {
	addBasicBlock();
	Symbol *sym = scope()->find(n->mName);
	assert(sym && sym->type() == Symbol::stLabel);
	LabelSymbol *label = static_cast<LabelSymbol*>(sym);
	label->setBasicBlock(mCurrentBasicBlock);
	return true;
}

bool FunctionCodeGenerator::basicBlockGenerationPass(ast::WhileStatement *n) {
	addBasicBlock();
	bool valid = basicBlockGenerationPass(&n->mBlock);
	addBasicBlock();
	mExitLocations.insert(n, mCurrentBasicBlockIt);
	return valid;
}

bool FunctionCodeGenerator::basicBlockGenerationPass(ast::SelectStatement *n) {
	assert("STUB" && 0);
	return false;
}

void FunctionCodeGenerator::addBasicBlock() {
	mCurrentBasicBlock = llvm::BasicBlock::Create(mFunction->getContext(), "First basic block", mFunction);
	assert(mCurrentBasicBlock);
	mBasicBlocks.append(mCurrentBasicBlock);
	mCurrentBasicBlockIt = mBasicBlocks.end();
	--mCurrentBasicBlockIt;
}

bool FunctionCodeGenerator::generateLocalVariables() {
	for (Scope::ConstIterator i = mScope->begin(); i != mScope->end(); ++i) {
		Symbol *sym = i.value();
		if (sym->type() == Symbol::stVariable) {
			mBuilder->construct(static_cast<VariableSymbol*>(sym));
		}
	}
	return true;
}

bool FunctionCodeGenerator::generateDestructors() {
	for (Scope::ConstIterator i = mScope->begin(); i != mScope->end(); ++i) {
		Symbol *sym = i.value();
		if (sym->type() == Symbol::stVariable) {
			mBuilder->destruct(static_cast<VariableSymbol*>(sym));
		}
	}
	return true;
}
