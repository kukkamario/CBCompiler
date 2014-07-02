#include "functionvaluetype.h"
#include "liststringjoin.h"
#include "value.h"
#include "function.h"
#include "builder.h"


FunctionValueType::FunctionValueType(Runtime *r, Function *function) :
	ValueType(r),
	mReturnType(function->returnValue()),
	mParamTypes(function->paramTypes()),
	mFunction(function) {

	std::vector<llvm::Type*> params;
	params.reserve(mParamTypes.size());
	for (ValueType *vt : mParamTypes) {
		params.push_back(vt->llvmType());
	}
	if (mReturnType == 0) {
		mType = llvm::FunctionType::get(llvm::Type::getVoidTy(r->module()->getContext()), params, false);
	} else {
		mType = llvm::FunctionType::get(mReturnType->llvmType(), params, false);
	}
}

QString FunctionValueType::name() const {
	if (mReturnType) {
		return QStringLiteral("Function (") % listStringJoin(mParamTypes, [](ValueType *valTy) {
			return valTy->name();
		}) % QStringLiteral(") As ") % mReturnType->name();
	}
	else {
		return QStringLiteral("Command (") % listStringJoin(mParamTypes, [](ValueType *valTy) {
			return valTy->name();
		}) % QStringLiteral(")");
	}
}


llvm::Constant *FunctionValueType::defaultValue() const {
	assert("FunctionValueType::defaultValue not implemented" && 0);
	return 0;
}


int FunctionValueType::size() const {
	return mRuntime->dataLayout().getTypeStoreSize(mType);
}

ValueType::CastCost FunctionValueType::castingCostToOtherValueType(const ValueType *to) const {
	if (to != this) return ValueType::ccNoCast;
	return ValueType::ccNoCost;
}

Value FunctionValueType::cast(Builder *, const Value &v) const {
	if (v.valueType() != this) return Value();
	return v;
}


