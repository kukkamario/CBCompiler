#include "bytevaluetype.h"
#include "llvm.h"
#include "value.h"
#include "builder.h"
#include "abstractsyntaxtree.h"


ByteValueType::ByteValueType(Runtime *r, llvm::Module *mod) :
	ValueType(r){
	mType = llvm::Type::getInt8Ty(mod->getContext());
}

ValueType::CastCost ByteValueType::castingCostToOtherValueType(const ValueType *to) const {
	switch (to->basicType()) {
		case ValueType::Byte:
			return ccNoCost;
		case ValueType::Boolean:
			return ccCastToBoolean;
		case ValueType::Short:
		case ValueType::Integer:
			return ccCastToBigger;
		case ValueType::Float:
			return ccCastToFloat;
		case ValueType::String:
			return ccCastToString;
		default:
			return ccNoCast;
	}
}

Value ByteValueType::cast(Builder *builder, const Value &v) const {
	return builder->toByte(v);
}

llvm::Constant *ByteValueType::constant(uint8_t i) const {
	return llvm::ConstantInt::get(mType, llvm::APInt(8, i, false));
}

llvm::Constant *ByteValueType::defaultValue() const {
	return constant(0);
}


Value ByteValueType::generateOperation(Builder *builder, int opType, const Value &operand1, const Value &operand2, OperationFlags &operationFlags) const {
	return generateBasicTypeOperation(builder, opType, operand1, operand2, operationFlags);
}

Value ByteValueType::generateOperation(Builder *builder, int opType, const Value &operand, OperationFlags &operationFlags) const {
	return generateBasicTypeOperation(builder, opType, operand, operationFlags);
}

