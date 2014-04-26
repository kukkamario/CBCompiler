#include "symbolcollector.h"
#include "errorcodes.h"
#include "typesymbol.h"
#include "functionsymbol.h"
#include "variablesymbol.h"
#include "cbfunction.h"
#include "runtime.h"

SymbolCollector::SymbolCollector(Runtime *runtime, Settings *settings) :
	mSettings(settings),
	mRuntime(runtime)
{
}


bool SymbolCollector::collect(ast::Program *program, Scope *globalScope, Scope *mainScope) {
	const QList<ast::FunctionDefinition*> &funcDefs = program->functionDefinitions();
	const QList<ast::TypeDefinition*> &typeDefs = program->typeDefitions();

	bool valid = true;
	for (ast::TypeDefinition *def : typeDefs) {
		valid &= createTypeDefinition(def->identifier());
	}

	for (ast::FunctionDefinition *def : funcDefs) {
		valid &= createFunctionDefinition(def);
	}



	return valid;
}

bool SymbolCollector::actBefore(ast::Global *global) {
}


bool SymbolCollector::createTypeDefinition(ast::Identifier *id) {
	if (mGlobalScope->contains(id->name())) {
		symbolAlreadyDefinedError(id->codePoint(), mGlobalScope->find(id->name()));
		return false;
	}
	TypeSymbol *typeSymbol = new TypeSymbol(id->name(), mRuntime, id->codePoint());
	mGlobalScope->addSymbol(typeSymbol);
	return true;
}

bool SymbolCollector::createFunctionDefinition(ast::FunctionDefinition *funcDef) {
	Symbol *sym = mGlobalScope->find(funcDef->identifier()->name());
	FunctionSymbol *funcSym = 0;
	if (sym) {
		if (sym->type() != Symbol::stFunctionOrCommand) {
			symbolAlreadyDefinedError(funcDef->codePoint(), sym);
			return false;
		}
		funcSym = static_cast<FunctionSymbol*>(sym);
	}
	else {
		funcSym = new FunctionSymbol(funcDef->identifier()->name());
		mGlobalScope->addSymbol(funcSym);
	}

	ast::Node *parameterList = funcDef->parameterList();
	QList<VariableSymbol*> paramSymbols = handleVariableDefinitionList(parameterList);

	QList<CBFunction::Parameter> params;
	Function::ParamList paramValueTypes;
	for  (VariableSymbol *ps : paramSymbols) {
		if (!ps) {
			qDeleteAll(paramSymbols);
			return false;
		}
		CBFunction::Parameter p;
		p.mVariableSymbol = ps;
		paramValueTypes.append(ps->valueType());
		params.append(p);
	}

	ValueType *retType = resolveValueType(funcDef->returnType());

	if (funcSym->exactMatch(paramValueTypes)) {
		functionAlreadyDefinedError(funcDef->codePoint(), funcSym->exactMatch(paramValueTypes));
		return;
	}

	CBFunction *cbFunc = new CBFunction(funcDef->identifier()->name(), retType, params, new Scope(funcDef->identifier()->name(), mGlobalScope), funcDef->codePoint());
	funcSym->addFunction(cbFunc);
	return true;
}

void SymbolCollector::symbolAlreadyDefinedError(const CodePoint &cp, Symbol *existingSymbol) {
	if (existingSymbol->isRuntimeSymbol()) {
		emit error(ErrorCodes::ecSymbolAlreadyDefined, tr("Symbol \"%1\" already defined in the runtime").arg(existingSymbol->name()), cp);
	} else {
		emit error(ErrorCodes::ecSymbolAlreadyDefined, tr("Symbol \"%1\" already defined in %2").arg(existingSymbol->name(), existingSymbol->codePoint().toString()), cp);
	}

}

void SymbolCollector::functionAlreadyDefinedError(const CodePoint &cp, Function *oldFunctionDef) {
	if (oldFunctionDef->codePoint().isNull()) {
		emit error(ErrorCodes::ecFunctionAlreadyDefined, tr("Function \"%1\" with the same parameter list already defined in the runtime").arg(oldFunctionDef->name()), cp);
	}
	else {
		emit error(ErrorCodes::ecFunctionAlreadyDefined, tr("Function \"%1\" with the same parameter list already defined in %2").arg(oldFunctionDef->name(), oldFunctionDef->codePoint().toString()), cp);
	}
}

VariableSymbol *SymbolCollector::handleVariableDefinitionAndArrayInitialization(ast::Node *node) {
	switch (node->type()) {
		case ast::Node::ntVariableDefinition:
			return handleVariableDefinition(static_cast<ast::VariableDefinition*>(node));
		case ast::Node::ntArrayInitialization:
			return handleArrayInitialization(static_cast<ast::ArrayInitialization*>(node));
		default:
			emit error(ErrorCodes::ecExpectingVariableDefinition, tr("Expecting variable definition"), node->codePoint());
			return 0;
	}

}

VariableSymbol *SymbolCollector::handleVariableDefinition(ast::VariableDefinition *varDef) {
	ValueType *valueType = resolveValueType(varDef->valueType());
	if (!valueType) return 0;
	VariableSymbol *varSym = new VariableSymbol(varDef->identifier()->name(), valueType, varDef->codePoint());
	return varSym;
}

VariableSymbol *SymbolCollector::handleArrayInitialization(ast::ArrayInitialization *arrayInit) {
	ValueType *baseValueType = resolveValueType(arrayInit->valueType());
	ValueType *valTy = mRuntime->valueTypeCollection().arrayValueType(baseValueType, astListSize(arrayInit->dimensions()));
	VariableSymbol *varSym = new VariableSymbol(arrayInit->identifier()->name(), valTy, arrayInit->codePoint());
	return varSym;
}

QList<VariableSymbol *> SymbolCollector::handleVariableDefinitionList(ast::Node *node) {
	QList<VariableSymbol*> result;
	if (node->type() == ast::Node::ntList) {
		ast::List *list = static_cast<ast::List*>(node);
		for (ast::ChildNodeIterator i = list->childNodesBegin(); i != list->childNodesEnd(); i++) {
			result.append(handleVariableDefinitionAndArrayInitialization(*i));
		}
	}
	else {
		result.append(handleVariableDefinitionAndArrayInitialization(node));
	}
	return result;
}

ValueType *SymbolCollector::resolveValueType(ast::Node *valueType) {
	switch (valueType->type()) {
		case ast::Node::ntDefaultType:
			return defaultValueType();
		case ast::Node::ntBasicType:
			return basicValueType(static_cast<ast::BasicType*>(valueType));
		case ast::Node::ntNamedType:
			return namedValueType(static_cast<ast::NamedType*>(valueType));
		case ast::Node::ntArrayType:
			return arrayValueType(static_cast<ast::ArrayType*>(valueType));
	}

}

ValueType *SymbolCollector::basicValueType(ast::BasicType *basicType) {
	switch (basicType->valueType()) {
		case ast::BasicType::Integer:
			return mRuntime->intValueType();
		case ast::BasicType::Float:
			return mRuntime->floatValueType();
		case ast::BasicType::String:
			return mRuntime->stringValueType();
		default:
			assert("Invalid ast::BasicType::ValueType");
			return 0;
	}
}

ValueType *SymbolCollector::namedValueType(ast::NamedType *namedType) {
	ValueType *ty = mRuntime->valueTypeCollection().findNamedType(namedType->identifier()->name());
	if (!ty) {
		emit error(ErrorCodes::ecSymbolNotValueType, tr("Symbol \"%1\" isn't a value type").arg(namedType->identifier()->name()), namedType->codePoint());
	}
	return ty;
}

ValueType *SymbolCollector::arrayValueType(ast::ArrayType *arrayType) {
	ValueType *parentType = resolveValueType(arrayType->parentType());
	if (!parentType) return 0;
	return mRuntime->valueTypeCollection().arrayValueType(parentType, arrayType->dimensions());
}

ValueType *SymbolCollector::defaultValueType() {
	return mRuntime->intValueType();
}

int SymbolCollector::astListSize(ast::Node *node) {
	if (node->type() == ast::Node::ntList) {
		return static_cast<ast::List*>(node)->childNodeCount();
	}
	return 1;
}

bool SymbolCollector::actAfter(ast::Global *global) {
}

bool SymbolCollector::actBefore(ast::Const *c) {
}

bool SymbolCollector::actBefore(ast::FunctionDefinition *funcDef) {
}

bool SymbolCollector::actBefore(ast::VariableDefinition *def) {
}
