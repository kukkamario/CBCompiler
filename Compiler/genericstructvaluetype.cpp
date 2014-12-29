#include "genericstructvaluetype.h"
#include "builder.h"

GenericStructValueType::GenericStructValueType(llvm::Type *genericStructValueType, Runtime *runtime) :
	ValueType(runtime, genericStructValueType){
}

std::string GenericStructValueType::name() const {
	return std::stringLiteral("GenericStructValueType");
}

ValueType::CastCost GenericStructValueType::castingCostToOtherValueType(const ValueType *to) const {
	if (to == this) return ccNoCost;

	return ccNoCast;
}

Value GenericStructValueType::cast(Builder *builder, const Value &v) const {
	if (v.valueType() == this) {
		return v;
	}
	if (v.valueType()->isStruct() && v.isNormalValue()) {
		if (v.isReference()) {
			return Value(const_cast<GenericStructValueType*>(this), builder->bitcast(mType, v.value()));
		}
		llvm::Value *alloca = builder->irBuilder().CreateAlloca(v.valueType()->llvmType());
		builder->store(alloca, v.value());
		return Value(const_cast<GenericStructValueType*>(this), builder->bitcast(mType, alloca));
	}
	return Value();
}

llvm::Constant *GenericStructValueType::defaultValue() const {
	return llvm::ConstantPointerNull::get(llvm::cast<llvm::PointerType>(mType));
}

int GenericStructValueType::size() const {
	return mRuntime->dataLayout().getPointerSize();
}
