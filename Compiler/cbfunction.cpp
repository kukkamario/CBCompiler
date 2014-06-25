#include "cbfunction.h"
#include "scope.h"
#include "variablesymbol.h"
#include "value.h"
#include "runtime.h"
#include "builder.h"

CBFunction::CBFunction(const QString &name, ValueType *retValue, const QList<CBFunction::Parameter> &params, Scope *scope, const CodePoint &cp):
	Function(name, cp),
	mParams(params),
	mScope(scope),
	mFunctionValueType(0) {
	mReturnValue = retValue;
	mRequiredParams = 0;
	foreach(const Parameter &param, mParams) {
		if (!param.mDefaultValue.isValid()) mRequiredParams++;
		mParamTypes.append(param.mVariableSymbol->valueType());
	}

}

CBFunction::~CBFunction() {
	delete mFunctionValueType;
}

void CBFunction::generateFunction(Runtime *runtime) {
	mFunctionValueType = new FunctionValueType(mReturnValue->runtime(), mReturnValue, mParamTypes);
	mFunction = llvm::Function::Create(llvm::cast<llvm::FunctionType>(mFunctionValueType->llvmType()), llvm::Function::PrivateLinkage, "CBF_user_" + mName.toStdString(), runtime->module());
}


Value CBFunction::call(Builder *builder, const QList<Value> &params) {
	std::vector<llvm::Value*> p;
	QList<Parameter>::ConstIterator paramI = mParams.begin();
	foreach(const Value &val, params) {
		p.push_back(builder->llvmValue(val));
		paramI++;
	}

	//Default parameters
	for (int i = params.size(); i < mParams.size(); i++) {
		p.push_back(builder->llvmValue(paramI->mDefaultValue));
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
