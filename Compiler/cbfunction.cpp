#include "cbfunction.h"
#include "scope.h"
#include "variablesymbol.h"
#include "value.h"
#include "runtime.h"
#include "builder.h"

CBFunction::CBFunction(const QString &name, ValueType *retValue, const QList<CBFunction::Parameter> &params, Scope *scope, int line, QFile *file):
	Function(name, file, line),
	mParams(params),
	mScope(scope) {
	mReturnValue = retValue;
	mRequiredParams = 0;
	foreach(const Parameter &param, mParams) {
		if (!param.mDefaultValue.isValid()) mRequiredParams++;
		mParamTypes.append(param.mVariableSymbol->valueType());
	}
}

void CBFunction::generateFunction(Runtime *runtime) {
	std::vector<llvm::Type*> parameterTypes;
	foreach(const Parameter &param, mParams) {
		parameterTypes.push_back(param.mVariableSymbol->valueType()->llvmType());
	}
	llvm::FunctionType *funcTy = llvm::FunctionType::get(mReturnValue->llvmType(), parameterTypes, false);
	mFunction = llvm::Function::Create(funcTy, llvm::Function::PrivateLinkage, "CBF_user_" + mName.toStdString(), runtime->module());
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
		return Value(mReturnValue, ret);
	}
}
