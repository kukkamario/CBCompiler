#include "runtimefunction.h"
#include "runtime.h"
#include "valuetype.h"
#include "stringvaluetype.h"
#include "floatvaluetype.h"
#include "intvaluetype.h"
#include "shortvaluetype.h"
#include "bytevaluetype.h"

RuntimeFunction::RuntimeFunction(Runtime *r) :
	mRuntime(r) {

}

bool RuntimeFunction::construct(llvm::Function *func, const QString &name) {
	mFunction = func;
	llvm::FunctionType *funcTy = func->getFunctionType();
	llvm::Type *retTy = funcTy->getReturnType();
	if (retTy == mRuntime->stringValueType()->llvmType()) {
		mReturnValue = mRuntime->stringValueType();
	}
	else if (retTy == mRuntime->intValueType()->llvmType()) {
		mReturnValue = mRuntime->intValueType();
	}
	else if (retTy == mRuntime->floatValueType()->llvmType()) {
		mReturnValue = mRuntime->floatValueType();
	}
	else if (retTy == mRuntime->shortValueType()->llvmType()) {
		mReturnValue = mRuntime->shortValueType();
	}
	else if( retTy == mRuntime->byteValueType()->llvmType()) {
		mReturnValue = mRuntime->byteValueType();
	}
	else if ( retTy == llvm::Type::getVoidTy(mRuntime->module()->getContext())) {
		mReturnValue = 0;
	}
	else {
		return false;
	}
	mName = name;
	int paramNum = funcTy->getFunctionNumParams();
	if (mName.length() - paramNum < 1) return false;
	QString paramStr = mName.right(paramNum).toLower();
	mName.chop(paramNum);
	int param = 0;
	for (llvm::FunctionType::param_iterator i = funcTy->param_begin(); i != funcTy->param_end(); i++) {
		if (*i == mRuntime->stringValueType()->llvmType()) {
			if (paramStr.at(param) != 's') return false;
			mParamTypes.append(mRuntime->stringValueType());
		}
		else if (*i == mRuntime->intValueType()->llvmType()) {
			if (paramStr.at(param) != 'i') return false;
			mParamTypes.append(mRuntime->intValueType());
		}
		else if (*i == mRuntime->floatValueType()->llvmType()) {
			if (paramStr.at(param) != 'f') return false;
			mParamTypes.append(mRuntime->floatValueType());
		}
		else if (*i == mRuntime->shortValueType()->llvmType()) {
			if (paramStr.at(param) != 'w') return false;
			mParamTypes.append(mRuntime->shortValueType());
		}
		else if( *i == mRuntime->byteValueType()->llvmType()) {
			if (paramStr.at(param) != 'b') return false;
			mParamTypes.append(mRuntime->byteValueType());
		}
		else {
			return false;
		}
		param++;
	}
	return true;
}
