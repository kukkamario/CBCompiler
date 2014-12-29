#include "genericarrayvaluetype.h"
#include "runtime.h"
#include "builder.h"

GenericArrayValueType::GenericArrayValueType(llvm::PointerType *genericArrayPointerType, Runtime *r) :
	ValueType(r, genericArrayPointerType),
	mConstructFunction(0),
	mRefFunction(0),
	mDestructFunction(0),
	mAssignmentFunction(0)
{
}

GenericArrayValueType::~GenericArrayValueType() {

}

std::string GenericArrayValueType::name() const {
	return std::stringLiteral("GenericArrayValueType");
}

llvm::Constant *GenericArrayValueType::defaultValue() const {
	return llvm::ConstantPointerNull::get(llvm::cast<llvm::PointerType>(mType));
}

llvm::StructType *GenericArrayValueType::structType() const {
	llvm::PointerType *ptrTy = llvm::cast<llvm::PointerType>(mType);
	return llvm::cast<llvm::StructType>(ptrTy->getElementType());
}

int GenericArrayValueType::size() const {
	return mRuntime->dataLayout().getPointerSize();
}

ValueType::CastCost GenericArrayValueType::castingCostToOtherValueType(const ValueType *to) const {
	if (to->isArray()) return ccNoCost;
	return ccNoCast;
}

Value GenericArrayValueType::cast(Builder *builder, const Value &v) const {
	if (!v.valueType()->isArray()) {
		return Value();
	}
	return Value(const_cast<GenericArrayValueType*>(this), builder->bitcast(mType, builder->llvmValue(v)));
}


bool GenericArrayValueType::setAssignmentFunction(llvm::Function *func) {
	llvm::FunctionType *funcTy = func->getFunctionType();
	if (funcTy->getReturnType() != llvm::Type::getVoidTy(func->getContext())) return false;
	if (funcTy->getNumParams() != 2) return false;
	llvm::FunctionType::param_iterator i = funcTy->param_begin();
	const llvm::Type *const arg1 = *i;
	if (arg1 != mType->getPointerTo()) return false;
	i++;
	const llvm::Type *const arg2 = *i;
	if (arg2 != mType) return false;

	mAssignmentFunction = func;
	return true;
}


bool GenericArrayValueType::setConstructFunction(llvm::Function *func) {
	llvm::FunctionType *funcTy = func->getFunctionType();
	if (funcTy->getReturnType() != llvmType()) return false;
	if (funcTy->getNumParams() != 3) return false;
	llvm::Type *intT = llvm::IntegerType::getIntNTy(mRuntime->module()->getContext(), mRuntime->dataLayout().getPointerSizeInBits());
	llvm::FunctionType::param_iterator i = funcTy->param_begin();

	const llvm::Type *const arg1 = *i;
	if (arg1 != intT) return false;
	i++;

	const llvm::Type *const arg2 = *i;
	if (arg2 != intT->getPointerTo()) return false;
	i++;

	const llvm::Type *const arg3 = *i;
	if (arg3 != intT) return false;

	mConstructFunction = func;
	return true;
}

bool GenericArrayValueType::setDestructFunction(llvm::Function *func) {
	llvm::FunctionType *funcTy = func->getFunctionType();
	if (funcTy->getReturnType() != llvm::Type::getVoidTy(func->getContext())) return false;
	if (funcTy->getNumParams() != 1) return false;
	llvm::FunctionType::param_iterator i = funcTy->param_begin();
	const llvm::Type *const arg1 = *i;
	if (arg1 != mType) return false;
	mDestructFunction = func;
	return true;
}

bool GenericArrayValueType::setRefFunction(llvm::Function *func) {
	llvm::FunctionType *funcTy = func->getFunctionType();
	if (funcTy->getReturnType() != llvm::Type::getVoidTy(func->getContext())) return false;
	if (funcTy->getNumParams() != 1) return false;
	llvm::FunctionType::param_iterator i = funcTy->param_begin();
	const llvm::Type *const arg1 = *i;
	if (arg1 != mType) return false;
	i++;

	mRefFunction = func;
	return true;
}
