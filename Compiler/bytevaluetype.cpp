#include "bytevaluetype.h"
#include "llvm.h"
#include "value.h"
#include "builder.h"
#include "abstractsyntaxtree.h"


ByteValueType::ByteValueType(Runtime *r, llvm::Module *mod) :
	ValueType(r){
	mType = llvm::Type::getInt8Ty(mod->getContext());
}

ValueType::CastCost ByteValueType::castingCostToOtherValueType(ValueType *to) const {
	switch (to->type()) {
		case ValueType::Byte:
			return 0;
		case ValueType::Boolean:
			return 1;
		case ValueType::Short:
			return 1;
		case ValueType::Integer:
			return 1;
		case ValueType::Float:
			return 2;
		case ValueType::String:
			return 10;
		default:
			return sMaxCastCost;
	}
}

Value ByteValueType::cast(Builder *builder, const Value &v) const {
	return builder->toByte(v);
}

llvm::Constant *ByteValueType::constant(quint8 i) const {
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

