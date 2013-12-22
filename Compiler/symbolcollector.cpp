#include "symbolcollector.h"
#include "errorcodes.h"
#include "typesymbol.h"

SymbolCollector::SymbolCollector(Runtime *runtime, Settings *settings) :
	mSettings(settings),
	mRuntime(runtime)
{
}


void SymbolCollector::collect(ast::Program *program, Scope *globalScope, Scope *mainScope) {
	const QList<ast::FunctionDefinition*> &funcDefs = program->functionDefinitions();
	const QList<ast::TypeDefinition*> &typeDefs = program->typeDefitions();

	for (ast::TypeDefinition *def : typeDefs) {
		createTypeDefinition(def->identifier());
	}
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

void SymbolCollector::symbolAlreadyDefinedError(const CodePoint &cp, Symbol *existingSymbol) {
	if (existingSymbol->isRuntimeSymbol()) {
		emit error(ErrorCodes::ecSymbolAlreadyDefined, tr("Symbol \"%1\" already defined in the runtime").arg(existingSymbol->name()), cp);
	} else {
		emit error(ErrorCodes::ecSymbolAlreadyDefined, tr("Symbol \"%1\" already defined in %2").arg(existingSymbol->name(), existingSymbol->codePoint().toString()), cp);
	}

}

bool SymbolCollector::actAfter(ast::Global *global) {
}

bool SymbolCollector::actBefore(ast::Const *c) {
}

bool SymbolCollector::actBefore(ast::FunctionDefinition *funcDef) {
}

bool SymbolCollector::actBefore(ast::VariableDefinition *def) {
}
