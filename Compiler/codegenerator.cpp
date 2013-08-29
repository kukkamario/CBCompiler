#include "codegenerator.h"
#include "errorcodes.h"
#include "constantsymbol.h"
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include "intvaluetype.h"
#include "floatvaluetype.h"
#include "stringvaluetype.h"
#include "shortvaluetype.h"
#include "bytevaluetype.h"
#include "typesymbol.h"
#include "typepointervaluetype.h"
#include "functionsymbol.h"
#include "cbfunction.h"
#include <fstream>
#include "cbfunction.h"
#include "variablesymbol.h"
#include "valuetypesymbol.h"
#include <llvm/Assembly/AssemblyAnnotationWriter.h>

#ifndef M_PI
	#define M_PI 3.14159265358979323846
#endif

CodeGenerator::CodeGenerator(QObject *parent) :
	QObject(parent),
	mGlobalScope("Global"),
	mMainScope("Main", &mGlobalScope),
	mFuncCodeGen(this),
	mBuilder(0)
{
	mConstEval.setGlobalScope(&mGlobalScope);
	mConstEval.setRuntime(&mRuntime);
	mTypeChecker.setRuntime(&mRuntime);
	mTypeChecker.setGlobalScope(&mGlobalScope);
	mTypeChecker.setConstantExpressionEvaluator(&mConstEval);

	connect(&mConstEval, SIGNAL(error(int,QString,int,QString)), this, SIGNAL(error(int,QString,int,QString)));
	connect(&mConstEval, SIGNAL(warning(int,QString,int,QString)), this, SIGNAL(warning(int,QString,int,QString)));
	connect(&mRuntime, SIGNAL(error(int,QString,int,QString)), this, SIGNAL(error(int,QString,int,QString)));
	connect(&mRuntime, SIGNAL(warning(int,QString,int,QString)), this, SIGNAL(warning(int,QString,int,QString)));
	connect(&mTypeChecker, SIGNAL(error(int,QString,int,QString)), this, SIGNAL(error(int,QString,int,QString)));
	connect(&mTypeChecker, SIGNAL(warning(int,QString,int,QString)), this, SIGNAL(warning(int,QString,int,QString)));
	connect(&mFuncCodeGen, SIGNAL(error(int,QString,int,QString)), this, SIGNAL(error(int,QString,int,QString)));
	connect(&mFuncCodeGen, SIGNAL(warning(int,QString,int,QString)), this, SIGNAL(warning(int,QString,int,QString)));

	addPredefinedConstantSymbols();
}

bool CodeGenerator::initialize(const QString &runtimeFile, const Settings &settings) {
	mSettings = settings;
	mTypeChecker.setForceVariableDeclaration(settings.forceVariableDeclaration());
	if (!mRuntime.load(&mStringPool, runtimeFile)) {
		emit error(ErrorCodes::ecInvalidRuntime, tr("Runtime loading failed"), 0, 0);
		return false;
	}
	mValueTypes.append(mRuntime.valueTypes());
	addValueTypesToGlobalScope();

	createBuilder();
	mFuncCodeGen.setBuilder(mBuilder);
	return addRuntimeFunctions();
}

bool CodeGenerator::generate(ast::Program *program) {
	qDebug() << "Preparing code generation";
	qDebug() << "Calculating constants";
	bool constantsValid = calculateConstants(program);
	qDebug() << "Adding types to scopes";
	bool typesValid = addTypesToScope(program);
	qDebug() << "Adding globals to scopes";
	bool globalsValid = addGlobalsToScope(program);
	qDebug() << "Adding functions to scopes";
	bool functionDefinitionsValid = addFunctions(program->mFunctions);
	if (!(constantsValid && globalsValid && typesValid && functionDefinitionsValid)) return false;

	qDebug() << "Checking main scope";
	bool mainScopeValid = checkMainScope(program);
	qDebug() << "Main scope valid: " << mainScopeValid;
	qDebug() << "Checking local scopes of the functions";
	bool functionLocalScopesValid = checkFunctions();


#ifdef _DEBUG
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
	if (!(mainScopeValid && functionLocalScopesValid)) return false;

	qDebug() << "Starting code generation";
	mFuncCodeGen.setRuntime(&mRuntime);
	generateMainScope(&program->mMainBlock);
	generateFunctions();

	generateInitializers();
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
	llvm::raw_fd_ostream bitcodeFile("raw_bitcode.bc", fileOpenErrorInfo, llvm::raw_fd_ostream::F_Binary);
	if (fileOpenErrorInfo.empty()) {
		llvm::WriteBitcodeToFile(mRuntime.module(), bitcodeFile);
		bitcodeFile.close();
	}
	else {
		emit error(ErrorCodes::ecCantWriteBitcodeFile, tr("Can't write bitcode file \"raw_bitcode.bc\""),0,0);
		return false;
	}
	qDebug() << "Optimizing bitcode...\n";
	if (!mSettings.callOpt("raw_bitcode.bc", "optimized_bitcode.bc")) return false;
	qDebug() << "Creating native assembly...\n";
	if (!mSettings.callLLC("optimized_bitcode.bc", "llc")) return false;
	qDebug() << "Building binary...\n";
	if (!mSettings.callLinker("llc", path)) return false;
	qDebug() << "Success\n";
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

bool CodeGenerator::addFunctions(const QList<ast::FunctionDefinition*> &functions) {
	bool valid = true;
	for (QList<ast::FunctionDefinition*>::ConstIterator i = functions.begin(); i != functions.end(); i++) {

		CBFunction *func = mTypeChecker.checkFunctionDefinitionAndAddToGlobalScope(*i, &mGlobalScope);
		if (!func) {
			valid = false;
			continue;
		}
		func->generateFunction(&mRuntime);
		mCBFunctions[*i] = func;
	}
	return valid;
}

bool CodeGenerator::checkMainScope(ast::Program *program) {
	return mTypeChecker.run(&program->mMainBlock, &mMainScope);
}

bool CodeGenerator::checkFunctions() {
	bool valid = true;
	for (QMap<ast::FunctionDefinition *, CBFunction *>::ConstIterator i = mCBFunctions.begin(), end = mCBFunctions.end(); i != end; ++i) {
		mTypeChecker.setReturnValueType(i.value()->returnValue());
		if (!mTypeChecker.run(&i.key()->mBlock, i.value()->scope())) valid = false;
	}
	return valid;
}

bool CodeGenerator::calculateConstants(ast::Program *program) {
	bool valid = true;
	for (QList<ast::ConstDefinition*>::ConstIterator i = program->mConstants.begin(); i != program->mConstants.end(); i++) {
		ast::ConstDefinition* def = *i;
		if (mGlobalScope.find(def->mName)) {
			emit error(ErrorCodes::ecConstantAlreadyDefined, tr("Symbol \"%1\" already defined"), def->mLine, def->mFile);
			valid = false;
		}
		mConstEval.setCodeFile(def->mFile);
		mConstEval.setCodeLine(def->mLine);
		ConstantValue val = mConstEval.evaluate(def->mValue);
		if (!val.isValid()) {
			valid = false;
			continue;
		}

		if (def->mVarType.isEmpty()) {
			mGlobalScope.addSymbol(new ConstantSymbol(def->mName, val, def->mFile, def->mLine));
		}
		else {
			ValueType *constType = findValueType(def->mVarType, def->mLine, def->mFile);
			if (!constType) {
				valid = false;
				continue;
			}
			if (val.type() != constType->type()) {
				emit warning(ErrorCodes::ecForcingType, tr("The type of the constant differs from its value. Forcing the value to the type of the constant"), def->mLine, def->mFile);
			}
			if (constType == mRuntime.intValueType())
				mGlobalScope.addSymbol(new ConstantSymbol(def->mName, val.toInt(), def->mFile, def->mLine));
			else if (constType  == mRuntime.floatValueType())
					mGlobalScope.addSymbol(new ConstantSymbol(def->mName, val.toFloat(), def->mFile, def->mLine));
			else if (constType == mRuntime.shortValueType())
				mGlobalScope.addSymbol(new ConstantSymbol(def->mName, val.toShort(), def->mFile, def->mLine));
			else if (constType  == mRuntime.byteValueType())
				mGlobalScope.addSymbol(new ConstantSymbol(def->mName, val.toByte(), def->mFile, def->mLine));
			else if (constType  == mRuntime.stringValueType())
				mGlobalScope.addSymbol(new ConstantSymbol(def->mName, val.toString(), def->mFile, def->mLine));
			else {
				emit error(ErrorCodes::ecOnlyBasicValueTypeCanBeConstant, tr("\"%1\" is not a basic value type and cannot be a constant type").arg(def->mVarType), def->mLine, def->mFile);
				valid = false;
				continue;
			}
		}
	}
	return valid;
}

bool CodeGenerator::addGlobalsToScope(ast::Program *program) {
	bool valid = true;
	mTypeChecker.setScope(&mGlobalScope);
	for (QList<ast::GlobalDefinition*>::ConstIterator i = program->mGlobals.begin(); i != program->mGlobals.end(); i++) {
		ast::GlobalDefinition *globalDef = *i;
		mTypeChecker.setFile(globalDef->mFile);
		mTypeChecker.setLine(globalDef->mLine);
		for (QList<ast::Variable*>::ConstIterator v = globalDef->mDefinitions.begin(); v != globalDef->mDefinitions.end(); v++) {
			ast::Variable *var = *v;
			VariableSymbol *varSym = mTypeChecker.declareVariable(var);
			if (!varSym) {
				valid = false;
				continue;
			}
			varSym->setAlloca(mBuilder->createGlobalVariable(
						varSym->valueType()->llvmType(),
						false,
						llvm::GlobalValue::PrivateLinkage,
						varSym->valueType()->defaultValue(),
						("CB_Global_" + varSym->name()).toStdString()
						));

		}
	}
	return valid;
}

bool CodeGenerator::addTypesToScope(ast::Program *program) {
	bool valid = true;
	for (QList<ast::TypeDefinition*>::ConstIterator i = program->mTypes.begin(); i != program->mTypes.end(); i++) {
		ast::TypeDefinition *def = *i;
		Symbol *sym;
		if ((sym = mGlobalScope.find(def->mName))) {
			if (sym->file() == 0) {
				emit error(ErrorCodes::ecSymbolAlreadyDefinedInRuntime, tr("Symbol \"%1\" already defined in runtime library").arg(def->mName), def->mLine, def->mFile);
			}
			else {
				emit error(ErrorCodes::ecSymbolAlreadyDefined,
						   tr("Symbol \"%1\" at line %2 in file %3 already defined").arg(
							   def->mName, QString::number(sym->line()), sym->file()), def->mLine, def->mFile);
			}
			valid = false;
			continue;
		}
		TypeSymbol *type = new TypeSymbol(def->mName, &mRuntime, def->mFile, def->mLine);

		//Create an opaque member type so type pointers can be used in fields.
		type->createOpaqueTypes(mBuilder);
		mGlobalScope.addSymbol(type);
	}
	if (!valid) return false;

	for (QList<ast::TypeDefinition*>::ConstIterator i = program->mTypes.begin(); i != program->mTypes.end(); i++) {
		ast::TypeDefinition *def = *i;
		TypeSymbol *type = static_cast<TypeSymbol*>(mGlobalScope.find(def->mName));
		assert(type);
		for (QList<QPair<int, ast::Variable*> >::ConstIterator f = def->mFields.begin(); f != def->mFields.end(); f++ ) {
			const QPair<int, ast::Variable*> &fieldDef = *f;

			ValueType *valType = 0;
			if (fieldDef.second->mTypeName.isEmpty()) { //Default int field
				valType = mRuntime.intValueType();
			}
			else {
				valType = findValueType(fieldDef.second->mTypeName, fieldDef.first, def->mFile);
				if (!valType) {
					valid = false;
					continue;
				}
			}
			assert(valType);
			if (!type->addField(TypeField(fieldDef.second->mName, valType, def->mFile, fieldDef.first))) {
				emit error(ErrorCodes::ecTypeHasMultipleFieldsWithSameName, tr("Type \"%1\" has already field with name \"%2\"").arg(def->mName, fieldDef.second->mName), fieldDef.first, def->mFile);
				valid = false;
				continue;
			}
		}

		type->createTypePointerValueType(mBuilder);
		mTypes.append(type);
	}


	return valid;
}

void CodeGenerator::generateFunctions() {
	for (QMap<ast::FunctionDefinition *, CBFunction *>::ConstIterator i = mCBFunctions.begin(); i != mCBFunctions.end(); ++i) {
		mFuncCodeGen.setFunction(i.value()->function());
		mFuncCodeGen.generateCBFunction(i.key(), i.value());
	}
}

void CodeGenerator::generateMainScope(ast::Block *block) {
	mFuncCodeGen.setFunction(mRuntime.cbMain());
	mFuncCodeGen.setScope(&mMainScope);
	mFuncCodeGen.generateMainScope(block);
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
	foreach(TypeSymbol *type, mTypes) {
		type->initializeType(mBuilder);
	}
}

void CodeGenerator::createBuilder() {
	mBuilder = new Builder(mRuntime.module()->getContext());
	mBuilder->setRuntime(&mRuntime);
	mBuilder->setStringPool(&mStringPool);
}

void CodeGenerator::addValueTypesToGlobalScope() {
	mGlobalScope.addSymbol(new DefaultValueTypeSymbol(mRuntime.intValueType()));
	mGlobalScope.addSymbol(new DefaultValueTypeSymbol(mRuntime.floatValueType()));
	mGlobalScope.addSymbol(new DefaultValueTypeSymbol(mRuntime.shortValueType()));
	mGlobalScope.addSymbol(new DefaultValueTypeSymbol(mRuntime.stringValueType()));
	mGlobalScope.addSymbol(new DefaultValueTypeSymbol(mRuntime.byteValueType()));
}

void CodeGenerator::addPredefinedConstantSymbols() {
	ConstantSymbol *sym = new ConstantSymbol("pi", ConstantValue(M_PI), 0, 0);
	mGlobalScope.addSymbol(sym);
	sym = new ConstantSymbol("on", ConstantValue(true), 0, 0);
	mGlobalScope.addSymbol(sym);
	sym = new ConstantSymbol("off", ConstantValue(false), 0, 0);
	mGlobalScope.addSymbol(sym);
	sym = new ConstantSymbol("true", ConstantValue(true), 0, 0);
	mGlobalScope.addSymbol(sym);
	sym = new ConstantSymbol("false", ConstantValue(false), 0, 0);
	mGlobalScope.addSymbol(sym);
	sym = new ConstantSymbol("null", ConstantValue(ValueType::TypePointerCommon), 0, 0);
	mGlobalScope.addSymbol(sym);
}

ValueType *CodeGenerator::findValueType(const QString &valueTypeName, int line, QString file) {
	Symbol *sym = mGlobalScope.find(valueTypeName);
	if (!sym->isValueTypeSymbol()) {
		emit error(ErrorCodes::ecExpectingVarType, tr("\"%1\" is not a value type").arg(valueTypeName), line, file);
		return 0;
	}
	return static_cast<ValueTypeSymbol*>(sym)->valueType();
}



TypeSymbol *CodeGenerator::findTypeSymbol(const QString &typeName, const QString &f, int line) {
	Symbol *sym = mGlobalScope.find(typeName);
	if (!sym) {
		emit error(ErrorCodes::ecCantFindType, tr("Can't find type with name \"%1\""), line, f);
		return 0;
	}
	if (sym->type() != Symbol::stType) {
		emit error(ErrorCodes::ecCantFindType, tr("\"%1\" is not a type").arg(typeName), line, f);
		return 0;
	}
	return static_cast<TypeSymbol*>(sym);
}
