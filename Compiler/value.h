#ifndef VALUE_H
#define VALUE_H
#include "llvm.h"
#include <QString>
#include "constantvalue.h"

class Builder;
class Runtime;
class ValueType;
class Value {
	public:
		Value();
		Value(const Value &value);
		Value(const ConstantValue &c, Runtime *r);
		Value(ValueType *t, llvm::Value *v, bool reference);

		ValueType *valueType() const {return mValueType;}
		bool isConstant()const{return mConstant.isValid();}
		bool isValid() const{return mValueType != 0 && (isConstant() ? mConstant.isValid() : mValue != 0);}
		llvm::Value *value() const {return mValue;}
		const ConstantValue &constant() const {return mConstant;}
		void toLLVMValue(Builder *builder);
		bool isReference() const { return mReference; }
		void dump() const;
	private:
		ValueType *mValueType;
		llvm::Value *mValue;
		ConstantValue mConstant;
		bool mReference;
};

#endif // VALUE_H
