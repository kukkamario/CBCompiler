#include "runtimefunction.h"
#include "runtime.h"
#include "valuetype.h"
#include "stringvaluetype.h"
#include "floatvaluetype.h"
#include "intvaluetype.h"
#include "shortvaluetype.h"
#include "bytevaluetype.h"
#include "typepointervaluetype.h"
#include "typevaluetype.h"
#include "value.h"
#include "builder.h"

RuntimeFunction::RuntimeFunction(Runtime *r) :
	Function(QString(), 0,0),
	mRuntime(r){

}

bool RuntimeFunction::construct(llvm::Function *func, const QString &name) {
	mFunction = func;
	llvm::FunctionType *funcTy = func->getFunctionType();
	llvm::Type *retTy = funcTy->getReturnType();
	if ( retTy == llvm::Type::getVoidTy(mRuntime->module()->getContext())) {
		mReturnValue = 0;
	}
	else {
		mReturnValue = mRuntime->valueTypeCollection().valueTypeForLLVMType(retTy);
		if (!mReturnValue) return false;
	}
	mName = name.toLower();
	for (llvm::FunctionType::param_iterator i = funcTy->param_begin(); i != funcTy->param_end(); i++) {
		ValueType *paramType = mRuntime->valueTypeCollection().valueTypeForLLVMType(*i);
		if (paramType) {
			mParamTypes.append(paramType);
		}
		else {
			qDebug("Dumping an invalid runtime function: ");
			funcTy->dump();
			qDebug("\n-------\n\n");
			return false;
		}
	}
	mRequiredParams = mParamTypes.size();
	return true;
}

Value RuntimeFunction::call(Builder *builder, const QList<Value> &params) {
	std::vector<llvm::Value*> p;
	foreach(const Value &val, params) {
		p.push_back(builder->llvmValue(val));
	}

	llvm::Value *ret = builder->irBuilder().CreateCall(mFunction, p);
	if (isCommand()) {
		return Value();
	}
	else {
		return Value(mReturnValue, ret);
	}
}
