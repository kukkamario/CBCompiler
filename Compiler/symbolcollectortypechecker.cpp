#include "symbolcollectortypechecker.h"
#include "scope.h"
#include "runtime.h"
#include "errorcodes.h"
#include "variablesymbol.h"
#include "constantsymbol.h"
#include "typesymbol.h"
#include "functionsymbol.h"
#include "conversionhelper.h"
#include "typepointervaluetype.h"
#include "intvaluetype.h"
#include "floatvaluetype.h"
#include "typevaluetype.h"
#include "stringvaluetype.h"
#include "shortvaluetype.h"
#include "bytevaluetype.h"
#include "booleanvaluetype.h"
#include "arraysymbol.h"
#include "labelsymbol.h"
#include "cbfunction.h"
#include <QString>
#include <QDebug>
#include "constantexpressionevaluator.h"

SymbolCollectorTypeChecker::SymbolCollectorTypeChecker():
	mForceVariableDeclaration(false),
	mGlobalScope(0),
	mReturnValueType(0),
	mExitLevel(0),
	mExpressionLevel(0),
	mParentBlock(0)
{
}

ValueType *SymbolCollectorTypeChecker::typeCheck(ast::Node *s) {
	ValueType *ret = 0;
	switch(s->type()) {
		case ast::Node::ntInteger:
			ret = mRuntime->intValueType(); break;
		case ast::Node::ntString:
			ret = mRuntime->stringValueType(); break;
		case ast::Node::ntFloat:
			ret = mRuntime->floatValueType(); break;
		case ast::Node::ntExpression:
			ret = typeCheck((ast::Expression*)s); break;
		case ast::Node::ntFunctionCallOrArraySubscript:
			ret = typeCheck((ast::FunctionCallOrArraySubscript*)s); break;
		case ast::Node::ntVariable:
			ret = typeCheck((ast::Variable*)s); break;
		case ast::Node::ntUnary:
			ret = typeCheck((ast::Unary*)s); break;
		case ast::Node::ntTypePtrField:
			ret = typeCheck((ast::TypePtrField*)s); break;
		default:
			assert(0);
	}
	return ret;
}

ValueType *SymbolCollectorTypeChecker::typeCheck(ast::Expression *s) {
	mExpressionLevel++;
	ValueType *first = typeCheck(s->mFirst);
	if (!first) {
		mExpressionLevel--;
		return 0;
	}
	for (QList<ast::Operation>::ConstIterator i = s->mRest.begin(); i != s->mRest.end(); i++) {
		ValueType *second = typeCheck(i->mOperand);
		if (!second) {
			mExpressionLevel--;
			return 0;
		}
		ValueType *result = 0;
		switch (i->mOperator) {
			case ast::opEqual:
			case ast::opNotEqual:
				switch (first->type()) {
					case ValueType::Float:
					case ValueType::Integer:
					case ValueType::Byte:;
					case ValueType::Short:
					case ValueType::Boolean:
					case ValueType::String:
						switch (second->type()) {
							case ValueType::Float:
							case ValueType::Integer:
							case ValueType::Byte:
							case ValueType::Short:
							case ValueType::Boolean:
							case ValueType::String:
								result = mRuntime->booleanValueType(); break;
						}
						break;
					case ValueType::TypePointerCommon:
						switch (second->type()) {
							case ValueType::TypePointer:
							case ValueType::TypePointerCommon:
								result = mRuntime->booleanValueType();
								break;
						}
					case ValueType::TypePointer:
						switch (second->type()) {
							case ValueType::TypePointer:
								if (first == second) result = mRuntime->booleanValueType();
								break;
							case ValueType::TypePointerCommon:
								result = mRuntime->booleanValueType();
								break;
						}
				}
			case ast::opGreater:
			case ast::opLess:
			case ast::opGreaterEqual:
			case ast::opLessEqual:
				switch (first->type()) {
					case ValueType::Float:
					case ValueType::Integer:
					case ValueType::Byte:;
					case ValueType::Short:
					case ValueType::Boolean:
					case ValueType::String:
						switch (second->type()) {
							case ValueType::Float:
							case ValueType::Integer:
							case ValueType::Byte:
							case ValueType::Short:
							case ValueType::Boolean:
							case ValueType::String:
								result = mRuntime->booleanValueType(); break;
						}
						break;
				}
			case ast::opPlus:
				switch (first->type()) {
					case ValueType::Float:
						switch (second->type()) {
							case ValueType::Float:
							case ValueType::Integer:
							case ValueType::Byte:
							case ValueType::Short:
							case ValueType::Boolean:
								result = mRuntime->floatValueType(); break;
							case ValueType::String:
								result = mRuntime->stringValueType(); break;
						}
						break;
					case ValueType::Integer:
					case ValueType::Byte:;
					case ValueType::Short:
					case ValueType::Boolean:
						switch (second->type()) {
							case ValueType::Float:
								result = mRuntime->floatValueType(); break;
							case ValueType::Integer:
							case ValueType::Byte:
							case ValueType::Short:
							case ValueType::Boolean:
								result = mRuntime->intValueType(); break;
							case ValueType::String:
								result = mRuntime->stringValueType(); break;
						}
						break;
					case ValueType::String:
						switch (second->type()) {
							case ValueType::Float:
							case ValueType::Integer:
							case ValueType::Byte:
							case ValueType::Short:
							case ValueType::String:
								result = mRuntime->stringValueType(); break;
						}
						break;
				}
				break;
			case ast::opMinus:
			case ast::opMultiply:
			case ast::opDivide:
			case ast::opPower:
			case ast::opMod:
				switch (first->type()) {
					case ValueType::Float:
						switch (second->type()) {
							case ValueType::Float:
							case ValueType::Integer:
							case ValueType::Byte:
							case ValueType::Short:
								result = mRuntime->floatValueType(); break;
						}
						break;
					case ValueType::Integer:
					case ValueType::Byte:
					case ValueType::Short:
					case ValueType::Boolean:
						switch (second->type()) {
							case ValueType::Float:
								result = mRuntime->floatValueType(); break;
							case ValueType::Integer:
							case ValueType::Byte:
							case ValueType::Short:
								result = mRuntime->intValueType(); break;
						}
						break;
				}
				break;
			case ast::opShl:
			case ast::opShr:
			case ast::opSar:
				switch (first->type()) {
					case ValueType::Integer:
					case ValueType::Byte:
					case ValueType::Short:
					case ValueType::Boolean:
						switch (second->type()) {
							case ValueType::Integer:
							case ValueType::Byte:
							case ValueType::Short:
							case ValueType::Boolean:
								result = mRuntime->intValueType(); break;
						}
						break;
				}
				break;
			case ast::opAnd:
			case ast::opOr:
			case ast::opXor:
				if (first->canBeCastedToValueType(mRuntime->booleanValueType()) && second->canBeCastedToValueType(mRuntime->booleanValueType())) {
					mExpressionLevel--;
					return mRuntime->booleanValueType();
				}
			default:
				assert("Invalid operator" && 0);
				mExpressionLevel--;
				return 0;
		}
		if (!result) {
			emit error(ErrorCodes::ecMathematicalOperationOperandTypeMismatch,
					   tr("Mathematical operation operand type mismatch: No operation \"%1\" between %2 and %3").arg(ast::operatorToString(i->mOperator), first->name(), second->name()), mLine, mFile);
			mExpressionLevel--;
			return 0;
		}
		first = result;
	}
	mExpressionLevel--;
	return first;
}

ValueType *SymbolCollectorTypeChecker::typeCheck(ast::Unary *s) {
	ValueType *r = typeCheck(s->mOperand);
	if (!r) return 0;
	if (s->mOperator == ast::opNot) {
		return mRuntime->booleanValueType();
	}
	if (r == mRuntime->booleanValueType()) {
		return mRuntime->intValueType();
	}
	return r;
}

ValueType *SymbolCollectorTypeChecker::typeCheck(ast::FunctionCallOrArraySubscript *s) {
	mLine = s->mLine;
	mFile = s->mFile;
	Symbol *sym = mScope->find(s->mName);
	if (!sym) {
		emit error(ErrorCodes::ecCantFindSymbol, tr("Can't find function or array with name \"%1\"").arg(s->mName), mLine, mFile);
		return 0;
	}
	if (sym->type() == Symbol::stArray) {
		ArraySymbol *array = static_cast<ArraySymbol*>(sym);
		if (!validateArrayIndex(array, s->mParams)) return 0;
		return array->valueType();
	}
	else if (sym->type() == Symbol::stFunctionOrCommand) {
		FunctionSymbol *funcSym = static_cast<FunctionSymbol*>(sym);
		bool err = false;
		QList<ValueType*> params;
		for (QList<ast::Node*>::ConstIterator i = s->mParams.begin(); i != s->mParams.end(); i++) {
			ValueType *p = typeCheck(*i);
			if (p) {
				params.append(p);
			}
			else {
				err = true;
			}
		}
		if (err) return 0;
		Symbol::OverloadSearchError searchErr;
		Function *func = funcSym->findBestOverload(params, false, &searchErr);
		if (!func) {
			switch (searchErr) {
				case Symbol::oseCannotFindAny: {
						Function *command = funcSym->findBestOverload(params, true, &searchErr);
						switch (searchErr) {
							case Symbol::oseNoError:
								if (insideExpression()) {
									emit error(ErrorCodes::ecCantUseCommandInsideExpression, tr("Can't call command %1 inside an expression").arg(command->name()), s->mLine, s->mFile);
									return 0;
								}
								return mRuntime->intValueType(); //Not used for anything
							case Symbol::oseCannotFindAny: {
								QString pStr;
								for (QList<ValueType*>::ConstIterator i = params.begin(); i != params.end(); i++) {
									if (i != params.begin()) pStr += ',';
									pStr += (*i)->name();
								}
								emit error(ErrorCodes::ecCantFindFunction, tr("Can't find function or command \"%1\" with given parametres (%2)").arg(s->mName, pStr), mLine, mFile);
								return 0;
							}
							case Symbol::oseFoundMultipleOverloads: {
								emit error(ErrorCodes::ecMultiplePossibleOverloads, tr("Can't choose between command \"%1\" overloads").arg(s->mName), mLine, mFile);
								return 0;
							}
							default:
								assert(0);
								return 0;
						}
				}
				case Symbol::oseFoundMultipleOverloads:
					emit error(ErrorCodes::ecMultiplePossibleOverloads, tr("Can't choose between function \"%1\" overloads").arg(s->mName), mLine, mFile);
					return 0;
				default:
					assert(0); //WTF?
					return 0;
			}
		}
		return func->returnValue();
	}
	else {
		emit error(ErrorCodes::ecNotArrayOrFunction, tr("Symbol \"%1\" is not a function or an array").arg(s->mName), mLine, mFile);
		return 0;

	}
}

ValueType *SymbolCollectorTypeChecker::typeCheck(ast::TypePtrField *s) {
	Symbol *sym = mScope->find(s->mTypePtrVar);
	if (!sym || sym->type() != Symbol::stVariable) {
		emit error(ErrorCodes::ecNotVariable, tr("\"%1\" is not a variable").arg(s->mTypePtrVar), mLine, mFile);
		return 0;
	}

	VariableSymbol *varSym = static_cast<VariableSymbol*>(sym);
	if (varSym->valueType()->type() != ValueType::TypePointer) {
		emit error(ErrorCodes::ecNotTypePointer, tr("\"%1\" is not a type pointer").arg(s->mTypePtrVar), mLine, mFile);
		return 0;
	}
	TypePointerValueType *typePtrVt = static_cast<TypePointerValueType*>(varSym->valueType());
	if (!typePtrVt->typeSymbol()->hasField(s->mFieldName)) {
		emit error(ErrorCodes::ecCantFindTypeField, tr("Type \"%1\" doesn't have field \"%2\"").arg(typePtrVt->name(), s->mFieldName), mLine, mFile);
		return 0;
	}

	const TypeField &field = typePtrVt->typeSymbol()->field(s->mFieldName);
	if (s->mFieldType != ast::Variable::Default && field.valueType()->type() != valueTypeFromVarType(s->mFieldType)) {
		emit error(ErrorCodes::ecVarAlreadyDefinedWithAnotherType, tr("Type field \"%1\" is defined with another type").arg(s->mFieldName), mLine, mFile);
		return 0;
	}

	return field.valueType();
}

ValueType *SymbolCollectorTypeChecker::typeCheck(ast::Variable *s) {
	return checkVariable(s->mName, s->mVarType, s->mTypeName);
}

bool SymbolCollectorTypeChecker::run(ast::Block *block, Scope *scope) {
	mExitLevel = 0;
	mScope = scope;
	bool valid = checkBlock(block);
	qDebug() << "Block valid: " << valid;
	for (QMultiMap<QString, CodeLineInfo>::ConstIterator i = mRequiredLabels.begin(); i != mRequiredLabels.end(); ++i) {
		Symbol *sym = mScope->find(i.key());
		if (sym->type() != Symbol::stLabel) {
			emit error(ErrorCodes::ecExpectingLabel, tr("Expecting a label after Goto"), i.value().mLine, i.value().mFile);
			valid = false;
			continue;
		}
	}
	return valid;
}

CBFunction *SymbolCollectorTypeChecker::checkFunctionDefinitionAndAddToGlobalScope(ast::FunctionDefinition *func, Scope *globalScope) {
	mScope = globalScope;
	mLine = func->mLine;
	mFile = func->mFile;
	Symbol *sym = mScope->find(func->mName);
	FunctionSymbol *funcSym = 0;
	if (sym) {
		if (sym->type() != Symbol::stFunctionOrCommand) {
			emit error(ErrorCodes::ecSymbolAlreadyDefinedWithDifferentType, tr("Symbol \"%1\" already defined").arg(func->mName), mLine, mFile);
			return 0;
		}
		funcSym = static_cast<FunctionSymbol*>(sym);
	}
	else {
		funcSym = new FunctionSymbol(func->mName);
		globalScope->addSymbol(funcSym);
	}

	//Function local scope
	Scope *funcScope = new Scope(func->mName, globalScope);
	mScope = funcScope;
	QList<CBFunction::Parameter> paramList;
	bool valid = true;
	foreach (const ast::FunctionParametreDefinition &param, func->mParams) {
		VariableSymbol *var = declareVariable(&param.mVariable);

		if (!var) valid = false;
		CBFunction::Parameter p;
		p.mVariableSymbol = var;

		if (param.mDefaultValue) {
			mConstEval->setCodeFile(mFile);
			mConstEval->setCodeLine(mLine);
			p.mDefaultValue = mConstEval->evaluate(param.mDefaultValue);
			if (!p.mDefaultValue.isValid()) valid = false;
		}
		paramList.append(p);
	}
	if (!valid) return false;

	//Function return type
	ValueType *retType = mRuntime->findValueType(valueTypeFromVarType(func->mRetType));

	CBFunction *function = new CBFunction(func->mName, retType, paramList, funcScope, mLine, mFile);

	//Exactly same function overload already defined
	Function *otherFunction = 0;
	if (otherFunction = funcSym->exactMatch(function->paramTypes())) {
		if (otherFunction->isRuntimeFunction()) {
			emit error(ErrorCodes::ecFunctionAlreadyDefined, tr("Function \"%1\" already defined in the runtime").arg(func->mName), mLine, mFile);
		}
		else {
			if (otherFunction->line() && otherFunction->file()) {
				emit error(ErrorCodes::ecFunctionAlreadyDefined, tr("Function \"%1\" already defined at line %2 in file \"%3\"").arg(func->mName, QString::number(otherFunction->line()), otherFunction->file()->fileName()), mLine, mFile);
			}
			else {
				emit error(ErrorCodes::ecFunctionAlreadyDefined, tr("Function \"%1\" already defined").arg(func->mName), mLine, mFile);
			}
		}
		return 0;
	}

	//Add function overload to FunctionSymbol
	funcSym->addFunction(function);
	return function;
}

void SymbolCollectorTypeChecker::setConstantExpressionEvaluator(ConstantExpressionEvaluator *constEval) {
	mConstEval = constEval;
}

ValueType *SymbolCollectorTypeChecker::typeCheckExpression(ast::Node *s) {
	mExpressionLevel++;
	ValueType *ret = typeCheck(s);
	mExpressionLevel--;
	return ret;
}

bool SymbolCollectorTypeChecker::checkBlock(ast::Block *block) {
	bool valid = true;
	ast::Block *tmpParentBlock = mParentBlock;
	mParentBlock = block;
	for (ast::Block::ConstIterator i = block->begin(); i != block->end(); i++) {
		if (!checkStatement(*i)) {
			valid = false;
		}
	}
	mParentBlock = tmpParentBlock;
	return valid;
}

bool SymbolCollectorTypeChecker::checkStatement(ast::Node *s) {
	switch(s->type()) {
		case ast::Node::ntArrayDefinition:
			return checkStatement((ast::ArrayDefinition*)s);
		case ast::Node::ntArraySubscriptAssignmentExpression:
			return checkStatement((ast::ArraySubscriptAssignmentExpression*)s);
		case ast::Node::ntIfStatement:
			return checkStatement((ast::IfStatement*)s);
		case ast::Node::ntWhileStatement:
			return checkStatement((ast::WhileStatement*)s);
		case ast::Node::ntAssignmentExpression:
			return checkStatement((ast::AssignmentExpression*)s);
		case ast::Node::ntForToStatement:
			return checkStatement((ast::ForToStatement*)s);
		case ast::Node::ntForEachStatement:
			return checkStatement((ast::ForEachStatement*)s);
		case ast::Node::ntVariableDefinition:
			return checkStatement((ast::VariableDefinition*)s);
		case ast::Node::ntSelectStatement:
			return checkStatement((ast::SelectStatement*)s);
		case ast::Node::ntRepeatForeverStatement:
			return checkStatement((ast::RepeatForeverStatement*)s);
		case ast::Node::ntRepeatUntilStatement:
			return checkStatement((ast::RepeatUntilStatement*)s);
		case ast::Node::ntCommandCall:
			return checkStatement((ast::CommandCall*)s);
		case ast::Node::ntFunctionCallOrArraySubscript:
			return checkStatement((ast::FunctionCallOrArraySubscript*)s);
		case ast::Node::ntReturn:
			return checkStatement((ast::Return*)s);
		case ast::Node::ntLabel:
			return checkStatement((ast::Label*)s);
		case ast::Node::ntGoto:
			return checkStatement((ast::Goto*)s);
		case ast::Node::ntExit:
			return checkStatement((ast::Exit*)s);
		case ast::Node::ntCommandCallOrArraySubscriptAssignmentExpression:
			return checkStatement((ast::CommandCallOrArraySubscriptAssignmentExpression*)s);
		default:
			assert(0);
	}
	return false;
}

bool SymbolCollectorTypeChecker::checkStatement(ast::IfStatement *s) {
	mFile = s->mFile;
	mLine = s->mLine;
	ValueType *cond = typeCheckExpression(s->mCondition);
	bool valid = true;
	if (!cond || !tryCastToBoolean(cond)) valid = false;
	if (!checkBlock(&s->mIfTrue)) valid = false;
	if (!checkBlock(&s->mElse)) valid = false;
	return valid;
}

bool SymbolCollectorTypeChecker::checkStatement(ast::WhileStatement *s) {
	mFile = s->mFile;
	mLine = s->mStartLine;
	ValueType *cond = typeCheckExpression(s->mCondition);
	mExitLevel++;
	bool valid = true;
	if (!cond || !tryCastToBoolean(cond)) valid = false;
	if (!checkBlock(&s->mBlock)) valid = false;
	mExitLevel--;
	return valid;
}

bool SymbolCollectorTypeChecker::checkStatement(ast::AssignmentExpression *s) {
	mFile = s->mFile;
	mLine = s->mLine;
	ValueType *var = typeCheck(s->mVariable);
	mExpressionLevel++;
	ValueType *value = typeCheckExpression(s->mExpression);
	mExpressionLevel--;
	bool valid = var && value;
	if (!value->canBeCastedToValueType(var)) {
		if (var->isTypePointer() && !value->isTypePointer()) {
			emit error(ErrorCodes::ecInvalidAssignment, tr("Invalid assignment. Can't assign %1 value to type pointer").arg(value->name()), mLine, mFile);
			valid = false;
		}
		else if(!var->isTypePointer() && value->isTypePointer()) {
			emit error(ErrorCodes::ecInvalidAssignment, tr("Invalid assignment. Can't assign type pointer to %1 variable").arg(var->name()), mLine, mFile);
			valid = false;
		}
		else if (var->isTypePointer() && value->isTypePointer() && var != value) {
			emit error(ErrorCodes::ecInvalidAssignment, tr("Type pointer of type \"%1\" can not be assigned to a type pointer variable of type \"%2\"").arg(value->name(), var->name()), mLine, mFile);
			valid = false;
		}
	}

	return valid;
}

bool SymbolCollectorTypeChecker::checkStatement(ast::ArraySubscriptAssignmentExpression *s) {
	mLine = s->mLine;
	mFile = s->mFile;
	ArraySymbol *array = findAndValidateArraySymbol(s->mArrayName);
	if (!array) return false;
	if (!validateArrayIndex(array, s->mSubscripts)) return false;
	ValueType *valTy = typeCheckExpression(s->mValue);
	if (!valTy) return false;
	if (!valTy->canBeCastedToValueType(array->valueType())) {
		emit error(ErrorCodes::ecInvalidAssignment, tr("Invalid assignment. No valid conversion from %1 to %2.").arg(valTy->name(), array->valueType()->name()), mLine, mFile);
		return false;
	}
	return true;
}

bool SymbolCollectorTypeChecker::checkStatement(ast::ForToStatement *s) {
	mFile = s->mFile;
	mLine = s->mStartLine;
	ValueType *var = checkVariable(s->mVarName, s->mVarType);
	ValueType *from = typeCheckExpression(s->mFrom);
	ValueType *to = typeCheckExpression(s->mTo);
	bool valid = from != 0;
	if (var) {
		if (!var->isNumber()) {
			emit error(ErrorCodes::ecNotNumber, tr("Variable \"%1\" should be number").arg(s->mVarName), mLine, mFile);
			valid = false;
		}
	}
	else {
		valid = false;
	}
	if (to){
		if (!to->isNumber()) {
			emit error(ErrorCodes::ecNotNumber, tr("\"To\" value should be a number"), mLine, mFile);
			valid = false;
		}
	}
	else {
		valid = false;
	}
	if (s->mStep) {
		ValueType *step = typeCheckExpression(s->mStep);
		if (step) {
			if (!step->isNumber()) {
				emit error(ErrorCodes::ecNotNumber, tr("\"Step\" value should be a number"), mLine, mFile);
				valid = false;
			}
		}
		else {
			valid = false;
		}
	}
	mExitLevel++;

	if (!checkBlock(&s->mBlock)) valid = false;
	mExitLevel--;
	return valid;
}

bool SymbolCollectorTypeChecker::checkStatement(ast::ForEachStatement *s) {
	ValueType *var = checkVariable(s->mVarName, ast::Variable::TypePtr, s->mTypeName);
	mExitLevel++;
	bool success = var != 0 && checkBlock(&s->mBlock);
	mExitLevel--;
	return success;
}

bool SymbolCollectorTypeChecker::checkStatement(ast::ArrayDefinition *s) {
	Symbol *sym = mScope->find(s->mName);
	if (sym) {
		emit error(ErrorCodes::ecVariableAlreadyDefined, tr("Symbol \"%1\" already defined").arg(s->mName), mLine, mFile);
		return false;
	}
	else {
		bool valid = true;
		for (QList<ast::Node*>::ConstIterator i = s->mDimensions.begin(); i != s->mDimensions.end(); i++) {
			if (!typeCheck(*i)) valid = false;
		}
		ArraySymbol *sym = new ArraySymbol(s->mName, mRuntime->findValueType(valueTypeFromVarType(s->mType)), s->mDimensions.size(), mFile, mLine);
		mGlobalScope->addSymbol(sym);
		return valid;
	}
}

bool SymbolCollectorTypeChecker::checkStatement(ast::VariableDefinition *s) {
	bool valid = true;
	mFile = s->mFile;
	mLine = s->mLine;
	for (QList<ast::Variable*>::ConstIterator i = s->mDefinitions.begin(); i != s->mDefinitions.end(); i++) {
		ast::Variable *var = *i;
		if (!declareVariable(var)) valid = false;
	}
	return valid;
}

bool SymbolCollectorTypeChecker::checkStatement(ast::SelectStatement *s) {
	assert(0 && "STUB");
	return false;
}

bool SymbolCollectorTypeChecker::checkStatement(ast::RepeatForeverStatement *s){
	mExitLevel++;
	bool success = checkBlock(&s->mBlock);
	mExitLevel--;
	return success;
}

bool SymbolCollectorTypeChecker::checkStatement(ast::RepeatUntilStatement *s) {
	bool valid = true;
	mLine = s->mEndLine;
	mFile = s->mFile;
	ValueType *condVt = typeCheckExpression(s->mCondition);
	if (condVt) {
		valid = tryCastToBoolean(condVt);
	}
	else {
		valid = false;
	}
	mExitLevel++;
	if (!checkBlock(&s->mBlock)) valid = false;
	mExitLevel--;
	return valid;
}

bool SymbolCollectorTypeChecker::checkStatement(ast::CommandCall *s) {
	mLine = s->mLine;
	mFile = s->mFile;
	bool err = false;
	QList<ValueType*> params;
	for (QList<ast::Node*>::ConstIterator i = s->mParams.begin(); i != s->mParams.end(); i++) {
		ValueType *p = typeCheckExpression(*i);
		if (p) {
			params.append(p);
		}
		else {
			err = true;
		}
	}
	if (err)
		return false;

	Symbol *sym = mScope->find(s->mName);
	if (!sym) {
		emit error(ErrorCodes::ecNotCommand, tr("Can't find symbol \"%1\"").arg(s->mName), mLine, mFile);
		return false;
	}
	if (sym->type() != Symbol::stFunctionOrCommand) {
		emit error(ErrorCodes::ecNotCommand, tr("Symbol \"%1\" is not a command").arg(s->mName), mLine, mFile);
		return false;
	}

	FunctionSymbol *funcSym = static_cast<FunctionSymbol*>(sym);
	Symbol::OverloadSearchError searchErr;
	Function *command = funcSym->findBestOverload(params, true, &searchErr);
	if (!command) {
		switch (searchErr) {
			case Symbol::oseCannotFindAny:
				emit error(ErrorCodes::ecCantFindCommand, tr("Can't find command \"%1\" with given parametres").arg(s->mName), mLine, mFile);
				return false;
			case Symbol::oseFoundMultipleOverloads:
				emit error(ErrorCodes::ecMultiplePossibleOverloads, tr("Can't choose between command \"%1\" overloads").arg(s->mName), mLine, mFile);
				return false;
			default:
				assert(0); //WTF?
				return false;
		}
	}
	return true;
}

bool SymbolCollectorTypeChecker::checkStatement(ast::FunctionCallOrArraySubscript *s) {
	return typeCheck(s) != 0;
}

bool SymbolCollectorTypeChecker::checkStatement(ast::Return *s) {
	mFile = s->mFile;
	mLine = s->mLine;
	if (mReturnValueType) { //Function
		if (!s->mValue) {
			emit error(ErrorCodes::ecExpectingValueAfterReturn, tr("Expecting value after Return"), mLine, mFile);
			return false;
		}
		ValueType *retVal = typeCheckExpression(s->mValue);
		if (!retVal) return false;
		if (retVal->castingCostToOtherValueType(mReturnValueType) >= ValueType::sMaxCastCost) {
			emit error(ErrorCodes::ecInvalidReturn, tr("The type of the returned value doesn't match the return type of the function"), mLine, mFile);
			return false;
		}
		return true;
	}
	else { //Gosub return
		if (s->mValue) {
			emit error(ErrorCodes::ecGosubCannotReturnValue, tr("Gosub cannot return a value"), mLine, mFile);
			return false;
		}
		return true;
	}
}

bool SymbolCollectorTypeChecker::checkStatement(ast::Label *s) {
	mLine = s->mLine;
	mFile = s->mFile;
	Symbol *sym = mScope->find(s->mName);
	if (!sym) {
		LabelSymbol *label = new LabelSymbol(s->mName, mFile, mLine);
		mScope->addSymbol(label);
		return true;
	}
	if (sym->type() == Symbol::stLabel) {
		emit error(ErrorCodes::ecLabelAlreadyDefined, tr("Label already defined at line %1 in file %2").arg(QString::number(sym->line()), sym->file()->fileName()), mLine, mFile);
	}
	else {
		emit error(ErrorCodes::ecSymbolAlreadyDefined, tr("Symbol already defined at line %1 in file %2").arg(QString::number(sym->line()), sym->file()->fileName()), mLine, mFile);
	}

	return false;
}

bool SymbolCollectorTypeChecker::checkStatement(ast::Goto *s) {
	mLine = s->mLine;
	mFile = s->mFile;
	mRequiredLabels.insert(s->mLabel, CodeLineInfo(mLine, mFile));
	return true;
}

bool SymbolCollectorTypeChecker::checkStatement(ast::Gosub *s) {
	mLine = s->mLine;
	mFile = s->mFile;
	mRequiredLabels.insert(s->mLabel, CodeLineInfo(mLine, mFile));
	return true;
}

bool SymbolCollectorTypeChecker::checkStatement(ast::Exit *s) {
	mLine = s->mLine;
	mFile = s->mFile;
	if (mExitLevel == 0) {
		emit error(ErrorCodes::ecInvalidExit, tr("Cannot use Exit outside loops"), mLine, mFile);
		return false;
	}
	return true;
}

bool SymbolCollectorTypeChecker::checkStatement(ast::CommandCallOrArraySubscriptAssignmentExpression *s) {
	mFile = s->mFile;
	mLine = s->mLine;
	Symbol *sym = mScope->find(s->mName);
	if (!sym) {
		emit error(ErrorCodes::ecCantFindSymbol, tr("Cant find symbol '%1'").arg(s->mName), mLine, mFile);
		return false;
	}
	if (sym->type() == Symbol::stArray) {
		ast::ArraySubscriptAssignmentExpression *arrAssign = new ast::ArraySubscriptAssignmentExpression;
		arrAssign->mArrayName = s->mName;
		arrAssign->mFile = s->mFile;
		arrAssign->mLine = s->mLine;
		arrAssign->mSubscripts = QList<ast::Node*>() << s->mIndexOrExpressionInParentheses;
		arrAssign->mValue = s->mEqualTo;

		//replace ast::CommandCallOrArraySubscriptAssignmentExpression with ast::ArraySubscriptAssignmentExpression
		replaceParentBlockNode(s, arrAssign);

		s->mEqualTo = 0;
		s->mIndexOrExpressionInParentheses = 0;
		delete s;

		return checkStatement(arrAssign);
	}
	else if (sym->type() == Symbol::stFunctionOrCommand) { //s is ast::CommandCall with equality operator. command (x + 32) = 23
		ast::Expression *param = new ast::Expression;
		param->mFirst = s->mIndexOrExpressionInParentheses;
		param->mRest = QList<ast::Operation>() << ast::Operation(ast::opEqual, s->mEqualTo);


		ast::CommandCall *commandCall = new ast::CommandCall;
		commandCall->mFile = s->mFile;
		commandCall->mLine = s->mLine;
		commandCall->mName = s->mName;
		commandCall->mParams = QList<ast::Node*>() << param;

		assert(mParentBlock);

		//replace ast::CommandCallOrArraySubscriptAssignmentExpression with ast::CommandCall
		replaceParentBlockNode(s, commandCall);

		s->mEqualTo = 0;
		s->mIndexOrExpressionInParentheses = 0;
		delete s;

		return checkStatement(commandCall);
	}
	emit error(ErrorCodes::ecSymbolNotArrayOrCommand, tr("Symbol \"%1\" is not an array or a command").arg(s->mName), mLine, mFile);
	return 0;
}

ArraySymbol *SymbolCollectorTypeChecker::findAndValidateArraySymbol(const QString &name) {
	Symbol *sym = mScope->find(name);
	if (!sym) {
		emit error(ErrorCodes::ecCantFindSymbol, tr("Cant find array '%1'").arg(name), mLine, mFile);
		return 0;
	}
	if (sym->type() != Symbol::stArray) {
		emit error(ErrorCodes::ecSymbolNotArray, tr("Symbol \"%1\" is not an array.").arg(name), mLine, mFile);
		return 0;
	}
	return static_cast<ArraySymbol*>(sym);

}

bool SymbolCollectorTypeChecker::validateArrayIndex(ArraySymbol *array, const QList<ast::Node *> &index) {
	if (array->dimensions() != index.size()) {
		emit error(ErrorCodes::ecArrayDimensionCountDoesntMatch, tr("The array \"%1\" has %2 dimensions").arg(array->name(), QString::number(array->dimensions())), mLine, mFile);
		return false;
	}
	bool valid = true;
	int indexNum = 1;
	for (QList<ast::Node*>::ConstIterator i = index.begin(); i != index.end(); i++) {
		ValueType *valTy = typeCheckExpression(*i);
		if (!valTy) {
			valid = false;
			continue;
		}
		if (!valTy->isNumber()) {
			emit error(ErrorCodes::ecExpectingNumberValue, tr("Array indices should be number values. %1 value was given as %2. index of the array \"%3\"").arg(
						   valTy->name(), QString::number(indexNum), array->name()), mLine, mFile);
			valid = false;
		}
		if (valTy == mRuntime->floatValueType()) {
			emit warning(ErrorCodes::ecDangerousFloatToIntCast, tr("Casting a float value to integer for an array \"%1\"subscript operation.").arg(array->name()), mLine, mFile);
		}
		indexNum++;
	}
	return valid;
}

ValueType *SymbolCollectorTypeChecker::checkTypePointerType(const QString &typeName) {
	Symbol *sym = mScope->find(typeName);
	if (!sym) {
		emit error(ErrorCodes::ecCantFindType, tr("Can't find type \"%1\"").arg(typeName), mLine, mFile);
		return 0;
	}
	if (sym->type() != Symbol::stType) {
		emit error(ErrorCodes::ecCantFindType, tr("\"%1\" is not a type").arg(typeName), mLine, mFile);
		return 0;
	}
	return static_cast<TypeSymbol*>(sym)->typePointerValueType();
}

bool SymbolCollectorTypeChecker::tryCastToBoolean(ValueType *t) {
	if (t->castingCostToOtherValueType(mRuntime->booleanValueType()) >= ValueType::sMaxCastCost) {
		emit error(ErrorCodes::ecNoCastFromTypePointer, tr("Required a value which can be casted to boolean"), mLine, mFile);
		return false;
	}
	return true;
}

ValueType *SymbolCollectorTypeChecker::checkVariable(const QString &name, ast::Variable::VarType type, const QString &typeName) {
	Symbol *sym = mScope->find(name);
	if (sym) {
		if (sym->type() == Symbol::stConstant) {
			ConstantSymbol *constant = static_cast<ConstantSymbol*>(sym);
			ValueType *valType = mRuntime->findValueType(constant->value().type());
			return valType;
		}
		else if (sym->type() == Symbol::stType) {
			if (type != ast::Variable::Default) {
				emit error(ErrorCodes::ecTypeCantHaveValueType, tr("Symbol \"%1\" is a Type. Type can't have a variable type specifier").arg(name), mLine, mFile);
				return 0;
			}
			return mRuntime->typeValueType();
		}
		else {
			if (sym->type() != Symbol::stVariable) {
				emit error(ErrorCodes::ecNotVariable, tr("\"%1\" is not a variable").arg(name), mLine, mFile);
				return 0;
			}
			VariableSymbol *var = static_cast<VariableSymbol*>(sym);
			if (type != ast::Variable::TypePtr) {
				if (type != ast::Variable::Default && valueTypeFromVarType(type) != var->valueType()->type()) {
					emit error(ErrorCodes::ecVarAlreadyDefinedWithAnotherType, tr("Variable already defined with another type"), mLine, mFile);
					return 0;
				}
			}
			else {
				ValueType *typePtr = checkTypePointerType(typeName);
				if (!typePtr) return 0;
				if (typePtr != var->valueType()) {
					emit error(ErrorCodes::ecVarAlreadyDefinedWithAnotherType, tr("Variable already defined with another type"), mLine, mFile);
					return 0;
				}
			}
			return var->valueType();
		}
	}
	else {
		if (mForceVariableDeclaration) {
			emit error(ErrorCodes::ecVariableNotDefined, tr("Variable \"%1\" is not defined").arg(name), mLine, mFile);
			return 0;
		}
		ValueType *vt;
		if(type == ast::Variable::TypePtr) { //Type pointer
			Symbol *typeSym = mScope->find(typeName);
			if (!typeSym) {
				emit error(ErrorCodes::ecCantFindType, tr("Can't find type with name \"%1\"").arg(typeName), mLine, mFile);
				return 0;
			}
			if (typeSym->type() != Symbol::stType) {
				emit error(ErrorCodes::ecCantFindType, tr("\"%1\" is not a type").arg(typeName), mLine, mFile);
				return 0;
			}
			vt = static_cast<TypeSymbol*>(typeSym)->typePointerValueType();
		}
		else {
			vt = mRuntime->findValueType(valueTypeFromVarType(type));
		}

		VariableSymbol *varSym = new VariableSymbol(name, vt, mFile, mLine);
		mScope->addSymbol(varSym);
		return vt;
	}
}

VariableSymbol *SymbolCollectorTypeChecker::declareVariable(const ast::Variable *var) {
	Symbol *sym = mScope->find(var->mName);
	if (sym) {
		//TODO: better error messages
		if (sym->type() == Symbol::stVariable) {
			emit error(ErrorCodes::ecVariableAlreadyDefined,
					   tr("Variable \"%1\" already defined at line %2 in file \"%3\"").arg(
						   var->mName, QString::number(sym->line()), sym->file()->fileName())
					   , mLine, mFile);
			return 0;
		}
		else {
			emit error(ErrorCodes::ecVariableAlreadyDefined, tr("Symbol \"%1\" already defined").arg(var->mName), mLine, mFile);
			return 0;
		}
	}
	else {
		if (var->mVarType != ast::Variable::TypePtr) {
			VariableSymbol *varSym = new VariableSymbol(var->mName, mRuntime->findValueType(valueTypeFromVarType(var->mVarType)), mFile, mLine);
			mScope->addSymbol(varSym);
			return varSym;
		}
		else {
			ValueType *valType = checkTypePointerType(var->mTypeName);
			if (!valType) {
				return 0;
			}
			VariableSymbol *varSym = new VariableSymbol(var->mName, valType, mFile, mLine);
			mScope->addSymbol(varSym);
			return varSym;
		}
	}
}

void SymbolCollectorTypeChecker::replaceParentBlockNode(ast::Node *search, ast::Node *replace) {
	int index = 0;
	for (ast::Block::Iterator i = mParentBlock->begin(); i != mParentBlock->end(); i++) {
		if (*i == search) {
			mParentBlock->replace(index, replace);
			return;
		}
		index++;
	}
	assert("Cant find node" && 0);
}

