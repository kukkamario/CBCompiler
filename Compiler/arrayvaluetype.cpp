#include "arrayvaluetype.h"
#include "genericarrayvaluetype.h"
#include "builder.h"
#include "runtime.h"
#include "abstractsyntaxtree.h"

ArrayValueType::ArrayValueType(ValueType *baseType, llvm::Type *llvmType, int dimensions):
	ValueType(baseType->runtime(), llvmType),
	mBaseValueType(baseType),
	mConstructFunction(0),
	mDimensions(dimensions) {
	assert(dimensions > 0);
}

QString ArrayValueType::name() const {
	return mBaseValueType->name() + "[" + QString(",").repeated(mDimensions - 1) + "]";
}

ValueType::CastCost ArrayValueType::castingCostToOtherValueType(const ValueType *to) const {
	if (to == this) return ccNoCost;
	return ccNoCast;
}

Value ArrayValueType::cast(Builder *, const Value &v) const {
	if (v.valueType() == this) return v;

	return Value();
}


llvm::Constant *ArrayValueType::defaultValue() const {
	return llvm::ConstantPointerNull::get(llvm::cast<llvm::PointerType>(mType));
}

int ArrayValueType::size() const {
	return mRuntime->dataLayout().getPointerSize();
}

Value ArrayValueType::generateOperation(Builder *builder, int opType, const Value &operand1, const Value &operand2, OperationFlags &operationFlags) const {
	if (opType == ast::ExpressionNode::opAssign) {
		if (!operand1.isReference()) {
			operationFlags |= OperationFlag::ReferenceRequired;
			return Value();
		}
		assert(operand1.valueType() == this && operand2.valueType() == this);
		builder->store(operand1, operand2);
	}

	operationFlags |= OperationFlag::NoSuchOperation;
	return Value();
}

void ArrayValueType::assignArray(Builder *builder, llvm::Value *var, llvm::Value *array) {
	builder->irBuilder().CreateCall2(
				mRuntime->genericArrayValueType()->assignmentFunction(),
				builder->bitcast(mRuntime->genericArrayValueType()->llvmType()->getPointerTo(), var),
				builder->bitcast(mRuntime->genericArrayValueType()->llvmType(), array));
}

Value ArrayValueType::constructArray(Builder *builder, const QList<Value> &dims) {
	assert(dims.size() == mDimensions);
	if (!mConstructFunction) {
		llvm::Type *intT = builder->irBuilder().getIntPtrTy(&mRuntime->dataLayout());
		std::vector<llvm::Type*> paramTypes;
		for (int i = 0; i != mDimensions; i++) {
			paramTypes.push_back(intT);
		}


		llvm::FunctionType *ft = llvm::FunctionType::get(llvmType(), paramTypes, false);
		mConstructFunction = llvm::Function::Create(ft, llvm::Function::InternalLinkage, ("constructor_" + name()).toStdString(), mRuntime->module());
		llvm::BasicBlock *bb = llvm::BasicBlock::Create(builder->context(), "bb", mConstructFunction);
		llvm::IRBuilder<> irBuilder(bb);
		llvm::AllocaInst *pArr = irBuilder.CreateAlloca(intT, irBuilder.getInt32(mDimensions));

		llvm::Function::arg_iterator arg_i = mConstructFunction->arg_begin();
		for (int i = 0; i != mDimensions; i++) {
			llvm::Value *dPtr = irBuilder.CreateGEP(pArr, irBuilder.getInt32(i));
			irBuilder.CreateStore(arg_i, dPtr);
			arg_i++;
		}

		llvm::Value *genericArray = irBuilder.CreateCall3(
					mRuntime->genericArrayValueType()->constructFunction(),
					llvm::ConstantInt::get(intT, mDimensions),
					pArr,
					llvm::ConstantInt::get(intT, mBaseValueType->size()));
		irBuilder.CreateRet(irBuilder.CreateBitCast(genericArray, llvmType()));
	}

	std::vector<llvm::Value*> params;
	for (const Value &v : dims) {
		params.push_back(builder->intPtrTypeValue(v));
	}

	return Value(this, builder->irBuilder().CreateCall(mConstructFunction, params));
}

Value ArrayValueType::arraySubscript(Builder *builder, const Value &array, const QList<Value> &dims) {
	assert(array.valueType() == this);
	assert(mDimensions == dims.size());
	llvm::Value *arr = builder->llvmValue(array);
	llvm::IRBuilder<> &irBuilder = builder->irBuilder();
	llvm::Value *arrayDataHeader = irBuilder.CreateStructGEP(arr, 0);
	llvm::Value *genericHeader = irBuilder.CreateStructGEP(arrayDataHeader, 0);
	llvm::Value *offset = irBuilder.CreateStructGEP(genericHeader, 4);
	llvm::Value *mults = irBuilder.CreateStructGEP(arrayDataHeader, 2);


	int i = 0;
	llvm::Value *sum = 0;
	llvm::Value *gepParams[2];
	gepParams[0] = irBuilder.getInt32(0);
	for (const Value &val : dims) {
		llvm::Value *v = builder->intPtrTypeValue(val);
		gepParams[1] = irBuilder.getInt32(i);
		llvm::Value *mult = irBuilder.CreateLoad(irBuilder.CreateGEP(mults, gepParams));
		llvm::Value *r = irBuilder.CreateMul(v, mult);
		if (sum) {
			sum = irBuilder.CreateAdd(sum, r);
		} else {
			sum = r;
		}
		i++;
	}

	llvm::Value *arrData = irBuilder.CreateBitCast(arr, irBuilder.getInt8PtrTy());
	arrData = irBuilder.CreateGEP(arrData, irBuilder.CreateLoad(offset));

	arrData = irBuilder.CreateBitCast(arrData, mBaseValueType->llvmType()->getPointerTo());
	arrData = irBuilder.CreateGEP(arrData, sum);
	return Value(mBaseValueType, arrData, true);

}

void ArrayValueType::refArray(Builder *builder, llvm::Value *array) {
	builder->irBuilder().CreateCall(
				mRuntime->genericArrayValueType()->refFunction(),
				builder->bitcast(mRuntime->genericArrayValueType()->llvmType(), array));
}

void ArrayValueType::destructArray(Builder *builder, llvm::Value *array) {
	builder->irBuilder().CreateCall(
				mRuntime->genericArrayValueType()->destructFunction(),
				builder->bitcast(mRuntime->genericArrayValueType()->llvmType(), array));
}
