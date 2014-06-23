#include "genericarrayvaluetype.h"
#include "runtime.h"

GenericArrayValueType::GenericArrayValueType(llvm::PointerType *genericArrayPointerType, Runtime *r) :
	ValueType(r, genericArrayPointerType){
}

GenericArrayValueType::~GenericArrayValueType() {

}

QString GenericArrayValueType::name() const {
	return QStringLiteral("GenericArrayValueType");
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
	if (to == this) return ccNoCost;
	return ccNoCast;
}
