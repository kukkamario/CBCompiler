#include "runtimefunction.h"
#include "runtime.h"
#include "valuetype.h"
#include "stringvaluetype.h"
#include "floatvaluetype.h"
#include "intvaluetype.h"
#include "shortvaluetype.h"
#include "bytevaluetype.h"
#include "typepointervaluetype.h"
#include "functionvaluetype.h"
#include "typevaluetype.h"
#include "value.h"
#include "builder.h"

RuntimeFunction::RuntimeFunction(Runtime *r) :
	Function(QString(), CodePoint()),
	mRuntime(r),
	mFunctionValueType(0) {

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
	for (llvm::Function::arg_iterator i = func->arg_begin(); i != func->arg_end(); i++) {
		if (i->getArgNo() == 0 && i->hasStructRetAttr()) {
			retTy = llvm::dyn_cast<llvm::PointerType>(i->getType())->getElementType();
			mReturnValue = mRuntime->valueTypeCollection().valueTypeForLLVMType(retTy);
			continue;
		}

		ValueType *paramType = mRuntime->valueTypeCollection().valueTypeForLLVMType(i->getType());
		if (!paramType) {
			if (i->getType()->isPointerTy()) {
				llvm::Type *trueType = llvm::dyn_cast<llvm::PointerType>(i->getType())->getElementType();
				paramType = mRuntime->valueTypeCollection().valueTypeForLLVMType(trueType);
			}
		}

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
	mFunctionValueType = mRuntime->valueTypeCollection().functionValueType(mReturnValue, mParamTypes);
	return true;
}

Value RuntimeFunction::call(Builder *builder, const QList<Value> &params) {
	std::vector<llvm::Value*> p;
	bool returnInParameters = false;
	llvm::AllocaInst *returnValueAlloca = 0;
	std::vector<llvm::AllocaInst*> tempVars;
	if (!mFunction->arg_empty()) {
		llvm::Function::const_arg_iterator i = mFunction->arg_begin();
		if (i->hasStructRetAttr()) {
			returnValueAlloca = builder->temporaryVariable(mReturnValue->llvmType());
			tempVars.push_back(returnValueAlloca);
			p.insert(p.begin(), returnValueAlloca);
			i++;
			returnInParameters = true;
		}
		foreach(const Value &val, params) {
			if (i->getType() == val.valueType()->llvmType()->getPointerTo()) {
				if (val.isReference()) {
					p.push_back(val.value());
				}
				else {
					llvm::AllocaInst *allocaInst = builder->temporaryVariable(val.valueType()->llvmType());
					tempVars.push_back(allocaInst);
					builder->CreateStore(val.value(), allocaInst);
					p.push_back(allocaInst);
				}
			}
			else {
				assert(i->getType() == val.valueType()->llvmType());
				p.push_back(builder->llvmValue(val));

			}
			i++;
		}
	}

	llvm::Value *ret = builder->CreateCall(mFunction, p);
	Value returnValue;
	if (returnInParameters) {
		returnValue = Value(mReturnValue, builder->CreateLoad(returnValueAlloca));
	}
	else if (isCommand()) {
		returnValue = Value();
	} else {
		returnValue = Value(mReturnValue, ret, false);
	}

	for (llvm::AllocaInst *alloc : tempVars) {
		builder->removeTemporaryVariable(alloc);
	}
	return returnValue;
}

