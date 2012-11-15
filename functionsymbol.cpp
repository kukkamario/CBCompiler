#include "functionsymbol.h"
#include "valuetype.h"

FunctionSymbol::FunctionSymbol(const QString &name):
	Symbol(name)
{
}

Function *FunctionSymbol::findBestOverload(const QList<ValueType *> &paramTypes, FunctionSymbol::OverloadSearchError *err)
{
	if (err) *err = oseNoError;
	if (mFunctions.empty()) {
		if (err) *err = oseCannotFindAny;
		return 0;
	}
	//No overloads
	if (mFunctions.size() == 1) {
		Function *f = mFunctions.first();
		if (f->paramTypes().size() < paramTypes.size() || f->requiredParams() > paramTypes.size()) {
			if (err) *err = oseCannotFindAny;
			return 0;
		}
		Function::ParamList::ConstIterator p1i = f->paramTypes().begin();
		ValueType::CastCostType totalCost = f->paramTypes().size() - paramTypes.size();
		for (Function::ParamList::ConstIterator p2i = paramTypes.begin(); p2i != paramTypes.end(); p2i++) {
			totalCost += (*p1i)->castCost(*p2i);
			p1i++;
		}
		if (totalCost >= ValueType::maxCastCost) {
			if (err) *err = oseCannotFindAny;
			return 0;
		}
		return f;
	}
	bool multiples = false;
	Function *bestFunc = 0;
	ValueType::CastCostType bestCost = ValueType::maxCastCost;

	for (QList<Function*>::ConstIterator fi = mFunctions.begin(); fi != mFunctions.end(); fi++) {
		Function *f = *fi;
		if (f->paramTypes().size() >= paramTypes.size() && f->requiredParams() <= paramTypes.size()) {
			Function::ParamList::ConstIterator p1i = f->paramTypes().begin();
			ValueType::CastCostType totalCost = f->paramTypes().size() - paramTypes.size();
			for (Function::ParamList::ConstIterator p2i = paramTypes.begin(); p2i != paramTypes.end(); p2i++) {
				totalCost += (*p1i)->castCost(*p2i);
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

