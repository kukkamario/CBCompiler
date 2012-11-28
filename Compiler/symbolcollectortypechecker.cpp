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
#include "stringvaluetype.h"
#include "shortvaluetype.h"
#include "bytevaluetype.h"
#include "booleanvaluetype.h"
#include "arraysymbol.h"
SymbolCollectorTypeChecker::SymbolCollectorTypeChecker():
	mForceVariableDeclaration(false)
{
}

ValueType *SymbolCollectorTypeChecker::typeCheck(ast::Node *s) {
	switch(s->type()) {
		case ast::Node::ntInteger:
			return mRuntime->intValueType();
		case ast::Node::ntString:
			return mRuntime->stringValueType();
		case ast::Node::ntFloat:
			return mRuntime->floatValueType();
		case ast::Node::ntExpression:
			return typeCheck((ast::Expression*)s);
		case ast::Node::ntFunctionCallOrArraySubscript:
			return typeCheck((ast::FunctionCallOrArraySubscript*)s);
		case ast::Node::ntNew:
			return typeCheck((ast::New*)s);
		case ast::Node::ntVariable:
			return typeCheck((ast::Variable*)s);
		case ast::Node::ntUnary:
			return typeCheck((ast::Unary*)s);
		case ast::Node::ntTypePtrField:
			return typeCheck((ast::TypePtrField*)s);
		default:
			assert(0);
			return 0;
	}
}

ValueType *SymbolCollectorTypeChecker::typeCheck(ast::Expression *s) {
	ValueType *first = typeCheck(s->mFirst);
	if (!first) return 0;

	for (QList<ast::Operation>::ConstIterator i = s->mRest.begin(); i != s->mRest.end(); i++) {
		ValueType *second = typeCheck(i->mOperand);
		if (!second) return 0;
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
					case ValueType::NULLTypePointer:
						switch (second->type()) {
							case ValueType::TypePointer:
							case ValueType::NULLTypePointer:
								result = mRuntime->booleanValueType();
								break;
						}
					case ValueType::TypePointer:
						switch (second->type()) {
							case ValueType::TypePointer:
								if (first == second) result = mRuntime->booleanValueType();
								break;
							case ValueType::NULLTypePointer:
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
				if (first->castCost(mRuntime->booleanValueType()) < ValueType::maxCastCost && second->castCost(mRuntime->booleanValueType()) < ValueType::maxCastCost ) {
					return mRuntime->booleanValueType();
				}
			default:
				assert(0);
				return 0;
		}
		if (!result) {
			emit error(ErrorCodes::ecMathematicalOperationOperandTypeMismatch,
					   tr("Mathematical operation operand type mismatch: No operation \"%1\" between %2 and %3").arg(ast::operatorToString(i->mOperator), first->name(), second->name()), mLine, mFile);
			return 0;
		}
		first = result;
	}
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

ValueType *SymbolCollectorTypeChecker::typeCheck(ast::New *s) {
	Symbol *sym = mScope->find(s->mTypeName);
	if (!sym) {
		emit error(ErrorCodes::ecCantFindType, tr("Can't find type with name \"%1\"").arg(s->mTypeName), mLine, mFile);
		return 0;
	}
	if (sym->type() != Symbol::stType) {
		emit error(ErrorCodes::ecCantFindType, tr("\"%1\" is not a type").arg(s->mTypeName), mLine, mFile);
		return 0;
	}
	return static_cast<TypeSymbol*>(sym)->typePointerValueType();
}

ValueType *SymbolCollectorTypeChecker::typeCheck(ast::FunctionCallOrArraySubscript *s) {
	mLine = s->mLine;
	mFile = s->mFile;
	Symbol *sym = mScope->find(s->mName);
	if (sym->type() == Symbol::stArray) {
		ArraySymbol *array = static_cast<ArraySymbol*>(sym);
		bool err = false;
		for (QList<ast::Node*>::ConstIterator i = s->mParams.begin(); i != s->mParams.end(); i++) {
			ValueType *p = typeCheck(*i);
			if (p) {
				if (!(p->type() == ValueType::Integer || p->type() == ValueType::Short || p->type() == ValueType::Byte)) {
					emit error(ErrorCodes::ecArraySubscriptNotInteger, tr("Array subscript should be integer value"), mLine, mFile);
					err = true;
				}
			}
			else {
				err = true;
			}
		}
		if (array->dimensions() != s->mParams.size()) {
			emit error(ErrorCodes::ecInvalidArraySubscript, tr("Array dimensions don't match, expecing %1 array subscripts").arg(QString::number(array->dimensions())), mLine, mFile);
			err = true;
		}
		if (err) return 0;
		return array->valueType();
	}
	else if (sym->type() == Symbol::stFunctionOrCommand) {
		FunctionSymbol *funcSym = static_cast<FunctionSymbol*>(sym);
		bool err;
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
				case Symbol::oseCannotFindAny:
					emit error(ErrorCodes::ecCantFindFunction, tr("Can't find function \"%1\" with given parametres").arg(s->mName), mLine, mFile);
					return 0;
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
	Symbol *sym = mScope->find(s->mName);
	if (sym) {
		if (sym->type() != Symbol::stVariable) {
			emit error(ErrorCodes::ecNotVariable, tr("\"%1\" is not a variable").arg(s->mName), mLine, mFile);
			return 0;
		}
		VariableSymbol *var = static_cast<VariableSymbol*>(sym);
		if (s->mVarType != ast::Variable::TypePtr) {
			if (s->mVarType != ast::Variable::Default && valueTypeFromVarType(s->mVarType) != var->valueType()->type()) {
				emit error(ErrorCodes::ecVarAlreadyDefinedWithAnotherType, tr("Variable already defined with another type"), mLine, mFile);
				return 0;
			}
		}
		else {
			Symbol *sym = mScope->find(s->mTypeName);
			if (!sym) {
				emit error(ErrorCodes::ecCantFindType, tr("Can't find type with name \"%1\"").arg(s->mTypeName), mLine, mFile);
				return 0;
			}
			if (sym->type() != Symbol::stType) {
				emit error(ErrorCodes::ecCantFindType, tr("\"%1\" is not a type").arg(s->mTypeName), mLine, mFile);
				return 0;
			}
			if (static_cast<TypeSymbol*>(sym)->typePointerValueType() != var->valueType()) {
				emit error(ErrorCodes::ecVarAlreadyDefinedWithAnotherType, tr("Variable already defined with another type"), mLine, mFile);
				return 0;
			}
		}
		return var->valueType();
	}
	else {
		if (mForceVariableDeclaration) {
			emit error(ErrorCodes::ecVariableNotDefined, tr("Variable \"%1\" is not defined").arg(s->mName), mLine, mFile);
			return 0;
		}
		ValueType *vt;
		if(s->mVarType == ast::Variable::TypePtr) { //Type pointer
			Symbol *typeSym = mScope->find(s->mTypeName);
			if (!typeSym) {
				emit error(ErrorCodes::ecCantFindType, tr("Can't find type with name \"%1\"").arg(s->mTypeName), mLine, mFile);
				return 0;
			}
			if (typeSym->type() != Symbol::stType) {
				emit error(ErrorCodes::ecCantFindType, tr("\"%1\" is not a type").arg(s->mTypeName), mLine, mFile);
				return 0;
			}
			vt = static_cast<TypeSymbol*>(typeSym)->typePointerValueType();
		}
		else {
			vt = mRuntime->findValueType(valueTypeFromVarType(s->mVarType));
		}

		VariableSymbol *varSym = new VariableSymbol(s->mName, vt, mFile, mLine);
		mScope->addSymbol(varSym);
		return vt;
	}
}


bool SymbolCollectorTypeChecker::run(const ast::Block *block, Scope *scope) {
	mScope = scope;
	return true;
}
