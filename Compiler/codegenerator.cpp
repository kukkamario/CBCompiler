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
#include "nullvaluetype.h"
#include "typesymbol.h"
#include "typepointervaluetype.h"
#include "functionsymbol.h"
#include "cbfunction.h"
#include <fstream>
#include "cbfunction.h"
#include "variablesymbol.h"
#include "valuetypesymbol.h"
#include "customvaluetype.h"
#include "structvaluetype.h"
#include <llvm/Transforms/IPO/PassManagerBuilder.h>
#include <iostream>
#include "llc.h"

#ifndef M_PI
	#define M_PI 3.14159265358979323846
#endif

CodeGenerator::CodeGenerator(QObject *parent) :
	QObject(parent),
	mSymbolCollector(&mRuntime),
	mGlobalScope("Global"),
	mMainScope("Main", &mGlobalScope),
	mFuncCodeGen(&mRuntime),
	mBuilder(0)
{
	connect(&mRuntime, &Runtime::error, this, &CodeGenerator::error);
	connect(&mRuntime, &Runtime::warning, this, &CodeGenerator::warning);
	connect(&mSymbolCollector, &SymbolCollector::error, this, &CodeGenerator::error);
	connect(&mSymbolCollector, &SymbolCollector::warning, this, &CodeGenerator::warning);
	connect(&mFuncCodeGen, &FunctionCodeGenerator::error, this, &CodeGenerator::error);
	connect(&mFuncCodeGen, &FunctionCodeGenerator::warning, this, &CodeGenerator::warning);
}

bool CodeGenerator::initialize() {
	if (!mRuntime.load(&mStringPool)) {
		emit error(ErrorCodes::ecInvalidRuntime, tr("Runtime loading failed"), CodePoint());
		return false;
	}

	createBuilder();
	addPredefinedConstantSymbols();
	return addRuntimeFunctions();
}

bool CodeGenerator::generate(ast::Program *program) {
	if (!Settings::quiet()) std::cout << "Preparing code generation..." << std::endl;
	if (Settings::verbose()) std::cout << "Declaring types..." << std::endl;
	if (!mSymbolCollector.declareValueTypes(program, &mGlobalScope, &mMainScope)) {
		if (Settings::verbose()) std::cerr << "Failed" << std::endl;
		return false;
	}

	if (Settings::verbose()) std::cout << "Generating types..." << std::endl;
	if (!generateTypesAndStructs(program)) {
		if (Settings::verbose()) std::cerr << "Failed" << std::endl;
		return false;
	}
	if (Settings::verbose()) std::cout << "Collecting symbols..." << std::endl;
	if (!mSymbolCollector.collect(program, &mGlobalScope, &mMainScope)) {
		if (Settings::verbose()) std::cerr << "Failed" << std::endl;
		return false;
	}



	if (Settings::verbose()) std::cout << "Generating global variables..." << std::endl;
	if (!generateGlobalVariables()) {
		if (Settings::verbose()) std::cerr << "Failed" << std::endl;
		return false;
	}
	if (Settings::verbose()) std::cout << "Generating function definitions..." << std::endl;
	generateFunctionDefinitions(program->functionDefinitions());


	if (!Settings::saveScopesFile().isEmpty()) {
		QFile file(Settings::saveScopesFile());
		if (file.open(QFile::WriteOnly)) {
			QTextStream s(&file);
			mGlobalScope.writeToStream(s);
			file.close();
		}
		else {
			qCritical() << "Can't open " << Settings::saveScopesFile();
		}
	}


	if (!Settings::quiet()) std::cout << "Generating code..." << std::endl;
	if (Settings::verbose()) std::cout << "Generating main scope..." << std::endl;
	if (!generateMainScope(program->mainBlock())) {
		if (Settings::verbose()) std::cerr << "Failed" << std::endl;
		return false;
	}
	if (Settings::verbose()) std::cout << "Generating functions..." << std::endl;
	if (!generateFunctions(program->functionDefinitions())) {
		if (Settings::verbose()) std::cerr << "Failed" << std::endl;
		return false;
	}

	if (Settings::verbose()) std::cout << "Generating initializers..." << std::endl;
	generateInitializers();
	if (Settings::verbose()) std::cout << "Finished code generation \n" << std::endl;
	return true;
}

bool CodeGenerator::createExecutable(LLC &llc, const QString &outputFile) {
	if (Settings::verbose()) std::cout << "Running bitcode verifier" << std::endl;
	std::string fileOpenErrorInfo;
	llvm::raw_fd_ostream out("verifier.log", fileOpenErrorInfo, llvm::sys::fs::OpenFlags::F_None);
	if (llvm::verifyModule(*mRuntime.module(), &out)) { //Invalid module
		llvm::AssemblyAnnotationWriter asmAnnoWriter;
		out << "\n\n\n-----LLVM-IR-----\n\n\n";
		mRuntime.module()->print(out, &asmAnnoWriter);
		out.close();
		std::cerr << "Invalid module. See verifier.log" << std::endl;
		return false;
	}
	out.close();

	if (!Settings::saveUnoptimizedBitcodeFile().isEmpty()) {
		if (Settings::verbose()) {
			std::cout << "Writing unoptimized bitcode to file \""
					  << Settings::saveUnoptimizedBitcodeFile().toStdString() << "\"" << std::endl;
		}
		llvm::raw_fd_ostream out(Settings::saveUnoptimizedBitcodeFile().toStdString().c_str(), fileOpenErrorInfo, llvm::sys::fs::OpenFlags::F_None);
		llvm::AssemblyAnnotationWriter asmAnnoWriter;
		mRuntime.module()->print(out, &asmAnnoWriter);
		out.close();
	}

	optimizeModule();

	if (Settings::emitLLVMIR()) {
		llvm::raw_fd_ostream bitcodeFile(outputFile.toStdString().c_str(), fileOpenErrorInfo, llvm::sys::fs::OpenFlags::F_None);
		if (fileOpenErrorInfo.empty()) {
			llvm::WriteBitcodeToFile(mRuntime.module(), bitcodeFile);
			bitcodeFile.close();
			return true;
		}
		else {
			emit error(ErrorCodes::ecCantWriteBitcodeFile, tr("Can't write bitcode file \"%1\"").arg(outputFile), CodePoint());
			return false;
		}
	}


	QString p = QDir::currentPath();
	QDir::setCurrent(QCoreApplication::applicationDirPath());
	std::string outputFileName;

	if (!Settings::quiet()) std::cout << "Generating machine code..." << std::endl;
	if (!llc.compileModule(mRuntime.module(), outputFileName)) {
		return false;
	}

	if (!Settings::compileOnly()) {
		if (!Settings::quiet()) std::cout << "Linking..." << std::endl;
		if (!Settings::callLinker(QString::fromStdString(outputFileName), outputFile)) {
			emit error(ErrorCodes::ecNativeLinkingFailed, tr("Native linking failed"), CodePoint());
			QDir::setCurrent(p);
			return false;
		}
		qDebug() << "Success\n";

	}
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

void CodeGenerator::optimizeModule() {

	llvm::PassManagerBuilder passManagerBuilder;
	passManagerBuilder.SizeLevel = 0;
	switch (Settings::optLevel()) {
		case Settings::O0:
			passManagerBuilder.OptLevel = 0;
			return;
		case Settings::O1:
			passManagerBuilder.OptLevel = 1; break;
		case Settings::O2:
			passManagerBuilder.OptLevel = 2; break;
		case Settings::O3:
			passManagerBuilder.OptLevel = 3; break;
	}

	if (!Settings::quiet()) std::cout << "Optimizing..." << std::endl;

	llvm::Module *module = mRuntime.module();

	passManagerBuilder.DisableTailCalls = false;
	passManagerBuilder.LoadCombine = true;

	llvm::PassManager modulePassManager;
	passManagerBuilder.populateModulePassManager(modulePassManager);

	llvm::FunctionPassManager functionPassManager(module);
	passManagerBuilder.populateFunctionPassManager(functionPassManager);

	if (Settings::verbose()) std::cout << "Optimizing the functions..." << std::endl;


	for (auto funcIt = module->begin(); funcIt != module->end(); ++funcIt) {
		functionPassManager.run(*funcIt);
	}

	if (Settings::verbose()) std::cout << "Optimizing the module..." << std::endl;
	modulePassManager.run(*module);
}


void CodeGenerator::addPredefinedConstantSymbols() {
	ConstantSymbol *sym = new ConstantSymbol("pi", mRuntime.floatValueType(), ConstantValue(M_PI), CodePoint());
	mGlobalScope.addSymbol(sym);
	sym = new ConstantSymbol("maxfloat", mRuntime.floatValueType(), ConstantValue(std::numeric_limits<float>::max()), CodePoint());
	mGlobalScope.addSymbol(sym);
	sym = new ConstantSymbol("on", mRuntime.booleanValueType(), ConstantValue(true), CodePoint());
	mGlobalScope.addSymbol(sym);
	sym = new ConstantSymbol("off", mRuntime.booleanValueType(), ConstantValue(false), CodePoint());
	mGlobalScope.addSymbol(sym);
	sym = new ConstantSymbol("true", mRuntime.booleanValueType(), ConstantValue(true), CodePoint());
	mGlobalScope.addSymbol(sym);
	sym = new ConstantSymbol("false", mRuntime.booleanValueType(), ConstantValue(false), CodePoint());
	mGlobalScope.addSymbol(sym);
	sym = new ConstantSymbol("null", mRuntime.nullValueType(), ConstantValue(ConstantValue::Null), CodePoint());
	mGlobalScope.addSymbol(sym);
}

bool CodeGenerator::generateTypesAndStructs(ast::Program *program) {
	for (ast::TypeDefinition* def : program->typeDefinitions()) {
		Symbol *sym = mGlobalScope.find(def->identifier()->name());
		assert(sym && sym->type() == Symbol::stType);
		TypeSymbol *type = static_cast<TypeSymbol*>(sym);

		//Create an opaque member type so type pointers can be used in fields.
		type->createOpaqueTypes(mBuilder);
	}
	QList<StructValueType*> structValueTypes = mRuntime.valueTypeCollection().structValueTypes();
	for (StructValueType *structVT : structValueTypes) {
		structVT->createOpaqueType();
	}

	if (!mSymbolCollector.createStructAndTypeFields(program)) return false;

	for (StructValueType *structVT : structValueTypes) {
		structVT->generateLLVMType();
	}




	for (QList<ast::TypeDefinition*>::ConstIterator i = program->typeDefinitions().begin(); i != program->typeDefinitions().end(); i++) {
		ast::TypeDefinition *def = *i;
		TypeSymbol *type = static_cast<TypeSymbol*>(mGlobalScope.find(def->identifier()->name()));
		type->createTypePointerValueType(mBuilder);
	}



	return true;
}


