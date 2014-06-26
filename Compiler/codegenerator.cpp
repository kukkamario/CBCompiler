#include "codegenerator.h"
#include "errorcodes.h"
#include "constantsymbol.h"
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QDir>
#include <QCoreApplication>
#include "intvaluetype.h"
#include "floatvaluetype.h"
#include "stringvaluetype.h"
#include "shortvaluetype.h"
#include "bytevaluetype.h"
#include "booleanvaluetype.h"
#include "typesymbol.h"
#include "typepointervaluetype.h"
#include "functionsymbol.h"
#include "cbfunction.h"
#include <fstream>
#include "cbfunction.h"
#include "variablesymbol.h"
#include "valuetypesymbol.h"
#include "customvaluetype.h"
#include <llvm/Assembly/AssemblyAnnotationWriter.h>


#ifndef M_PI
	#define M_PI 3.14159265358979323846
#endif

CodeGenerator::CodeGenerator(QObject *parent) :
	QObject(parent),
	mSymbolCollector(&mRuntime, &mSettings),
	mGlobalScope("Global"),
	mMainScope("Main", &mGlobalScope),
	mFuncCodeGen(&mRuntime, &mSettings),
	mBuilder(0)
{
	connect(&mRuntime, &Runtime::error, this, &CodeGenerator::error);
	connect(&mRuntime, &Runtime::warning, this, &CodeGenerator::warning);
	connect(&mSymbolCollector, &SymbolCollector::error, this, &CodeGenerator::error);
	connect(&mSymbolCollector, &SymbolCollector::warning, this, &CodeGenerator::warning);
	connect(&mFuncCodeGen, &FunctionCodeGenerator::error, this, &CodeGenerator::error);
	connect(&mFuncCodeGen, &FunctionCodeGenerator::warning, this, &CodeGenerator::warning);

	addPredefinedConstantSymbols();
}

bool CodeGenerator::initialize(const Settings &settings) {
	mSettings = settings;
	if (!mRuntime.load(&mStringPool, settings)) {
		emit error(ErrorCodes::ecInvalidRuntime, tr("Runtime loading failed"), CodePoint());
		return false;
	}
	addValueTypesToGlobalScope();

	createBuilder();
	return addRuntimeFunctions();
}

bool CodeGenerator::generate(ast::Program *program) {
	qDebug() << "Preparing code generation...";
	qDebug() << "Collecting symbols...";
	if (!mSymbolCollector.collect(program, &mGlobalScope, &mMainScope)) {
		qDebug() << "Failed";
		return false;
	}
	qDebug() << "Generating types...";
	if (!generateTypes(program)) {
		qDebug() << "Failed";
		return false;
	}
	qDebug() << "Generating global variables...";
	if (!generateGlobalVariables()) {
		qDebug() << "Failed";
		return false;
	}
	qDebug() << "Generating function definitions...";
	generateFunctionDefinitions(program->functionDefinitions());


#ifdef DEBUG_OUTPUT
	QFile file("scopes.txt");
	if (file.open(QFile::WriteOnly)) {
		QTextStream s(&file);
		mGlobalScope.writeToStream(s);
		file.close();
	}
	else {
		qCritical() << "Can't open scopes.txt";
	}
#endif

	qDebug() << "Starting code generation";
	qDebug() << "Generating main scope...";
	if (!generateMainScope(program->mainBlock())) {
		qDebug() << "Failed";
		return false;
	}
	qDebug() << "Generating functions...";
	if (!generateFunctions(program->functionDefinitions())) {
		qDebug() << "Failed";
		return false;
	}

	qDebug() << "Generating initializers...";
	generateInitializers();
	qDebug() << "Finished code generation \n\n";
	return true;
}

bool CodeGenerator::createExecutable(const QString &path) {
	std::string errorInfo;
	std::string fileOpenErrorInfo;
	if (llvm::verifyModule(*mRuntime.module(), llvm::ReturnStatusAction, &errorInfo)) { //Invalid module
		llvm::AssemblyAnnotationWriter asmAnnoWriter;
		llvm::raw_fd_ostream out("verifier.log", fileOpenErrorInfo);
		out << errorInfo;
		out << "\n\n\n-----LLVM-IR-----\n\n\n";
		mRuntime.module()->print(out, &asmAnnoWriter);
		out.close();
		qDebug("Invalid module. See verifier.log");
		return false;
	}

	QString p = QDir::currentPath();
	QDir::setCurrent(QCoreApplication::applicationDirPath());

	llvm::raw_fd_ostream bitcodeFile("raw_bitcode.bc", fileOpenErrorInfo, llvm::sys::fs::F_Binary);
	if (fileOpenErrorInfo.empty()) {
		llvm::WriteBitcodeToFile(mRuntime.module(), bitcodeFile);
		bitcodeFile.close();
	}
	else {
		emit error(ErrorCodes::ecCantWriteBitcodeFile, tr("Can't write bitcode file \"raw_bitcode.bc\""), CodePoint());
		QDir::setCurrent(p);
		return false;
	}
	qDebug() << "Optimizing bitcode...\n";
	if (!mSettings.callOpt("raw_bitcode.bc", "optimized_bitcode.bc")) return false;
	qDebug() << "Creating native assembly...\n";
	if (!mSettings.callLLC("optimized_bitcode.bc", "llc")) return false;
	qDebug() << "Building binary...\n";

	QFileInfo fi;
	fi.setFile(QDir(p), path);

	if (!mSettings.callLinker("llc", fi.absoluteFilePath())) return false;
	qDebug() << "Success\n";
	QDir::setCurrent(p);
	return true;
}

bool CodeGenerator::addRuntimeFunctions() {
	const QList<RuntimeFunction*> runtimeFunctions = mRuntime.functions();
	for (QList<RuntimeFunction*>::ConstIterator i = runtimeFunctions.begin(); i != runtimeFunctions.end(); i ++)  {
		RuntimeFunction* func = *i;

		Symbol *sym;
		FunctionSymbol *funcSym;
		if ((sym = mGlobalScope.find(func->name()))) {
			assert(sym->type() == Symbol::stFunctionOrCommand);
			funcSym = static_cast<FunctionSymbol*>(sym);
		}
		else {
			funcSym = new FunctionSymbol(func->name());
			mGlobalScope.addSymbol(funcSym);
		}
		funcSym->addFunction(func);
	}
	return true;
}

bool CodeGenerator::generateFunctionDefinitions(const QList<ast::FunctionDefinition*> &functions) {
	bool valid = true;
	for (QList<ast::FunctionDefinition*>::ConstIterator i = functions.begin(); i != functions.end(); i++) {
		CBFunction *func = mSymbolCollector.functionByDefinition(*i);
		func->generateFunction(&mRuntime);
	}
	return valid;
}


bool CodeGenerator::generateGlobalVariables() {
	for (Scope::Iterator i = mGlobalScope.begin(); i != mGlobalScope.end(); i++) {
		if ((*i)->type() == Symbol::stVariable) {
			VariableSymbol *varSym = static_cast<VariableSymbol*>(*i);
			varSym->setAlloca(mBuilder->createGlobalVariable(
						varSym->valueType()->llvmType(),
						false,
						llvm::GlobalValue::PrivateLinkage,
						varSym->valueType()->defaultValue(),
						("CB_Global_" + varSym->name()).toStdString()
						));
		}
	}
	return true;
}

bool CodeGenerator::generateFunctions(const QList<ast::FunctionDefinition*> &functions) {
	bool valid = true;
	for (QList<ast::FunctionDefinition*>::ConstIterator i = functions.begin(); i != functions.end(); i++) {
		CBFunction *func = mSymbolCollector.functionByDefinition(*i);
		valid &= mFuncCodeGen.generate(mBuilder, (*i)->block(), func, &mGlobalScope);
	}
	return valid;
}

bool CodeGenerator::generateMainScope(ast::Block *block) {
	return mFuncCodeGen.generateMainBlock(mBuilder, block, mRuntime.cbMain(), &mMainScope, &mGlobalScope);
}

void CodeGenerator::generateInitializers() {
	mInitializationBlock = llvm::BasicBlock::Create(mBuilder->context(), "Initialize", mRuntime.cbInitialize());
	generateStringLiterals();
	generateTypeInitializers();
	mBuilder->setInsertPoint(mInitializationBlock);
	mBuilder->irBuilder().CreateRetVoid();
}

void CodeGenerator::generateStringLiterals() {
	mBuilder->setInsertPoint(mInitializationBlock);
	mBuilder->stringPool()->generateStringLiterals(mBuilder);
}

void CodeGenerator::generateTypeInitializers() {
	for (Symbol *sym : mGlobalScope) {
		if (sym->type() == Symbol::stType) {
			TypeSymbol *type = static_cast<TypeSymbol*>(sym);
			type->initializeType(mBuilder);
		}
	}
}

void CodeGenerator::createBuilder() {
	mBuilder = new Builder(mRuntime.module()->getContext());
	mBuilder->setRuntime(&mRuntime);
	mBuilder->setStringPool(&mStringPool);
}

void CodeGenerator::addValueTypesToGlobalScope() {
	QList<ValueType*> valueTypes = mRuntime.valueTypeCollection().namedTypes();
	for (ValueType* valueType : valueTypes) {
		mGlobalScope.addSymbol(new DefaultValueTypeSymbol(valueType));
	}
}

void CodeGenerator::addPredefinedConstantSymbols() {
	ConstantSymbol *sym = new ConstantSymbol("pi", mRuntime.floatValueType(), ConstantValue(M_PI), CodePoint());
	mGlobalScope.addSymbol(sym);
	sym = new ConstantSymbol("on", mRuntime.booleanValueType(), ConstantValue(true), CodePoint());
	mGlobalScope.addSymbol(sym);
	sym = new ConstantSymbol("off", mRuntime.booleanValueType(), ConstantValue(false), CodePoint());
	mGlobalScope.addSymbol(sym);
	sym = new ConstantSymbol("true", mRuntime.booleanValueType(), ConstantValue(true), CodePoint());
	mGlobalScope.addSymbol(sym);
	sym = new ConstantSymbol("false", mRuntime.booleanValueType(), ConstantValue(false), CodePoint());
	mGlobalScope.addSymbol(sym);
	sym = new ConstantSymbol("null", mRuntime.typePointerCommonValueType(), ConstantValue(ConstantValue::Null), CodePoint());
	mGlobalScope.addSymbol(sym);
}

bool CodeGenerator::generateTypes(ast::Program *program) {
	for (QList<ast::TypeDefinition*>::ConstIterator i = program->typeDefitions().begin(); i != program->typeDefitions().end(); i++) {
		ast::TypeDefinition *def = *i;
		Symbol *sym = mGlobalScope.find(def->identifier()->name());
		assert(sym && sym->type() == Symbol::stType);
		TypeSymbol *type = static_cast<TypeSymbol*>(sym);

		//Create an opaque member type so type pointers can be used in fields.
		type->createOpaqueTypes(mBuilder);
	}

	for (QList<ast::TypeDefinition*>::ConstIterator i = program->typeDefitions().begin(); i != program->typeDefitions().end(); i++) {
		ast::TypeDefinition *def = *i;
		TypeSymbol *type = static_cast<TypeSymbol*>(mGlobalScope.find(def->identifier()->name()));
		type->createTypePointerValueType(mBuilder);
	}
	return true;
}


