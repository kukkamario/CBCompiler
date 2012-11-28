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

CodeGenerator::CodeGenerator() :
	mGlobalScope("Global"),
	mMainScope("Main", &mGlobalScope) {
	mConstEval.setGlobalScope(&mGlobalScope);
	connect(&mConstEval, SIGNAL(error(int,QString,int,QFile*)), this, SIGNAL(error(int,QString,int,QFile*)));
	connect(&mConstEval, SIGNAL(warning(int,QString,int,QFile*)), this, SIGNAL(warning(int,QString,int,QFile*)));
	connect(&mRuntime, SIGNAL(error(int,QString,int,QFile*)), this, SIGNAL(error(int,QString,int,QFile*)));
	connect(&mRuntime, SIGNAL(warning(int,QString,int,QFile*)), this, SIGNAL(warning(int,QString,int,QFile*)));
}

bool CodeGenerator::initialize(const QString &runtimeFile) {
	if (!mRuntime.load(&mStringPool, runtimeFile)) {
		return false;
	}

	mValueTypes.append(mRuntime.valueTypes());

	return addRuntimeFunctions();
}

bool CodeGenerator::generate(ast::Program *program) {
	bool constS = evaluateConstants(program);
	bool typeS = addTypesToScope(program);
	bool globalS = addGlobalsToScope(program);
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
	if (!(constS && globalS && typeS)) return false;
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
		}
		funcSym->addFunction(func);
	}
	return true;
}

bool CodeGenerator::evaluateConstants(ast::Program *program) {
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
				TypeSymbol *type = findTypeSymbol(fieldDef.second->mTypeName, def->mFile, fieldDef.first);
				if (type) {
					if (!type->addField(TypeField(fieldDef.second->mName, type->typePointerValueType(), def->mFile, fieldDef.first))) {
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
