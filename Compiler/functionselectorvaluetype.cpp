#include "functionselectorvaluetype.h"
#include "liststringjoin.h"
#include "value.h"
#include "function.h"
#include "builder.h"
#include "functionvaluetype.h"

FunctionSelectorValueType::FunctionSelectorValueType(Runtime *runtime, const std::vector<Function *> &functions) :
	ValueType(runtime),
	mFunctions(functions)
{

}


std::string FunctionSelectorValueType::name() const {
	if (mFunctions.empty()) return std::stringLiteral("Empty function selector");

	return std::stringLiteral("Function ") % mFunctions.first()->name() % std::stringLiteral(" overload(s): { ") % listStringJoin(mFunctions, [](Function *func) {
		return func->functionValueType()->name();
	}) % std::stringLiteral(" }");
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

std::vector<Function *> FunctionSelectorValueType::overloads() const {
	return mFunctions;
}


