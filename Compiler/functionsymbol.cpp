#include "functionsymbol.h"
#include "valuetype.h"
#include "functionselectorvaluetype.h"

FunctionSymbol::FunctionSymbol(const QString &name):
	Symbol(name, CodePoint()),
	mSelector(0)
{
}

void FunctionSymbol::addFunction(Function *func) {
	mFunctions.append(func);
}

Function *FunctionSymbol::exactMatch(const Function::ParamList &params) const{
	for (QList<Function*>::ConstIterator i = mFunctions.begin(); i != mFunctions.end(); i++) {
		Function *func = *i;
		if (func->paramTypes() == params) return func;
	}
	return 0;
}

Function *FunctionSymbol::findBestOverload(const QList<ValueType *> &paramTypes, bool command, FunctionSymbol::OverloadSearchError *err)
{
	if (err) *err = oseNoError;
	if (mFunctions.empty()) {
		if (err) *err = oseCannotFindAny;
		return 0;
	}
	//No overloads
	if (mFunctions.size() == 1) {
		Function *f = mFunctions.first();
		if (f->requiredParams() > paramTypes.size() || (f->returnValue() != 0) == command) {
			if (err) *err = oseCannotFindAny;
			return 0;
		}
		Function::ParamList::ConstIterator p1i = f->paramTypes().begin();
		ValueType::CastCostType totalCost = f->paramTypes().size() - paramTypes.size();
		for (Function::ParamList::ConstIterator p2i = paramTypes.begin(); p2i != paramTypes.end(); p2i++) {
			totalCost += (*p2i)->castingCostToOtherValueType(*p1i);
			p1i++;
		}
		if (totalCost >= ValueType::sMaxCastCost) {
			if (err) *err = oseCannotFindAny;
			return 0;
		}
		return f;
	}
	bool multiples = false;
	Function *bestFunc = 0;
	ValueType::CastCostType bestCost = ValueType::sMaxCastCost;

	for (QList<Function*>::ConstIterator fi = mFunctions.begin(); fi != mFunctions.end(); fi++) {
		Function *f = *fi;
		if (f->paramTypes().size() >= paramTypes.size() && f->requiredParams() <= paramTypes.size() && (f->returnValue() == 0) == command) {
			Function::ParamList::ConstIterator p1i = f->paramTypes().begin();
			ValueType::CastCostType totalCost = f->paramTypes().size() - paramTypes.size();
			for (Function::ParamList::ConstIterator p2i = paramTypes.begin(); p2i != paramTypes.end(); p2i++) {
				totalCost += (*p2i)->castingCostToOtherValueType(*p1i);
				p1i++;
			}
			if (totalCost == bestCost) {
				multiples = true;
				continue;
			}
			if (totalCost < bestCost) {
				bestFunc = f;
				bestCost = totalCost;
				multiples = false;
				continue;
			}
		}
	}
	if (bestFunc == 0) {
		if (err) *err = oseCannotFindAny;
		return 0;
	}
	if (multiples) {
		if (err) *err = oseFoundMultipleOverloads;
		return 0;
	}
	return bestFunc;
}

QString FunctionSymbol::info() const {
	QString str("Function " + mName);
	if (mFunctions.count() > 1) str += ", " + QString::number(mFunctions.count()) + " overloads";
	return str;
}

FunctionSelectorValueType *FunctionSymbol::functionSelector() const {
	if (!mSelector) {
		mSelector = new FunctionSelectorValueType();
		mSelector->
	}

	return mSelector;
}

