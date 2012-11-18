#ifndef VALUE_H
#define VALUE_H
class ValueType;
#include "llvm.h"
#include <QString>
class Value {
	public:
		Value();
		ValueType *valueType() const {return mValueType;}
	private:
		ValueType *mValueType;
		llvm::Value *mValue;
		bool mConstant;
		union {
				int mInt;
				float mFloat;
				quint16 mShort;
				quint8 mByte;
				QString *mString;
		} mConstantData;
};

#endif // VALUE_H
