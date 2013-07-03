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
#include "conversionhelper.h"
#include "cbfunction.h"
#include <fstream>

#ifndef M_PI
	#define M_PI 3.14159265358979323846
#endif

CodeGenerator::CodeGenerator(QObject *parent) :
	QObject(parent),
	mGlobalScope("Global"),
	mMainScope("Main", &mGlobalScope),
	mFuncCodeGen(this)
{
	mConstEval.setGlobalScope(&mGlobalScope);
	mTypeChecker.setRuntime(&mRuntime);
	mTypeChecker.setGlobalScope(&mGlobalScope);
	mFuncCodeGen.setStringPool(&mStringPool);

	connect(&mConstEval, SIGNAL(error(int,QString,int,QFile*)), this, SIGNAL(error(int,QString,int,QFile*)));
	connect(&mConstEval, SIGNAL(warning(int,QString,int,QFile*)), this, SIGNAL(warning(int,QString,int,QFile*)));
	connect(&mRuntime, SIGNAL(error(int,QString,int,QFile*)), this, SIGNAL(error(int,QString,int,QFile*)));
	connect(&mRuntime, SIGNAL(warning(int,QString,int,QFile*)), this, SIGNAL(warning(int,QString,int,QFile*)));
	connect(&mTypeChecker, SIGNAL(error(int,QString,int,QFile*)), this, SIGNAL(error(int,QString,int,QFile*)));
	connect(&mTypeChecker, SIGNAL(warning(int,QString,int,QFile*)), this, SIGNAL(warning(int,QString,int,QFile*)));
	connect(&mFuncCodeGen, SIGNAL(error(int,QString,int,QFile*)), this, SIGNAL(error(int,QString,int,QFile*)));
	connect(&mFuncCodeGen, SIGNAL(warning(int,QString,int,QFile*)), this, SIGNAL(warning(int,QString,int,QFile*)));

	addPredefinedConstantSymbols();
}

bool CodeGenerator::initialize(const QString &runtimeFile, const Settings &settings) {
	mSettings = settings;
	mTypeChecker.setForceVariableDeclaration(settings.forceVariableDeclaration());
	if (!mRuntime.load(&mStringPool, runtimeFile)) {
		return false;
	}

	mValueTypes.append(mRuntime.valueTypes());

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
	bool functionDefinitionsValid = addFunctions(program);
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

	//TODO: User functions

	mFuncCodeGen.setFunction(mRuntime.cbMain());
	mFuncCodeGen.setScope(&mMainScope);
	mFuncCodeGen.setRuntime(&mRuntime);

	mFuncCodeGen.setIsMainScope(true);
	qDebug() << "Starting code generation";
	bool valid =  mFuncCodeGen.generateFunctionCode(&program->mMainBlock);
	mFuncCodeGen.generateStringLiterals();
	return valid;
}

bool CodeGenerator::createExecutable(const QString &path) {
	std::string errorInfo;
	if (llvm::verifyModule(*mRuntime.module(), llvm::ReturnStatusAction, &errorInfo)) { //Invalid module
		std::ofstream out;
		out.open("verifier.log");
		out << errorInfo;
		out.close();
		qDebug("Invalid module. See verifier.log");
		return false;
	}
	llvm::raw_fd_ostream bitcodeFile("raw_bitcode.bc", errorInfo, llvm::raw_fd_ostream::F_Binary);
	if (errorInfo.empty()) {
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
		if (sym = mGlobalScope.find(func->name())) {
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

bool CodeGenerator::addFunctions(ast::Program *program) {
	bool valid = true;
	for (QList<ast::FunctionDefinition*>::ConstIterator i = program->mFunctions.begin(); i != program->mFunctions.end(); i++) {
		ast::FunctionDefinition *funcDef = *i;
		QList<CBFunction::Parametre> params;
		bool funcValid = true;
		for (QList<ast::FunctionParametreDefinition>::ConstIterator i = funcDef->mParams.begin(), end = funcDef->mParams.end(); i != end; ++i) {
			const ast::FunctionParametreDefinition &paramDef = *i;
			if (mGlobalScope.find(paramDef.mVariable.mName)) {
				emit error(ErrorCodes::ecParametreSymbolAlreadyDefined,
						   tr("Parametre symbol \"%1\" already defined").arg(paramDef.mVariable.mName),
						   funcDef->mLine, funcDef->mFile);
				funcValid = false;
				continue;
			}
			CBFunction::Parametre param;
			param.mName = paramDef.mVariable.mName;
			if (paramDef.mDefaultValue) {
				mConstEval.setCodeFile(funcDef->mFile);
				mConstEval.setCodeLine(funcDef->mLine);
				param.mDefaultValue = mConstEval.evaluate(paramDef.mDefaultValue);
				if (!param.mDefaultValue.isValid())  {
					funcValid = false;
				}
			}

			if (paramDef.mVariable.mVarType != ast::Variable::TypePtr) {
				param.mType = mRuntime.findValueType(valueTypeFromVarType(paramDef.mVariable.mVarType));
			}
			else {
				TypeSymbol *ts = findTypeSymbol(paramDef.mVariable.mTypeName, funcDef->mFile, funcDef->mLine);
				if (!ts) {
					funcValid = false;
					continue;
				}
				param.mType = ts->typePointerValueType();
			}
			params.append(param);
		}
		if (!funcValid) {
			valid = false;
			continue;
		}

		ValueType *retT = mRuntime.findValueType(valueTypeFromVarType(funcDef->mRetType));

		Symbol *sym;
		FunctionSymbol *funcSym;
		if (sym = mGlobalScope.find(funcDef->mName)) {
			if (sym->type() == Symbol::stFunctionOrCommand) {
				emit error(ErrorCodes::ecSymbolAlreadyDefined,
						   tr("Symbol \"%1\" already defined at line %2 in file %3").arg(funcDef->mName, QString::number(sym->line()), sym->file()->fileName()),
						   funcDef->mLine, funcDef->mFile);
				valid = false;
				continue;
			}
			funcSym = static_cast<FunctionSymbol*>(sym);
		}
		else {
			funcSym = new FunctionSymbol(funcDef->mName);
			mGlobalScope.addSymbol(funcSym);
		}

		CBFunction *cbFunc = new CBFunction(funcDef->mName, retT, params, funcDef->mLine, funcDef->mFile);
		Function *otherFunc;
		if ((otherFunc = funcSym->exactMatch(cbFunc->paramTypes()))) {
			if (otherFunc->isRuntimeFunction()) {
				emit error(ErrorCodes::ecFunctionAlreadyDefined, tr("The function \"%1\" already defined in the runtime library").arg(funcDef->mName), funcDef->mLine, funcDef->mFile);
				valid = false;
				continue;
			}
			else {
				emit error(ErrorCodes::ecFunctionAlreadyDefined,
						   tr("The function \"%1\" already defined at line %2 in file \"%3\"").arg(funcDef->mName, QString::number(otherFunc->line()),otherFunc->file()->fileName()),
						   funcDef->mLine, funcDef->mFile);
				valid = false;
				continue;
			}
		}
		cbFunc->scope()->setParent(&mGlobalScope);
		cbFunc->setBlock(&funcDef->mBlock);
		mCBFunctions.append(cbFunc);
		funcSym->addFunction(cbFunc);
	}
	return valid;
}

bool CodeGenerator::checkMainScope(ast::Program *program) {
	return mTypeChecker.run(&program->mMainBlock, &mMainScope);
}

bool CodeGenerator::checkFunctions() {
	bool valid = true;
	for (QList<CBFunction*>::ConstIterator i = mCBFunctions.begin(), end = mCBFunctions.end(); i != end; ++i) {
		mTypeChecker.setReturnValueType((*i)->returnValue());
		if (!mTypeChecker.run((*i)->block(), (*i)->scope())) valid = false;
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

		if (def->mVarType == ast::Variable::Default) {
			mGlobalScope.addSymbol(new ConstantSymbol(def->mName, val, def->mFile, def->mLine));
		}
		else {
			if (val.type() != valueTypeFromVarType(def->mVarType)) {
				emit warning(ErrorCodes::ecForcingType, tr("The type of the constant differs from its value. Forcing the value to the type of the constant"), def->mLine, def->mFile);
			}
			switch (def->mVarType) {
				case ast::Variable::Integer:
					mGlobalScope.addSymbol(new ConstantSymbol(def->mName, val.toInt(), def->mFile, def->mLine));
					break;
				case ast::Variable::Float:
					mGlobalScope.addSymbol(new ConstantSymbol(def->mName, val.toFloat(), def->mFile, def->mLine));
					break;
				case ast::Variable::Short:
					mGlobalScope.addSymbol(new ConstantSymbol(def->mName, val.toShort(), def->mFile, def->mLine));
					break;
				case ast::Variable::Byte:
					mGlobalScope.addSymbol(new ConstantSymbol(def->mName, val.toByte(), def->mFile, def->mLine));
					break;
				case ast::Variable::String:
					mGlobalScope.addSymbol(new ConstantSymbol(def->mName, val.toString(), def->mFile, def->mLine));
					break;
				default:
					assert(0);
			}
		}
	}
	return valid;
}

bool CodeGenerator::addGlobalsToScope(ast::Program *program) {
	bool valid = true;
	for (QList<ast::GlobalDefinition*>::ConstIterator i = program->mGlobals.begin(); i != program->mGlobals.end(); i++) {
		ast::GlobalDefinition *globalDef = *i;
		for (QList<ast::Variable*>::ConstIterator v = globalDef->mDefinitions.begin(); v != globalDef->mDefinitions.end(); v++) {
			ast::Variable *var = *v;
			Symbol *sym;
			if (sym = mGlobalScope.find(var->mName)) {
				if (sym->file() == 0) {
					emit error(ErrorCodes::ecSymbolAlreadyDefinedInRuntime, tr("Symbol \"%1\" already defined in runtime library").arg(var->mName), globalDef->mLine, globalDef->mFile);
				}
				else {
					emit error(ErrorCodes::ecSymbolAlreadyDefined,
							   tr("Symbol \"%1\" at line %2 in file %3 already defined").arg(
								   var->mName, QString::number(sym->line()), sym->file()->fileName()), globalDef->mLine, globalDef->mFile);
				}
				valid = false;
				continue;
			}
		}
	}
	return valid;
}

bool CodeGenerator::addTypesToScope(ast::Program *program) {
	bool valid = true;
	for (QList<ast::TypeDefinition*>::ConstIterator i = program->mTypes.begin(); i != program->mTypes.end(); i++) {
		ast::TypeDefinition *def = *i;
		Symbol *sym;
		if (sym = mGlobalScope.find(def->mName)) {
			if (sym->file() == 0) {
				emit error(ErrorCodes::ecSymbolAlreadyDefinedInRuntime, tr("Symbol \"%1\" already defined in runtime library").arg(def->mName), def->mLine, def->mFile);
			}
			else {
				emit error(ErrorCodes::ecSymbolAlreadyDefined,
						   tr("Symbol \"%1\" at line %2 in file %3 already defined").arg(
							   def->mName, QString::number(sym->line()), sym->file()->fileName()), def->mLine, def->mFile);
			}
			valid = false;
			continue;
		}
		TypeSymbol *type = new TypeSymbol(def->mName, def->mFile, def->mLine);
		mGlobalScope.addSymbol(type);
		if (!type->createTypePointerValueType(&mRuntime)) {
			emit error(ErrorCodes::ecCantCreateTypePointerLLVMStructType, tr("Can't create llvm StructType for %1. Bug?!!!").arg(def->mName), def->mLine, def->mFile);
			return false;
		}
	}
	if (!valid) return false;

	for (QList<ast::TypeDefinition*>::ConstIterator i = program->mTypes.begin(); i != program->mTypes.end(); i++) {
		ast::TypeDefinition *def = *i;
		TypeSymbol *type = static_cast<TypeSymbol*>(mGlobalScope.find(def->mName));
		assert(type);
		for (QList<QPair<int, ast::Variable*> >::ConstIterator f = def->mFields.begin(); f != def->mFields.end(); f++ ) {
			const QPair<int, ast::Variable*> &fieldDef = *f;
			if (fieldDef.second->mVarType == ast::Variable::TypePtr) {
				TypeSymbol *t = findTypeSymbol(fieldDef.second->mTypeName, def->mFile, fieldDef.first);
				if (t) {
					if (!type->addField(TypeField(fieldDef.second->mName, t->typePointerValueType(), def->mFile, fieldDef.first))) {
						emit error(ErrorCodes::ecTypeHasMultipleFieldsWithSameName, tr("Type \"%1\" has already field with name \"%2\"").arg(def->mName, fieldDef.second->mName), fieldDef.first, def->mFile);
						valid = false;
					}
				}
				else {
					valid = false;
				}
			}
			else {
				if (!type->addField(TypeField(fieldDef.second->mName, mRuntime.findValueType(valueTypeFromVarType(fieldDef.second->mVarType)), def->mFile, fieldDef.first))) {
					emit error(ErrorCodes::ecTypeHasMultipleFieldsWithSameName, tr("Type \"%1\" has already field with name \"%2\"").arg(def->mName, fieldDef.second->mName), fieldDef.first, def->mFile);
					valid = false;
				}
			}
		}
	}


	return valid;
}

void CodeGenerator::addPredefinedConstantSymbols() {
	ConstantSymbol *sym = new ConstantSymbol("pi", ConstantValue(M_PI), 0, 0);
	mGlobalScope.addSymbol(sym);
}



TypeSymbol *CodeGenerator::findTypeSymbol(const QString &typeName, QFile *f, int line) {
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
