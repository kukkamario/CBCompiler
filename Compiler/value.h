#ifndef VALUE_H
#define VALUE_H
class ValueType;
#include "llvm.h"
#include <QString>
class Value {
	public:
		Value();
		Value(const Value &value);
		Value(bool v);
		Value(int v);
		Value(float f);
		Value(quint16 v);
		Value(quint8 v);
		Value(ValueType *t, llvm::Value *v);

		ValueType *valueType() const {return mValueType;}
		bool isConstant()const{return mConstant;}
	private:
		ValueType *mValueType;
		llvm::Value *mValue;
		bool mConstant;
		union {
				bool mBool;
				int mInt;
				float mFloat;
				quint16 mShort;
				quint8 mByte;
				QString *mString;
		} mConstantData;
};

#endif // VALUE_H
