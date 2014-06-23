#include "functionvaluetype.h"
#include "liststringjoin.h"
#include "value.h"
#include "function.h"
#include "builder.h"

FunctionValueType::FunctionValueType(Runtime *r, ValueType *returnValue, const QList<ValueType *> paramTypes) :
	ValueType(r),
	mReturnType(returnValue),
	mParamTypes(paramTypes) {
	std::vector<llvm::Type*> params;
	params.reserve(paramTypes.size());
	for (ValueType *vt : paramTypes) {
		params.push_back(vt->llvmType());
	}
	if (returnValue == 0) {
		mType = llvm::FunctionType::get(llvm::Type::getVoidTy(r->module()->getContext()), params, false);
	} else {
		mType = llvm::FunctionType::get(returnValue->llvmType(), params, false);
	}

}


QString FunctionValueType::name() const {
	return QStringLiteral("Function (") % listStringJoin(mParamTypes, [](ValueType *valTy) {
		return valTy->name();
	}) % QStringLiteral(") As ") % mReturnType->name();
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


