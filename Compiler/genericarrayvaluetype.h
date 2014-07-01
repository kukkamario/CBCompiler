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
	bool isArray() const { return true; }
	bool isNumber() const { return false; }
	llvm::Constant *defaultValue() const;
	llvm::StructType *structType() const;

	int size() const;

	bool canBeCastedToValueType(ValueType *to) const;
	CastCost castingCostToOtherValueType(const ValueType *to) const;

	Value cast(Builder *builder, const Value &v) const;

	llvm::LLVMContext &context();
	Runtime *runtime() const { return mRuntime; }

	bool setConstructFunction(llvm::Function *f);
	bool setDestructFunction(llvm::Function *f);
	bool setRefFunction(llvm::Function *f);
	bool setAssignmentFunction(llvm::Function *f);
	llvm::Function *constructFunction() const { return mConstructFunction; }
	llvm::Function *destructFunction() const { return mDestructFunction; }
	llvm::Function *refFunction() const { return mRefFunction; }
	llvm::Function *assignmentFunction() const { return mAssignmentFunction; }

private:
	llvm::Function *mConstructFunction;
	llvm::Function *mRefFunction;
	llvm::Function *mDestructFunction;
	llvm::Function *mAssignmentFunction;
};

#endif // GENERICARRAYVALUETYPE_H
