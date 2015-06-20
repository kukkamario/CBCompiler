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
#include "structvaluetype.h"

#include "runtime.h"
#include "typepointervaluetype.h"

SymbolCollector::SymbolCollector(Runtime *runtime) :
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

bool SymbolCollector::declareValueTypes(ast::Program *program, Scope *globalScope, Scope *mainScope) {
	mGlobalScope = globalScope;
	mMainScope = mainScope;

	const QList<ast::TypeDefinition*> &typeDefs = program->typeDefinitions();
	const QList<ast::StructDefinition*> &classDefs = program->structDefinitions();

	mValid = true;
	for (ast::TypeDefinition *def : typeDefs) {
		mValid &= createTypeDefinition(def->identifier());
	}

	for (ast::StructDefinition *def : classDefs) {
		mValid &= createStructDefinition(def->identifier());
	}



	QMap<QString, ValueType*> valueTypes = mRuntime->valueTypeCollection().namedTypesMap();
	for (auto valueTypeIt = valueTypes.begin(); valueTypeIt != valueTypes.end(); ++valueTypeIt) {
		if (!valueTypeIt.value()->isTypePointer())
			mGlobalScope->addSymbol(new DefaultValueTypeSymbol(valueTypeIt.key(), valueTypeIt.value()));
	}
	return mValid;
}

bool SymbolCollector::createStructAndTypeFields(ast::Program *program) {
	const QList<ast::TypeDefinition*> &typeDefs = program->typeDefinitions();
	const QList<ast::StructDefinition*> &classDefs = program->structDefinitions();

	for (ast::StructDefinition *def : classDefs) {
		mValid &= createStructFields(def);
	}

	for (ast::TypeDefinition *def : typeDefs) {
		mValid &= createTypeFields(def);
	}
	return mValid;
}


bool SymbolCollector::collect(ast::Program *program, Scope *globalScope, Scope *mainScope) {
	mGlobalScope = globalScope;
	mCurrentScope = mMainScope = mainScope;
	mFunctions.clear();

	const QList<ast::FunctionDefinition*> &funcDefs = program->functionDefinitions();

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

	ConstantSymbol *symbol = addConstantSymbol(id, valType, (mCurrentScope == mMainScope) ? mGlobalScope : mCurrentScope);

	mConstEval.setScope(mCurrentScope);
	ConstantValue val = mConstEval.evaluate(c->value());
	if (!val.isValid()) {
		return;
	}
	if (valType == 0) {
		ValueType *constValType = mRuntime->valueTypeCollection().constantValueType(val.type());
		symbol->setValueType(constValType);
	}
	symbol->setValue(val);
}

void SymbolCollector::visit(ast::Dim *c) {
	variableDefinitionList(c, mCurrentScope);
}

void SymbolCollector::visit(ast::Variable *c) {
	Symbol *existingSymbol = mCurrentScope->find(c->identifier()->name());
	ValueType *valType = resolveValueType(c->valueType());
	if (!valType) return;
	if (!existingSymbol) {
		if (Settings::forceVariableDeclaration()) {
			emit error(ErrorCodes::ecVariableNotDefined, tr("Variable \"%1\" hasn't been declared").arg(c->identifier()->name()), c->codePoint());
			return;
		}
		ValueType *valType = resolveValueType(c->valueType());
		addVariableSymbol(c->identifier(), valType, mCurrentScope);
		return;
	}

	if (existingSymbol->type() == Symbol::stVariable) {
		VariableSymbol *varSym = static_cast<VariableSymbol*>(existingSymbol);
		if (varSym->valueType() != valType && c->valueType()->type() != ast::Node::ntDefaultType) {
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
	if (existingSymbol->type() == Symbol::stType) {
		if (c->valueType()->type() != ast::Node::ntDefaultType) {
			emit error(ErrorCodes::ecTypeCantHaveValueType, tr("\"%1\" is a type. It can't have a value type"), c->codePoint());
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

void SymbolCollector::visit(ast::Gosub *c) {

}


void SymbolCollector::visit(ast::Goto *c) {

}

void SymbolCollector::visit(ast::Identifier *n) {
	Symbol *symbol = mCurrentScope->find(n->name());
	if (!symbol) {
		QString info = Settings::forceVariableDeclaration() ? tr("You should declare variables with Dim-statement before using") : tr("First usage of a variable should be a assignment.");
		emit error(ErrorCodes::ecCantFindSymbol, tr("Can't find symbol \"%1\". (%2)").arg(n->name(), info), n->codePoint());
	}
}




void SymbolCollector::visit(ast::Expression *c) {
	if (c->associativity() == ast::Expression::RightToLeft) { // Assignment
		ast::Node *before = c->firstOperand();
		for (ast::ExpressionNode *n : c->operations()) {
			if (before) {
				if (before->type() == ast::Node::ntIdentifier) {
					ast::Identifier *id = before->cast<ast::Identifier>();
					Symbol *existingSymbol = mCurrentScope->find(id->name());
					ValueType *valType = mRuntime->intValueType();
					if (!existingSymbol) {
						if (Settings::forceVariableDeclaration()) {
							emit error(ErrorCodes::ecVariableNotDefined, tr("Variable \"%1\" hasn't been declared").arg(id->name()), id->codePoint());
							return;
						}
						addVariableSymbol(id, valType, mCurrentScope);
					}
				}
				else {
					before->accept(this);
				}
			}
			before = n->operand();
		}
		before->accept(this);
	}
	else {
		c->firstOperand()->accept(this);
		for (ast::ExpressionNode *n : c->operations()) {
			if (n->op() != ast::ExpressionNode::opMember) {
				n->accept(this);
			}
		}
	}

}

bool SymbolCollector::createStructDefinition(ast::Identifier *id) {
	if (mGlobalScope->contains(id->name())) {
		symbolAlreadyDefinedError(id->codePoint(), mGlobalScope->find(id->name()));
		return false;
	}

	StructValueType *structValueType = new StructValueType(id->name(), id->codePoint(), mRuntime);

	mRuntime->valueTypeCollection().addStructValueType(structValueType);
	return true;
}



bool SymbolCollector::createTypeDefinition(ast::Identifier *id) {
	if (mGlobalScope->contains(id->name())) {
		symbolAlreadyDefinedError(id->codePoint(), mGlobalScope->find(id->name()));
		return false;
	}
	TypeSymbol *typeSymbol = new TypeSymbol(id->name(), mRuntime, id->codePoint());
	mGlobalScope->addSymbol(typeSymbol);
	mRuntime->valueTypeCollection().addTypePointerValueType(typeSymbol->typePointerValueType());
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
				if (!typeSymbol->addField(TypeField(name, valType, node->codePoint()))) {
					emit error(ErrorCodes::ecTypeHasMultipleFieldsWithSameName, tr("Type field \"%1\" is already defined").arg(varDef->identifier()->name()), varDef->codePoint());
				}
				break;
			}
			default:
				assert("Invalid ast::TypeDefinition" && 0);
		}
	}
	return true;
}

bool SymbolCollector::createStructFields(ast::StructDefinition *def) {

	ValueType *valueType = mRuntime->valueTypeCollection().findNamedType(def->identifier()->name());
	assert(valueType && valueType->isStruct());

	StructValueType *structValueType = static_cast<StructValueType*>(valueType);

	QList<StructField> fields;
	for (ast::Node *node : def->fields()) {
		switch(node->type()) {
			case ast::Node::ntVariable: {
				ast::Variable *varDef = node->cast<ast::Variable>();
				QString name = varDef->identifier()->name();
				ValueType *valType = resolveValueType(varDef->valueType());
				if (!valType) return false;
				fields.append(StructField(name, valType, node->codePoint()));
				break;
			}
			default:
				assert("Invalid ast::TypeDefinition" && 0);
		}
	}

	structValueType->setFields(fields);
	if (structValueType->containsItself()) {
		emit error(ErrorCodes::ecStructContainsItself, tr("Struct \"%1\" contains itself which isn't allowed (infinite recursion)").arg(def->identifier()->name()), def->codePoint());
		return false;
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
	bool success;
	QList<CBFunction::Parameter> params = functionParameterList(parameterList, scope, success);
	if (!success) return false;
	QList<ValueType*> paramValueTypes;
	for (const CBFunction::Parameter p : params) {
		paramValueTypes.append(p.mVariableSymbol->valueType());
	}

	ValueType *retType = 0;
	if (funcDef->returnType())
		retType = resolveValueType(funcDef->returnType());

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

QList<CBFunction::Parameter> SymbolCollector::functionParameterList(ast::Node *node, Scope *scope, bool &success) {
	success = true;
	ast::List *list = node->cast<ast::List>();
	QList<CBFunction::Parameter> ret;
	for (ast::Node *def : list->items()) {
		ast::VariableDefinition *varDef = def->cast<ast::VariableDefinition>();
		VariableSymbol *varSymbol = variableDefinition(varDef, scope);
		if (!varSymbol) {
			success = false;
			return QList<CBFunction::Parameter>();
		}
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
	ast::Node *array = def->array();

	int dimCount = astListSize(def->dimensions());
	ValueType *itemValueType = resolveValueType(def->valueType());
	if (!itemValueType) return 0;
	ast::Identifier *id = 0;
	if (array->type() == ast::Node::ntIdentifier) {
		id = array->cast<ast::Identifier>();
	} else if (array->type() == ast::Node::ntVariable) {
		ast::Variable *var = array->cast<ast::Variable>();
		id = var->identifier();
		if (var->valueType()->type() != ast::Node::ntDefaultType && def->valueType()->type() != ast::Node::ntDefaultType) {
			ValueType *itemValueType2 = resolveValueType(var->valueType());
			if (itemValueType != itemValueType2) {
				emit error(ErrorCodes::ecVariableTypeDefinedTwice, tr("Array element type defined twice. %1 and %2").arg(itemValueType->name(), itemValueType2->name()), def->codePoint());
				return 0;
			}
			else {
				emit warning(WarningCodes::wcVariableTypeDefinedTwice, tr("Array element type defined twice."), def->codePoint());
			}
		}
	}
	else {
		emit error(ErrorCodes::ecExpectingIdentifier, tr("Expecting an identifier in array initialization"), def->codePoint());
		return 0;
	}
	ValueType *arrayValueType = mRuntime->valueTypeCollection().arrayValueType(itemValueType, dimCount);
	return addVariableSymbol(id, arrayValueType, scope);
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
	ConstantSymbol *sym = new ConstantSymbol(identifier->name(), type, identifier->codePoint());
	scope->addSymbol(sym);
	return sym;
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

