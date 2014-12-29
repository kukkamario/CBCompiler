#include "codegenerator.h"
#include "errorcodes.h"
#include "constantsymbol.h"
#include <iostream>
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


#ifndef M_PI
	#define M_PI 3.14159265358979323846
#endif

CodeGenerator::CodeGenerator(ErrorHandler *errorHandler) :
	mRuntime(errorHandler),
	mSymbolCollector(&mRuntime, mSettings),
	mGlobalScope("Global"),
	mMainScope("Main", &mGlobalScope),
	mFuncCodeGen(&mRuntime, mSettings),
	mBuilder(0),
	mErrorHandler(errorHandler)
{
}

bool CodeGenerator::initialize(Settings *settings) {
	mSettings = settings;
	if (!mRuntime.load(&mStringPool, settings)) {
		emit error(ErrorCodes::ecInvalidRuntime, "Runtime loading failed", CodePoint());
		return false;
	}

	createBuilder();
	addPredefinedConstantSymbols();
	return addRuntimeFunctions();
}

bool CodeGenerator::generate(ast::Program *program) {
	std::cout << "Preparing code generation...";
	std::cout << "Collecting symbols...";
	if (!mSymbolCollector.collect(program, &mGlobalScope, &mMainScope)) {
		std::cout << "Failed";
		return false;
	}

	std::cout << "Generating types...";
	if (!generateTypesAndStructes(program)) {
		std::cout << "Failed";
		return false;
	}
	std::cout << "Generating global variables...";
	if (!generateGlobalVariables()) {
		std::cout << "Failed";
		return false;
	}
	std::cout << "Generating function definitions...";
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

	std::cout << "Starting code generation";
	std::cout << "Generating main scope...";
	if (!generateMainScope(program->mainBlock())) {
		std::cout << "Failed";
		return false;
	}
	std::cout << "Generating functions...";
	if (!generateFunctions(program->functionDefinitions())) {
		std::cout << "Failed";
		return false;
	}

	std::cout << "Generating initializers...";
	generateInitializers();
	std::cout << "Finished code generation \n\n";
	return true;
}

bool CodeGenerator::createExecutable(const std::string &path) {
	std::string fileOpenErrorInfo;
	llvm::raw_fd_ostream out("verifier.log", fileOpenErrorInfo, llvm::sys::fs::OpenFlags::F_None);
	if (llvm::verifyModule(*mRuntime.module(), &out)) { //Invalid module
		llvm::AssemblyAnnotationWriter asmAnnoWriter;
		out << "\n\n\n-----LLVM-IR-----\n\n\n";
		mRuntime.module()->print(out, &asmAnnoWriter);
		out.close();
		std::cerr << "Invalid module. See verifier.log";
		return false;
	}
	out.close();

	//std::string p = QDir::currentPath();
	//QDir::setCurrent(QCoreApplication::applicationDirPath());

	llvm::raw_fd_ostream bitcodeFile("raw_bitcode.bc", fileOpenErrorInfo, llvm::sys::fs::OpenFlags::F_None);
	if (fileOpenErrorInfo.empty()) {
		llvm::WriteBitcodeToFile(mRuntime.module(), bitcodeFile);
		bitcodeFile.close();
	}
	else {
		error(ErrorCodes::ecCantWriteBitcodeFile, "Can't write bitcode file \"raw_bitcode.bc\"", CodePoint());
		//QDir::setCurrent(p);
		return false;
	}
	std::cout << "Optimizing bitcode...\n";
	if (!mSettings->callOpt("raw_bitcode.bc", "optimized_bitcode.bc")) {
		error(ErrorCodes::ecOptimizingFailed, "Failed to execute optimizing command", CodePoint());
		return false;
	}
	std::cout << "Creating native assembly...\n";
	if (!mSettings->callLLC("optimized_bitcode.bc", "llc")) {
		error(ErrorCodes::ecCantCreateObjectFile, "Creating a object file failed", CodePoint());
		return false;
	}
	std::cout << "Building binary...\n";

	if (!mSettings->callLinker("llc", "cbrun")) {
		error(ErrorCodes::ecNativeLinkingFailed, "Native linking failed", CodePoint());
		return false;
	}
	std::cout << "Success\n";
	//QDir::setCurrent(p);
	return true;
}

bool CodeGenerator::addRuntimeFunctions() {
	const std::vector<RuntimeFunction*> runtimeFunctions = mRuntime.functions();
	for (std::vector<RuntimeFunction*>::const_iterator i = runtimeFunctions.begin(); i != runtimeFunctions.end(); i ++)  {
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

bool CodeGenerator::generateFunctionDefinitions(const std::vector<ast::FunctionDefinition*> &functions) {
	bool valid = true;
	for (std::vector<ast::FunctionDefinition*>::const_iterator i = functions.begin(); i != functions.end(); i++) {
		CBFunction *func = mSymbolCollector.functionByDefinition(*i);
		func->generateFunction(&mRuntime);
	}
	return valid;
}


bool CodeGenerator::generateGlobalVariables() {
	for (Scope::iterator i = mGlobalScope.begin(); i != mGlobalScope.end(); i++) {
		if ((i->second)->type() == Symbol::stVariable) {
			VariableSymbol *varSym = static_cast<VariableSymbol*>(i->second);
			varSym->setAlloca(mBuilder->createGlobalVariable(
						varSym->valueType()->llvmType(),
						false,
						llvm::GlobalValue::PrivateLinkage,
						varSym->valueType()->defaultValue(),
						("CB_Global_" + varSym->name())
						));
		}
	}
	return true;
}

bool CodeGenerator::generateFunctions(const std::vector<ast::FunctionDefinition*> &functions) {
	bool valid = true;
	for (std::vector<ast::FunctionDefinition*>::const_iterator i = functions.begin(); i != functions.end(); i++) {
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
	for (auto &scopeSym : mGlobalScope) {
		Symbol *sym = scopeSym.second;
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
	sym = new ConstantSymbol("null", mRuntime.nullValueType(), ConstantValue(ConstantValue::Null), CodePoint());
	mGlobalScope.addSymbol(sym);
}

bool CodeGenerator::generateTypesAndStructes(ast::Program *program) {
	for (ast::TypeDefinition* def : program->typeDefinitions()) {
		Symbol *sym = mGlobalScope.find(def->identifier()->name());
		assert(sym && sym->type() == Symbol::stType);
		TypeSymbol *type = static_cast<TypeSymbol*>(sym);

		//Create an opaque member type so type pointers can be used in fields.
		type->createOpaqueTypes(mBuilder);
	}

	std::vector<StructValueType*> notGeneratedValueTypes = mRuntime.valueTypeCollection().structValueTypes();
	while (!notGeneratedValueTypes.empty()) {
		for (std::vector<StructValueType*>::iterator i = notGeneratedValueTypes.begin(); i != notGeneratedValueTypes.end();) {
			StructValueType *structValueType = *i;
			if (structValueType->generateLLVMType()) {
				i = notGeneratedValueTypes.erase(i);
			} else {
				++i;
			}
		}
	}


	for (std::vector<ast::TypeDefinition*>::const_iterator i = program->typeDefinitions().begin(); i != program->typeDefinitions().end(); i++) {
		ast::TypeDefinition *def = *i;
		TypeSymbol *type = static_cast<TypeSymbol*>(mGlobalScope.find(def->identifier()->name()));
		type->createTypePointerValueType(mBuilder);
	}



	return true;
}


