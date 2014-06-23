#ifndef GENERICARRAYVALUETYPE_H
#define GENERICARRAYVALUETYPE_H
#include "llvm.h"
#include "global.h"
#include "valuetype.h"

class GenericArrayValueType : public ValueType {
public:
	GenericArrayValueType(llvm::PointerType *genericArrayPointerType, Runtime *r);
	~GenericArrayValueType();
	QString name() const;
	bool isNamedValueType() const { return false; }
	bool isTypePointer() const { return false; }
	bool isNumber() const { return false; }
	llvm::Constant *defaultValue() const;
	llvm::StructType *structType() const;

	int size() const;

	bool canBeCastedToValueType(ValueType *to) const;
	CastCost castingCostToOtherValueType(const ValueType *to) const;

	virtual Value cast(Builder *builder, const Value &v) const = 0;

	llvm::LLVMContext &context();
	Runtime *runtime() const { return mRuntime; }
};

#endif // GENERICARRAYVALUETYPE_H
