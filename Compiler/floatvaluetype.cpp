#include "floatvaluetype.h"
#include "llvm.h"
#include "value.h"


FloatValueType::FloatValueType(Runtime *runtime, llvm::Module *mod) :
	ValueType(runtime){
	mType = llvm::Type::getFloatTy(mod->getContext());
}

ValueType::CastCostType FloatValueType::castCost(ValueType *to) const {
	switch (to->type()) {
		case Float:
			return 0;
		case Boolean:
			return 1;
		case Integer:
			return 2;
		case String:
			return 10;
		case Short:
			return 3;
		case Byte:
			return 4;
		default:
			return maxCastCost;
	}
}

Value FloatValueType::cast(llvm::IRBuilder<> *builder, const Value &v) const {
	return Value();
}

llvm::Value *FloatValueType::constant(float f) {
	return llvm::ConstantFP::get(mType->getContext(), llvm::APFloat(f));
}
