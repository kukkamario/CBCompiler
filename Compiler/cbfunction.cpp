#include "cbfunction.h"
#include "scope.h"
#include "variablesymbol.h"
#include "value.h"
#include "runtime.h"
#include "builder.h"

CBFunction::CBFunction(const std::string &name, ValueType *retValue, const std::vector<CBFunction::Parameter> &params, Scope *scope, const CodePoint &cp):
	Function(name, cp),
	mParams(params),
	mScope(scope),
	mFunctionValueType(0) {
	mReturnValue = retValue;
	mRequiredParams = 0;
	foreach(const Parameter &param, mParams) {
		if (!param.mDefaultValue.isValid()) mRequiredParams++;
		mParamTypes.push_back(param.mVariableSymbol->valueType());
	}
}

CBFunction::~CBFunction() {
	delete mFunctionValueType;
}

void CBFunction::generateFunction(Runtime *runtime) {
	mFunctionValueType = runtime->valueTypeCollection().functionValueType(mReturnValue, mParamTypes);
	mFunction = llvm::Function::Create(llvm::cast<llvm::FunctionType>(mFunctionValueType->llvmType()->getContainedType(0)), llvm::Function::PrivateLinkage, "CBF_user_" + mName, runtime->module());
}


Value CBFunction::call(Builder *builder, const std::vector<Value> &params) {
	std::vector<llvm::Value*> p;
	std::vector<Parameter>::const_iterator paramI = mParams.begin();
	foreach(const Value &val, params) {
		p.push_back(builder->llvmValue(val));
		paramI++;
	}

	//Default parameters
	for (size_t i = params.size(); i < mParams.size(); i++) {
		Value val = paramI->mVariableSymbol->valueType()->cast(builder, Value(paramI->mDefaultValue, builder->runtime()));
		p.push_back(builder->llvmValue(val));
		paramI++;
	}

	llvm::Value *ret = builder->irBuilder().CreateCall(mFunction, p);
	if (isCommand()) {
		return Value();
	}
	else {
		return Value(mReturnValue, ret, false);
	}
}
