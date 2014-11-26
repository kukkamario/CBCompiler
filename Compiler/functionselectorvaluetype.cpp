#include "functionselectorvaluetype.h"
#include "liststringjoin.h"
#include "value.h"
#include "function.h"
#include "builder.h"
#include "functionvaluetype.h"

FunctionSelectorValueType::FunctionSelectorValueType(Runtime *runtime, const QList<Function *> &functions) :
	ValueType(runtime),
	mFunctions(functions)
{

}


QString FunctionSelectorValueType::name() const {
	if (mFunctions.isEmpty()) return QStringLiteral("Empty function selector");

	return QStringLiteral("Function ") % mFunctions.first()->name() % QStringLiteral(" overload(s): { ") % listStringJoin(mFunctions, [](Function *func) {
		return func->functionValueType()->name();
	}) % QStringLiteral(" }");
}


llvm::Constant *FunctionSelectorValueType::defaultValue() const {
	assert("FunctionSelectorValueType doesn't have default value" && 0);
	return 0;
}


int FunctionSelectorValueType::size() const {
	assert("FunctionSelectorValueType isn't concrete type");
	return 0;
}

ValueType::CastCost FunctionSelectorValueType::castingCostToOtherValueType(const ValueType *to) const {
	if (mFunctions.size() == 1) {
		if (mFunctions.first()->functionValueType() == to) {
			return ValueType::ccNoCost;
		}
	}
	if (to != this) return ValueType::ccNoCast;
	return ValueType::ccNoCost;
}

Value FunctionSelectorValueType::cast(Builder *, const Value &v) const {
	if (v.valueType() != this) return Value();
	return v;
}

QList<Function *> FunctionSelectorValueType::overloads() const {
	return mFunctions;
}


