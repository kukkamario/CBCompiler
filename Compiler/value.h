#ifndef VALUE_H
#define VALUE_H
class ValueType;
#include "llvm.h"
#include <QString>
#include "constantvalue.h"
class Runtime;
class Value {
	public:
		Value();
		Value(const Value &value);
		Value(const ConstantValue &c, Runtime *r);
		Value(ValueType *t, llvm::Value *v);

		ValueType *valueType() const {return mValueType;}
		bool isConstant()const{return mConstant.isValid();}
		bool isValid() const{return mValueType != 0;}
		llvm::Value *value() const {return mValue;}
		const ConstantValue &constant() const {return mConstant;}
	private:
		ValueType *mValueType;
		llvm::Value *mValue;
		ConstantValue mConstant;
};

#endif // VALUE_H
