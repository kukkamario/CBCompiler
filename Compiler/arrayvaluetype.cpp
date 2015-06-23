#include "arrayvaluetype.h"
#include "genericarrayvaluetype.h"
#include "builder.h"
#include "runtime.h"
#include "abstractsyntaxtree.h"
#include "intvaluetype.h"

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
		return operand1;
	}

	operationFlags |= OperationFlag::NoSuchOperation;
	return Value();
}

void ArrayValueType::generateDestructor(Builder *builder, const Value &value) {
	if (value.isReference() || value.isConstant()) return;
	destructArray(builder, value.value());
}

Value ArrayValueType::generateLoad(Builder *builder, const Value &var) const {
	assert(var.isReference());
	llvm::Value *v = builder->CreateLoad(var.value());
	refArray(builder, v);
	return Value(var.valueType(), v, false);
}

Value ArrayValueType::dimensionSize(Builder *builder, const Value &array, const Value &dimNum) {
	llvm::Value * arr = builder->llvmValue(array);
	llvm::Value *arrayDataHeader = builder->CreateStructGEP(arr, 0);
	llvm::Value *sizes = builder->CreateStructGEP(arrayDataHeader, 1);
	llvm::Value *gepParams[2];
	gepParams[0] = builder->getInt32(0);
	gepParams[1] = builder->llvmValue(builder->toInt(dimNum));
	llvm::Value *size = builder->CreateLoad(builder->CreateGEP(sizes, gepParams));
	if (size->getType() != builder->getInt32Ty()) {
		size = builder->CreateTrunc(size, builder->getInt32Ty());
	}
	return Value(mRuntime->intValueType(), size, false);
}

void ArrayValueType::assignArray(Builder *builder, llvm::Value *var, llvm::Value *array) {
	builder->CreateCall2(
				mRuntime->genericArrayValueType()->assignmentFunction(),
				builder->bitcast(mRuntime->genericArrayValueType()->llvmType()->getPointerTo(), var),
				builder->bitcast(mRuntime->genericArrayValueType()->llvmType(), array));
}

Value ArrayValueType::constructArray(Builder *builder, const QList<Value> &dims) {
	assert(dims.size() == mDimensions);
	if (!mConstructFunction) {
		llvm::Type *intT = builder->getIntPtrTy(&mRuntime->dataLayout());
		std::vector<llvm::Type*> paramTypes;
		for (int i = 0; i != mDimensions; i++) {
			paramTypes.push_back(intT);
		}


		llvm::FunctionType *ft = llvm::FunctionType::get(llvmType(), paramTypes, false);
		mConstructFunction = llvm::Function::Create(ft, llvm::Function::InternalLinkage, ("constructor_" + name()).toStdString(), mRuntime->module());
		llvm::BasicBlock *bb = llvm::BasicBlock::Create(builder->context(), "bb", mConstructFunction);
		llvm::IRBuilder<> irBuilder(bb);
		llvm::AllocaInst *pArr = builder->CreateAlloca(intT, builder->getInt32(mDimensions));

		llvm::Function::arg_iterator arg_i = mConstructFunction->arg_begin();
		for (int i = 0; i != mDimensions; i++) {
			llvm::Value *dPtr = builder->CreateGEP(pArr, builder->getInt32(i));
			builder->CreateStore(arg_i, dPtr);
			arg_i++;
		}

		llvm::Value *genericArray = builder->CreateCall3(
					mRuntime->genericArrayValueType()->constructFunction(),
					llvm::ConstantInt::get(intT, mDimensions),
					pArr,
					llvm::ConstantInt::get(intT, mBaseValueType->size()));
		builder->CreateRet(builder->CreateBitCast(genericArray, llvmType()));
	}

	std::vector<llvm::Value*> params;
	for (const Value &v : dims) {
		params.push_back(builder->intPtrTypeValue(v));
	}

	return Value(this, builder->CreateCall(mConstructFunction, params));
}

Value ArrayValueType::arraySubscript(Builder *builder, const Value &array, const QList<Value> &dims) {
	assert(array.valueType() == this);
	assert(mDimensions == dims.size());
	llvm::Value *arr = builder->llvmValue(array);
	llvm::Value *arrayDataHeader = builder->CreateStructGEP(arr, 0);
	llvm::Value *genericHeader = builder->CreateStructGEP(arrayDataHeader, 0);
	llvm::Value *offset = builder->CreateStructGEP(genericHeader, 4);
	llvm::Value *mults = builder->CreateStructGEP(arrayDataHeader, 2);


	int i = 0;
	llvm::Value *sum = 0;
	llvm::Value *gepParams[2];
	gepParams[0] = builder->getInt32(0);
	for (const Value &val : dims) {
		llvm::Value *v = builder->intPtrTypeValue(val);
		gepParams[1] = builder->getInt32(i);
		llvm::Value *mult = builder->CreateLoad(builder->CreateGEP(mults, gepParams));
		llvm::Value *r = builder->CreateMul(v, mult);
		if (sum) {
			sum = builder->CreateAdd(sum, r);
		} else {
			sum = r;
		}
		i++;
	}

	llvm::Value *arrData = builder->CreateBitCast(arr, builder->getInt8PtrTy());
	arrData = builder->CreateGEP(arrData, builder->CreateLoad(offset));

	arrData = builder->CreateBitCast(arrData, mBaseValueType->llvmType()->getPointerTo());
	arrData = builder->CreateGEP(arrData, sum);
	return Value(mBaseValueType, arrData, true);

}

void ArrayValueType::refArray(Builder *builder, llvm::Value *array) const {
	builder->CreateCall(
				mRuntime->genericArrayValueType()->refFunction(),
				builder->bitcast(mRuntime->genericArrayValueType()->llvmType(), array));
}

void ArrayValueType::destructArray(Builder *builder, llvm::Value *array) {
	builder->CreateCall(
				mRuntime->genericArrayValueType()->destructFunction(),
				builder->bitcast(mRuntime->genericArrayValueType()->llvmType(), array));
}

llvm::Value *ArrayValueType::dataArray(Builder *builder, const Value &array) {
	llvm::Value *arr = builder->llvmValue(array);
	llvm::Value *arrayDataHeader = builder->CreateStructGEP(arr, 0);
	llvm::Value *genericHeader = builder->CreateStructGEP(arrayDataHeader, 0);
	llvm::Value *offset = builder->CreateLoad(builder->CreateStructGEP(genericHeader, 4));
	llvm::Value *arrData = builder->CreateBitCast(arr, builder->getInt8PtrTy());
	arrData = builder->CreateGEP(arrData, offset);
	return builder->CreateBitCast(arrData, mBaseValueType->llvmType()->getPointerTo());
}

llvm::Value *ArrayValueType::totalSize(Builder *builder, const Value &array) {
	llvm::Value *arr = builder->llvmValue(array);
	llvm::Value *arrayDataHeader = builder->CreateStructGEP(arr, 0);
	llvm::Value *genericHeader = builder->CreateStructGEP(arrayDataHeader, 0);
	llvm::Value *fullSize = builder->CreateLoad(builder->CreateStructGEP(genericHeader, 0));
	return fullSize;
}
