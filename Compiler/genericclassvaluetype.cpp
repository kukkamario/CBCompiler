#include "genericclassvaluetype.h"
#include "builder.h"

GenericClassValueType::GenericClassValueType(llvm::Type *genericClassValueType, Runtime *runtime) :
	ValueType(runtime, genericClassValueType){
}

QString GenericClassValueType::name() const {
	return QStringLiteral("GenericClassValueType");
}

ValueType::CastCost GenericClassValueType::castingCostToOtherValueType(const ValueType *to) const {
	if (to == this) return ccNoCost;

	return ccNoCast;
}

Value GenericClassValueType::cast(Builder *builder, const Value &v) const {
	if (v.valueType() == this) {
		return v;
	}
	if (v.valueType()->isClass() && v.isNormalValue()) {
		return Value(const_cast<GenericClassValueType*>(this), builder->bitcast(mType, builder->llvmValue(v)));
	}
	return Value();
}

llvm::Constant *GenericClassValueType::defaultValue() const {
	return llvm::ConstantPointerNull::get(llvm::cast<llvm::PointerType>(mType));
}

int GenericClassValueType::size() const {
	return mRuntime->dataLayout().getPointerSize();
}
