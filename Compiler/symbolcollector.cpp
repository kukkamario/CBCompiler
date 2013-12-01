#include "symbolcollector.h"

SymbolCollector::SymbolCollector(Settings *settings) {
}


void SymbolCollector::collect(ast::Program *program, Scope *globalScope, Scope *mainScope) {
	const QList<ast::FunctionDefinition*> &funcDefs = program->functionDefinitions();
	const QList<ast::TypeDefinition*> &typeDefs = program->typeDefitions();
	const QList<ast::Const*> &constants = program->constants();


}

bool SymbolCollector::actBefore(ast::Global *global) {
}

bool SymbolCollector::actAfter(ast::Global *global) {
}

bool SymbolCollector::actBefore(ast::Const *c) {
}

bool SymbolCollector::actBefore(ast::FunctionDefinition *funcDef) {
}

bool SymbolCollector::actBefore(ast::VariableDefinition *def) {
}
