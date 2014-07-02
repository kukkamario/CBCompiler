#include "genericstructvaluetype.h"
#include "builder.h"

GenericStructValueType::GenericStructValueType(llvm::Type *genericStructValueType, Runtime *runtime) :
	ValueType(runtime, genericStructValueType){
}

QString GenericStructValueType::name() const {
	return QStringLiteral("GenericStructValueType");
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
		return Value(const_cast<GenericStructValueType*>(this), builder->bitcast(mType, builder->llvmValue(v)));
	}
	return Value();
}

llvm::Constant *GenericStructValueType::defaultValue() const {
	return llvm::ConstantPointerNull::get(llvm::cast<llvm::PointerType>(mType));
}

int GenericStructValueType::size() const {
	return mRuntime->dataLayout().getPointerSize();
}
