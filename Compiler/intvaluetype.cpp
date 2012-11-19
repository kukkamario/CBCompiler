#include "intvaluetype.h"
#include "llvm.h"
#include "value.h"
#include "runtime.h"
IntValueType::IntValueType(Runtime *r, llvm::Module *mod) :
	ValueType(r){
	mType = llvm::IntegerType::get(mod->getContext(), 32);
}



ValueType::CastCostType IntValueType::castCost(ValueType *to) const {
	switch (to->type()) {
		case ValueType::Integer:
			return 0;
		case ValueType::Boolean:
			return 1;
		case ValueType::Float:
			return 2;
		case ValueType::Short:
			return 3;
		case ValueType::Byte:
			return 4;
		case ValueType::String:
			return 10;
		default:
			return maxCastCost;
	}
}

Value IntValueType::cast(llvm::IRBuilder<> *builder, const Value &v) const {
	/*if (v.isConstant()) {
		switch (v.valueType()->type()) {
			case ValueType::Integer:
				return v;
			case ValueType::Boolean:
				return v.mBool ? 1 : 0;
			case ValueType::Float:
				return int(v.mConstantData.mFloat + 0.5f);
			case ValueType::Short:
				return v.mConstantData.mShort;
			case ValueType::Byte:
				return v.mConstantData.mByte;
			case ValueType::String: {
				if (v.mConstantData.mString)
					return v.mConstantData.mString->toInt();
				return 0;
			}
			default:
				assert(0 == "WTF");
				return Value();
		}
	}

	switch (v.valueType()->type()) {
		case ValueType::Integer:
			return ;
		case ValueType::Boolean: {

		}
		case ValueType::Float:
			return 2;
		case ValueType::Short:
			return 3;
		case ValueType::Byte:
			return 4;
		case ValueType::String:
			return 10;
		default:
			return maxCastCost;
	}*/
	return Value();
}

llvm::Value *IntValueType::constant(int v) {
	return llvm::ConstantInt::get(mType, llvm::APInt(32, v, true));
}
