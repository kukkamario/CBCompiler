#include "symbolcollector.h"
#include "errorcodes.h"
#include "warningcodes.h"
#include "typesymbol.h"
#include "functionsymbol.h"
#include "variablesymbol.h"
#include "constantsymbol.h"
#include "arrayvaluetype.h"
#include "intvaluetype.h"
#include "labelsymbol.h"

#include "runtime.h"
#include "typepointervaluetype.h"

SymbolCollector::SymbolCollector(Runtime *runtime, Settings *settings) :
	mSettings(settings),
	mRuntime(runtime),
	mTypeResolver(runtime) {
	connect(&mTypeResolver, &TypeResolver::error, this, &SymbolCollector::error);
	connect(&mTypeResolver, &TypeResolver::warning, this, &SymbolCollector::warning);
	connect(&mConstEval, &ConstantExpressionEvaluator::error, this, &SymbolCollector::error);
	connect(&mConstEval, &ConstantExpressionEvaluator::warning, this, &SymbolCollector::warning);

	connect(this, &SymbolCollector::error, this, &SymbolCollector::errorOccured);
}

SymbolCollector::~SymbolCollector() {

}


bool SymbolCollector::collect(ast::Program *program, Scope *globalScope, Scope *mainScope) {
	mGlobalScope = globalScope;
	mCurrentScope = mMainScope = mainScope;
	mFunctions.clear();

	const QList<ast::FunctionDefinition*> &funcDefs = program->functionDefinitions();
	const QList<ast::TypeDefinition*> &typeDefs = program->typeDefitions();

	mValid = true;
	for (ast::TypeDefinition *def : typeDefs) {
		mValid &= createTypeDefinition(def->identifier());
	}

	for (ast::TypeDefinition *def : typeDefs) {
		mValid &= createTypeFields(def);
	}

	for (ast::FunctionDefinition *def : funcDefs) {
		mValid &= createFunctionDefinition(def);
	}


	program->mainBlock()->accept(this);

	if (!mValid) return false;

	for (QMap<ast::FunctionDefinition*, CBFunction*>::Iterator i = mFunctions.begin(); i != mFunctions.end(); ++i) {
		mCurrentScope = i.value()->scope();
		i.key()->block()->accept(this);
	}

	return mValid;
}

CBFunction *SymbolCollector::functionByDefinition(ast::FunctionDefinition *def) const {
	return mFunctions.value(def);
}

void SymbolCollector::visit(ast::Global *c) {
	variableDefinitionList(c, mGlobalScope);
}

void SymbolCollector::visit(ast::Const *c) {
	ast::Node *valueTypeNode = c->variable()->valueType();
	ast::Identifier *id = c->variable()->identifier();
	ValueType *valType = 0;
	if (valueTypeNode->type() != ast::Node::ntDefaultType) {
		valType = resolveValueType(valueTypeNode);
	}

	addConstantSymbol(id, valType, mCurrentScope);
}

void SymbolCollector::visit(ast::Dim *c) {
	variableDefinitionList(c, mCurrentScope);
}

void SymbolCollector::visit(ast::Variable *c) {
	Symbol *existingSymbol = mCurrentScope->find(c->identifier()->name());
	ValueType *valType = resolveValueType(c->valueType());
	if (!valType) return;
	if (!existingSymbol) {
		if (mSettings->forceVariableDeclaration()) {
			emit error(ErrorCodes::ecVariableNotDefined, tr("Variable \"%1\" hasn't been declared").arg(c->identifier()->name()), c->codePoint());
			return;
		}
		ValueType *valType = resolveValueType(c->valueType());
		addVariableSymbol(c->identifier(), valType, mCurrentScope);
		return;
	}

	if (existingSymbol->type() == Symbol::stVariable) {
		VariableSymbol *varSym = static_cast<VariableSymbol*>(existingSymbol);
		if (varSym->valueType() != valType) {
			emit error(ErrorCodes::ecVariableAlreadyDefinedWithAnotherType, tr("Variable \"%1\" has already been declared as another type in %2").arg(c->identifier()->name(), varSym->codePoint().toString()), c->codePoint());
		}
		return;
	}
	if (existingSymbol->type() == Symbol::stConstant) {
		ConstantSymbol *constSym = static_cast<ConstantSymbol*>(existingSymbol);
		if (c->valueType()->type() == ast::Node::ntDefaultType) {
			return;
		}

		if (constSym->autoValueType()) {
			emit warning(WarningCodes::wcTypeOfConstantIsIgnored, tr("The type of constant \"%1\" is ignored because it hasn't been declared when constant was defined %2").arg(c->identifier()->name(), constSym->codePoint().toString()), c->codePoint());
			return;
		}

		if (valType != constSym->valueType()) {
			emit error(ErrorCodes::ecConstantAlreadyDefinedWithAnotherType, tr("Variable \"%1\" has already been declared as another type in %2").arg(c->identifier()->name(), c->codePoint().toString()), c->codePoint());
		}
		return;
	}

	emit error(ErrorCodes::ecNotVariable, tr("Symbol \"%1\" is not a variable").arg(c->identifier()->name()), c->codePoint());
}

void SymbolCollector::visit(ast::Label *c) {
	Symbol *existingSymbol = mCurrentScope->find(c->name());
	if (existingSymbol) {
		if (existingSymbol->type() == Symbol::stLabel) {
			emit error(ErrorCodes::ecLabelAlreadyDefined, tr("Label \"%1\" already defined in %2").arg(c->name(), existingSymbol->codePoint().toString()), c->codePoint());
		}
		else {
			symbolAlreadyDefinedError(c->codePoint(), existingSymbol);
		}
		return;
	}

	mCurrentScope->addSymbol(new LabelSymbol(c->name(), c->codePoint()));
}

void SymbolCollector::visit(ast::Identifier *c) {
	Symbol *existingSymbol = mCurrentScope->find(c->name());
	ValueType *valType = mRuntime->intValueType();
	if (!valType) return;
	if (!existingSymbol) {
		if (mSettings->forceVariableDeclaration()) {
			emit error(ErrorCodes::ecVariableNotDefined, tr("Variable \"%1\" hasn't been declared").arg(c->name()), c->codePoint());
			return;
		}
		addVariableSymbol(c, valType, mCurrentScope);
		return;
	}
}


void SymbolCollector::visit(ast::ExpressionNode *c) {
	if (c->op() == ast::ExpressionNode::opMember) {
		ast::Node *n = c->operand();
		for (ast::ChildNodeIterator i = n->childNodesBegin(); i != n->childNodesEnd(); i++) {
			(*i)->accept(this);
		}
	} else {
		c->operand()->accept(this);
	}
}



bool SymbolCollector::createTypeDefinition(ast::Identifier *id) {
	if (mGlobalScope->contains(id->name())) {
		symbolAlreadyDefinedError(id->codePoint(), mGlobalScope->find(id->name()));
		return false;
	}
	TypeSymbol *typeSymbol = new TypeSymbol(id->name(), mRuntime, id->codePoint());
	mGlobalScope->addSymbol(typeSymbol);
	mRuntime->valueTypeCollection().addValueType(typeSymbol->typePointerValueType());
	return true;
}

bool SymbolCollector::createTypeFields(ast::TypeDefinition *def) {
	Symbol *sym = mGlobalScope->find(def->identifier()->name());
	assert(sym && sym->type() == Symbol::stType);
	TypeSymbol *typeSymbol = static_cast<TypeSymbol*>(sym);

	for (ast::Node *node : def->fields()) {
		switch(node->type()) {
			case ast::Node::ntVariable: {
				ast::Variable *varDef = node->cast<ast::Variable>();
				QString name = varDef->identifier()->name();
				ValueType *valType = resolveValueType(varDef->valueType());
				if (!valType) return false;
				typeSymbol->addField(TypeField(name, valType, node->codePoint()));
			}
			default:
				assert("Invalid ast::TypeDefinition" && 0);
		}
	}
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

	Scope *scope = new Scope(funcDef->identifier()->name(), mGlobalScope);
	QList<CBFunction::Parameter> params = functionParameterList(parameterList, scope);
	QList<ValueType*> paramValueTypes;
	for (const CBFunction::Parameter p : params) {
		paramValueTypes.append(p.mVariableSymbol->valueType());
	}

	ValueType *retType = resolveValueType(funcDef->returnType());

	if (funcSym->exactMatch(paramValueTypes)) {
		functionAlreadyDefinedError(funcDef->codePoint(), funcSym->exactMatch(paramValueTypes));
		return false;
	}

	CBFunction *cbFunc = new CBFunction(funcDef->identifier()->name(), retType, params, scope, funcDef->codePoint());
	funcSym->addFunction(cbFunc);
	mFunctions[funcDef] = cbFunc;
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


QList<VariableSymbol *> SymbolCollector::variableDefinitionList(ast::Node *node, Scope *scope) {
	QList<VariableSymbol*> result;
	if (node->type() == ast::Node::ntList || node->type() == ast::Node::ntGlobal || node->type() == ast::Node::ntDim) {
		for (ast::ChildNodeIterator i = node->childNodesBegin(); i != node->childNodesEnd(); i++) {
			result.append(variableDefinition(*i, scope));
		}
	}
	else {
		result.append(variableDefinition(node, scope));
	}
	return result;
}

QList<CBFunction::Parameter> SymbolCollector::functionParameterList(ast::Node *node, Scope *scope) {
	ast::List *list = node->cast<ast::List>();
	QList<CBFunction::Parameter> ret;
	for (ast::Node *def : list->items()) {
		ast::VariableDefinition *varDef = def->cast<ast::VariableDefinition>();
		VariableSymbol *varSymbol = variableDefinition(varDef, scope);
		CBFunction::Parameter param;
		param.mVariableSymbol = varSymbol;
		if (varDef->value()->type() != ast::Node::ntDefaultValue) {
			param.mDefaultValue = mConstEval.evaluate(varDef->value());
		}
		ret.append(param);
	}
	return ret;
}

ValueType *SymbolCollector::resolveValueType(ast::Node *valueType) {
	return mTypeResolver.resolve(valueType);
}

VariableSymbol *SymbolCollector::variableDefinition(ast::Node *def, Scope *scope) {
	switch (def->type()) {
		case ast::Node::ntVariableDefinition:
			return variableDefinition(def->cast<ast::VariableDefinition>(), scope);
		case ast::Node::ntArrayInitialization:
			return variableDefinition(def->cast<ast::ArrayInitialization>(), scope);
		default:
			assert("Invalid variable definition" && 0);
			return 0;
	}

}

VariableSymbol *SymbolCollector::variableDefinition(ast::VariableDefinition *def, Scope *scope) {
	ast::Identifier *identifier = def->identifier();
	ast::Node *type = def->valueType();
	ValueType *valueType = resolveValueType(type);
	if (!valueType) return 0;
	return addVariableSymbol(identifier, valueType, scope);
}

VariableSymbol *SymbolCollector::variableDefinition(ast::ArrayInitialization *def, Scope *scope) {
	int dimCount = astListSize(def->dimensions());
	ValueType *itemValueType = resolveValueType(def->valueType());
	ValueType *arrayValueType = mRuntime->valueTypeCollection().arrayValueType(itemValueType, dimCount);
	return addVariableSymbol(def->identifier(), arrayValueType, scope);
}

VariableSymbol *SymbolCollector::addVariableSymbol(ast::Identifier *identifier, ValueType *type, Scope *scope) {
	//FIXME? Should local variables shadow globals? warning?
	//Symbol *existingSymbol = scope->findOnlyThisScope(identifier->name());
	Symbol *existingSymbol = scope->find(identifier->name());
	if (existingSymbol) {
		symbolAlreadyDefinedError(identifier->codePoint(), existingSymbol);
		return 0;
	}
	VariableSymbol *var = new VariableSymbol(identifier->name(), type, identifier->codePoint());
	scope->addSymbol(var);
	return var;
}

ConstantSymbol *SymbolCollector::addConstantSymbol(ast::Identifier *identifier, ValueType *type, Scope *scope) {
	Symbol *existingSymbol = scope->find(identifier->name());
	if (existingSymbol) {
		symbolAlreadyDefinedError(identifier->codePoint(), existingSymbol);
		return 0;
	}
	return new ConstantSymbol(identifier->name(), type, identifier->codePoint());
}


int SymbolCollector::astListSize(ast::Node *node) {
	if (node->type() == ast::Node::ntList) {
		return static_cast<ast::List*>(node)->childNodeCount();
	}
	return 1;
}

void SymbolCollector::errorOccured(int , QString , CodePoint ) {
	mValid = false;
}

