#ifndef VALUE_H
#define VALUE_H
#include "llvm.h"
#include <QString>
#include "constantvalue.h"

class Builder;
class Runtime;
class ValueType;
class FunctionSelectorValueType;
class Value {
	public:
		Value();
		Value(const Value &value);
		Value(const ConstantValue &c, Runtime *r);
		Value(ValueType *t, llvm::Value *v, bool reference = false);
		explicit Value(ValueType *valType);
		explicit Value(FunctionSelectorValueType *t);
		~Value();

		Value &operator=(const Value &value);

		ValueType *valueType() const {return mValueType;}
		bool isConstant()const { return mType == tConstant; }
		bool isValid() const;
		llvm::Value *value() const {return mValue;}
		const ConstantValue &constant() const {return mConstant;}
		void toLLVMValue(Builder *builder);
		bool isReference() const { return mType == tReference; }
		void dump() const;
	private:
		ValueType *mValueType;
		llvm::Value *mValue;
		ConstantValue mConstant;
		enum {
			tNormalValue,
			tReference,
			tConstant,
			tValueType,
			tFunctionSelectorValueType
		} mType;
};

#endif // VALUE_H
