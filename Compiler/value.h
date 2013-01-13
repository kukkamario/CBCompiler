#ifndef VALUE_H
#define VALUE_H
class ValueType;
#include "llvm.h"
#include <QString>
#include "constantvalue.h"
class Value {
	public:
		Value();
		Value(const Value &value);
		Value(const ConstantValue &c);
		Value(ValueType *t, llvm::Value *v);

		ValueType *valueType() const {return mValueType;}
		bool isConstant()const{return mConstant.isValid();}
		bool isValid() {return mValueType != 0;}
		llvm::Value *value() {return mValue;}
		const ConstantValue &constant() const {return mConstant;}
	private:
		ValueType *mValueType;
		llvm::Value *mValue;
		ConstantValue mConstant;
};

#endif // VALUE_H
