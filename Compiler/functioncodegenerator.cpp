#include "functioncodegenerator.h"
#include "labelsymbol.h"
#include "arraysymbol.h"
#include "variablesymbol.h"
#include "functionsymbol.h"
FunctionCodeGenerator::FunctionCodeGenerator(QObject *parent):
	QObject(parent),
	mCurrentBasicBlock(0),
	mIsMainScope(false),
	mFunction(0),
	mCurrentBasicBlockIt(mBasicBlocks.end()),
	mCurrentExit(mBasicBlocks.end()),
	mExprGen(this)
{
	connect(&mExprGen, &ExpressionCodeGenerator::error, this, &FunctionCodeGenerator::error);
	connect(&mExprGen, &ExpressionCodeGenerator::warning, this, &FunctionCodeGenerator::warning);
}

void FunctionCodeGenerator::setFunction(llvm::Function *func) {
	mFunction = func;
}

void FunctionCodeGenerator::setIsMainScope(bool t) {
	mIsMainScope = t;
}

void FunctionCodeGenerator::setScope(Scope *scope) {
	mScope = scope;
}

void FunctionCodeGenerator::setSetupBasicBlock(llvm::BasicBlock *bb) {
	assert(mSetupBasicBlock == 0);
	mSetupBasicBlock = bb;
}

bool FunctionCodeGenerator::generateFunctionCode(ast::Block *n) {
	assert(mFunction);
	if (!mSetupBasicBlock) {
		mSetupBasicBlock = llvm::BasicBlock::Create(mFunction->getContext(), "First basic block", mFunction);
	}
	mBasicBlocks.append(mSetupBasicBlock);
	mCurrentBasicBlock = mSetupBasicBlock;
	mCurrentBasicBlockIt = mBasicBlocks.begin();

	if (!basicBlockGenerationPass(n)) return false;

	return generate(n);
}

bool FunctionCodeGenerator::generate(ast::Node *n) {
}

bool FunctionCodeGenerator::generate(ast::IfStatement *n) {
}

bool FunctionCodeGenerator::generate(ast::AssignmentExpression *n) {
}

bool FunctionCodeGenerator::generate(ast::CommandCall *n) {
	Symbol *sym = scope()->find(n->mName);
	assert(sym->type() == Symbol::stFunctionOrCommand);




	FunctionSymbol *funcSym = static_cast<FunctionSymbol*>(sym);
	funcSym->findBestOverload()
}

bool FunctionCodeGenerator::generate(ast::RepeatForeverStatement *n) {
}

bool FunctionCodeGenerator::generate(ast::FunctionCallOrArraySubscript *n) {
}

bool FunctionCodeGenerator::generate(ast::Exit *n) {
}

bool FunctionCodeGenerator::generate(ast::Return *n) {
}

bool FunctionCodeGenerator::generate(ast::Block *n) {
	for (ast::Block::ConstIterator i = n->begin(); i != n->end(); ++i) {
		ast::Node *s = *i;
		if (!generate(s)) return false;
	}
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
			case ast::SelectStatement:
				b = basicBlockGenerationPass((ast::SelectStatement*)s); break;

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
	LabelSymbol *label = static_cast<LabelSymbol>(sym);
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
	mBasicBlocks.append(mBasicBlocks);
	mCurrentBasicBlockIt = --mBasicBlocks.end();
}
