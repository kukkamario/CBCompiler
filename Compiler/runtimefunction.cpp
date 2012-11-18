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
	else {
		return false;
	}
}
