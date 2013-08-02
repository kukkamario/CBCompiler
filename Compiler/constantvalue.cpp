#include "constantvalue.h"
#include <math.h>

int cbPow(int a, int b) {
	double ret = pow(double(a), double(b));
	if (ret > INT_MAX || ret < INT_MIN) {
		return 0xFFFFFFFF;
	}
	return ret;
}

int imod(int a, int b) {
	return a % b;
}

ConstantValue::ConstantValue():
	mType(ValueType::Invalid)
{
}

ConstantValue::ConstantValue(ValueType::eType t) :
	mType(t) {
	switch(t) {
		case ValueType::Boolean:
			mData.mBool = false;
		case ValueType::Byte:
			mData.mByte = 0;
		case ValueType::Short:
			mData.mShort = 0;
		case ValueType::Integer:
			mData.mInt = 0;
		case ValueType::Float:
			mData.mFloat = 0;
		case ValueType::String:
			mData.mString.construct();
		default:
			return;
	}
}

ConstantValue::ConstantValue(bool t) :
	mType(ValueType::Boolean) {
	mData.mBool = t;
}

ConstantValue::ConstantValue(int i):
	mType(ValueType::Integer) {
	mData.mInt = i;
}

ConstantValue::ConstantValue(double d) :
	mType(ValueType::Float){
	mData.mFloat = d;
}

ConstantValue::ConstantValue(float f) :
	mType(ValueType::Float){
	mData.mFloat = f;
}

ConstantValue::ConstantValue(quint8 b) :
	mType(ValueType::Byte){
	mData.mByte = b;
}

ConstantValue::ConstantValue(quint16 s) :
	mType(ValueType::Short){
	mData.mShort = s;
}

ConstantValue::ConstantValue(const QString s) :
	mType(ValueType::String){
	mData.mString.construct(s);
}

ConstantValue::ConstantValue(const ConstantValue &o) :
	mType(o.mType) {
	if (mType == ValueType::String) {
		this->mData.mString.construct();
		*this->mData.mString = *o.mData.mString;
	}
	else {
		this->mData = o.mData;
	}
}

ConstantValue::~ConstantValue() {
	if (mType == ValueType::String) {
		mData.mString.destruct();
	}
}

ConstantValue &ConstantValue::operator =(const ConstantValue &v) {
	if (this->mType == ValueType::String) {
		if (v.mType == ValueType::String) {
			this->mData.mString = v.mData.mString;
			return *this;
		}

		this->mData.mString.destruct();
	}
	this->mType = v.mType;
	this->mData = v.mData;
	return *this;
}

bool ConstantValue::operator ==(const ConstantValue &o) {
	if (this->mType != o.mType) return false;
	switch( this->mType) {
		case ValueType::Boolean:
			return this->mData.mBool == o.mData.mBool;
		case ValueType::Byte:
			return this->mData.mByte == o.mData.mByte;
		case ValueType::Short:
			return this->mData.mShort == o.mData.mShort;
		case ValueType::Integer:
			return this->mData.mInt == o.mData.mInt;
		case ValueType::Float:
			return this->mData.mFloat == o.mData.mFloat;
		case ValueType::String:
			return *this->mData.mString == *o.mData.mString;
		default:
			return true;
	}
}

bool ConstantValue::operator !=(const ConstantValue &o) {
	if (this->mType != o.mType) return true;
	switch( this->mType) {
		case ValueType::Boolean:
			return this->mData.mBool != o.mData.mBool;
		case ValueType::Byte:
			return this->mData.mByte != o.mData.mByte;
		case ValueType::Short:
			return this->mData.mShort != o.mData.mShort;
		case ValueType::Integer:
			return this->mData.mInt != o.mData.mInt;
		case ValueType::Float:
			return this->mData.mFloat != o.mData.mFloat;
		case ValueType::String:
			return *this->mData.mString != *o.mData.mString;
		default:
			return true;
	}
}

ConstantValue ConstantValue::plus(const ConstantValue &a) {
	switch (a.mType) {
		case ValueType::Integer:
			return abs(a.mData.mInt);
		case ValueType::Float:
			return fabs(a.mData.mFloat);
		case ValueType::Short:
			return a.mData.mShort;
		case ValueType::Byte:
			return a.mData.mByte;
		default:
			return ConstantValue();
	}
}

ConstantValue ConstantValue::minus(const ConstantValue &a) {
	switch (a.mType) {
		case ValueType::Integer:
			return -a.mData.mInt;
		case ValueType::Float:
			return -a.mData.mFloat;
		case ValueType::Short:
			return -a.mData.mShort;
		case ValueType::Byte:
			return -a.mData.mByte;
		default:
			return ConstantValue();
	}
}

ConstantValue ConstantValue::not_(const ConstantValue &a) {
	if (a.mType == ValueType::TypePointer) {
		return ConstantValue();
	}

	return !a.toBool();
}

ConstantValue ConstantValue::equal(const ConstantValue &a, const ConstantValue &b) {
	switch (a.mType) {
		case ValueType::Integer:
			switch (b.mType) {
				case ValueType::Integer:
					return a.mData.mInt == b.mData.mInt;
				case ValueType::Float:
					return a.mData.mInt == b.mData.mFloat;
				case ValueType::Short:
					return a.mData.mInt == b.mData.mShort;
				case ValueType::Byte:
					return a.mData.mInt == b.mData.mByte;
				case ValueType::String:
					return QString::number(a.mData.mInt) == *b.mData.mString;
				default:
					return ConstantValue();
			}
		case ValueType::Float:
			switch (b.mType) {
				case ValueType::Integer:
					return a.mData.mFloat == b.mData.mInt;
				case ValueType::Float:
					return a.mData.mFloat == b.mData.mFloat;
				case ValueType::Short:
					return a.mData.mFloat == b.mData.mShort;
				case ValueType::Byte:
					return a.mData.mFloat == b.mData.mByte;
				case ValueType::String:
					return QString::number(a.mData.mFloat) == *b.mData.mString;
				default:
					return ConstantValue();
			}
		case ValueType::Short:
			switch (b.mType) {
				case ValueType::Integer:
					return a.mData.mShort == b.mData.mInt;
				case ValueType::Float:
					return a.mData.mShort == b.mData.mFloat;
				case ValueType::Short:
					return a.mData.mShort == b.mData.mShort;
				case ValueType::Byte:
					return a.mData.mShort == b.mData.mByte;
				case ValueType::String:
					return QString::number(a.mData.mShort) == *b.mData.mString;
				default:
					return ConstantValue();
			}
		case ValueType::Byte:
			switch (b.mType) {
				case ValueType::Integer:
					return a.mData.mByte == b.mData.mInt;
				case ValueType::Float:
					return a.mData.mByte == b.mData.mFloat;
				case ValueType::Short:
					return a.mData.mByte == b.mData.mShort;
				case ValueType::Byte:
					return a.mData.mByte == b.mData.mByte;
				case ValueType::String:
					return QString::number(a.mData.mByte) == *b.mData.mString;
				default:
					return ConstantValue();
			}
		case ValueType::String:
			switch (b.mType) {
				case ValueType::Integer:
					return *a.mData.mString == QString::number(b.mData.mInt);
				case ValueType::Float:
					return *a.mData.mString == QString::number(b.mData.mFloat);
				case ValueType::Short:
					return *a.mData.mString == QString::number(b.mData.mShort);
				case ValueType::Byte:
					return *a.mData.mString == QString::number(b.mData.mByte);
				case ValueType::String:
					return *a.mData.mString == *b.mData.mString;
				default:
					return ConstantValue();
			}
		case ValueType::TypePointerCommon:
			if (b.mType == ValueType::TypePointerCommon) { // NULL = NULL
				return true;
			}
		default:
			return ConstantValue();
	}
}

ConstantValue ConstantValue::notEqual(const ConstantValue &a, const ConstantValue &b) {
	switch (a.mType) {
		case ValueType::Integer:
			switch (b.mType) {
				case ValueType::Integer:
					return a.mData.mInt != b.mData.mInt;
				case ValueType::Float:
					return a.mData.mInt != b.mData.mFloat;
				case ValueType::Short:
					return a.mData.mInt != b.mData.mShort;
				case ValueType::Byte:
					return a.mData.mInt != b.mData.mByte;
				case ValueType::String:
					return QString::number(a.mData.mInt) != *b.mData.mString;
				default:
					return ConstantValue();
			}
		case ValueType::Float:
			switch (b.mType) {
				case ValueType::Integer:
					return a.mData.mFloat != b.mData.mInt;
				case ValueType::Float:
					return a.mData.mFloat != b.mData.mFloat;
				case ValueType::Short:
					return a.mData.mFloat != b.mData.mShort;
				case ValueType::Byte:
					return a.mData.mFloat != b.mData.mByte;
				case ValueType::String:
					return QString::number(a.mData.mFloat) != *b.mData.mString;
				default:
					return ConstantValue();
			}
		case ValueType::Short:
			switch (b.mType) {
				case ValueType::Integer:
					return a.mData.mShort != b.mData.mInt;
				case ValueType::Float:
					return a.mData.mShort != b.mData.mFloat;
				case ValueType::Short:
					return a.mData.mShort != b.mData.mShort;
				case ValueType::Byte:
					return a.mData.mShort != b.mData.mByte;
				case ValueType::String:
					return QString::number(a.mData.mShort) != *b.mData.mString;
				default:
					return ConstantValue();
			}
		case ValueType::Byte:
			switch (b.mType) {
				case ValueType::Integer:
					return a.mData.mByte != b.mData.mInt;
				case ValueType::Float:
					return a.mData.mByte != b.mData.mFloat;
				case ValueType::Short:
					return a.mData.mByte != b.mData.mShort;
				case ValueType::Byte:
					return a.mData.mByte != b.mData.mByte;
				case ValueType::String:
					return QString::number(a.mData.mByte) != *b.mData.mString;
				default:
					return ConstantValue();
			}
		case ValueType::String:
			switch (b.mType) {
				case ValueType::Integer:
					return *a.mData.mString != QString::number(b.mData.mInt);
				case ValueType::Float:
					return *a.mData.mString != QString::number(b.mData.mFloat);
				case ValueType::Short:
					return *a.mData.mString != QString::number(b.mData.mShort);
				case ValueType::Byte:
					return *a.mData.mString != QString::number(b.mData.mByte);
				case ValueType::String:
					return *a.mData.mString != *b.mData.mString;
				default:
					return ConstantValue();
			}
		case ValueType::TypePointerCommon:
			if (b.mType == ValueType::TypePointerCommon) {
				return false;
			}
		default:
			return ConstantValue();
	}
}

ConstantValue ConstantValue::greater(const ConstantValue &a, const ConstantValue &b) {
	switch (a.mType) {
		case ValueType::Integer:
			switch (b.mType) {
				case ValueType::Integer:
					return a.mData.mInt > b.mData.mInt;
				case ValueType::Float:
					return a.mData.mInt > b.mData.mFloat;
				case ValueType::Short:
					return a.mData.mInt > b.mData.mShort;
				case ValueType::Byte:
					return a.mData.mInt > b.mData.mByte;
				case ValueType::String:
					return QString::number(a.mData.mInt) > *b.mData.mString;
				default:
					return ConstantValue();
			}
		case ValueType::Float:
			switch (b.mType) {
				case ValueType::Integer:
					return a.mData.mFloat > b.mData.mInt;
				case ValueType::Float:
					return a.mData.mFloat > b.mData.mFloat;
				case ValueType::Short:
					return a.mData.mFloat > b.mData.mShort;
				case ValueType::Byte:
					return a.mData.mFloat > b.mData.mByte;
				case ValueType::String:
					return QString::number(a.mData.mFloat) > *b.mData.mString;
				default:
					return ConstantValue();
			}
		case ValueType::Short:
			switch (b.mType) {
				case ValueType::Integer:
					return a.mData.mShort > b.mData.mInt;
				case ValueType::Float:
					return a.mData.mShort > b.mData.mFloat;
				case ValueType::Short:
					return a.mData.mShort > b.mData.mShort;
				case ValueType::Byte:
					return a.mData.mShort > b.mData.mByte;
				case ValueType::String:
					return QString::number(a.mData.mShort) > *b.mData.mString;
				default:
					return ConstantValue();
			}
		case ValueType::Byte:
			switch (b.mType) {
				case ValueType::Integer:
					return a.mData.mByte > b.mData.mInt;
				case ValueType::Float:
					return a.mData.mByte > b.mData.mFloat;
				case ValueType::Short:
					return a.mData.mByte > b.mData.mShort;
				case ValueType::Byte:
					return a.mData.mByte > b.mData.mByte;
				case ValueType::String:
					return QString::number(a.mData.mByte) > *b.mData.mString;
				default:
					return ConstantValue();
			}
		case ValueType::String:
			switch (b.mType) {
				case ValueType::Integer:
					return *a.mData.mString > QString::number(b.mData.mInt);
				case ValueType::Float:
					return *a.mData.mString > QString::number(b.mData.mFloat);
				case ValueType::Short:
					return *a.mData.mString > QString::number(b.mData.mShort);
				case ValueType::Byte:
					return *a.mData.mString > QString::number(b.mData.mByte);
				case ValueType::String:
					return *a.mData.mString > *b.mData.mString;
				default:
					return ConstantValue();
			}
		default:
			return ConstantValue();
	}
}

ConstantValue ConstantValue::greaterEqual(const ConstantValue &a, const ConstantValue &b) {
	switch (a.mType) {
		case ValueType::Integer:
			switch (b.mType) {
				case ValueType::Integer:
					return a.mData.mInt >= b.mData.mInt;
				case ValueType::Float:
					return a.mData.mInt >= b.mData.mFloat;
				case ValueType::Short:
					return a.mData.mInt >= b.mData.mShort;
				case ValueType::Byte:
					return a.mData.mInt >= b.mData.mByte;
				case ValueType::String:
					return QString::number(a.mData.mInt) >= *b.mData.mString;
				default:
					return ConstantValue();
			}
		case ValueType::Float:
			switch (b.mType) {
				case ValueType::Integer:
					return a.mData.mFloat >= b.mData.mInt;
				case ValueType::Float:
					return a.mData.mFloat >= b.mData.mFloat;
				case ValueType::Short:
					return a.mData.mFloat >= b.mData.mShort;
				case ValueType::Byte:
					return a.mData.mFloat >= b.mData.mByte;
				case ValueType::String:
					return QString::number(a.mData.mFloat) >= *b.mData.mString;
				default:
					return ConstantValue();
			}
		case ValueType::Short:
			switch (b.mType) {
				case ValueType::Integer:
					return a.mData.mShort >= b.mData.mInt;
				case ValueType::Float:
					return a.mData.mShort >= b.mData.mFloat;
				case ValueType::Short:
					return a.mData.mShort >= b.mData.mShort;
				case ValueType::Byte:
					return a.mData.mShort >= b.mData.mByte;
				case ValueType::String:
					return QString::number(a.mData.mShort) >= *b.mData.mString;
				default:
					return ConstantValue();
			}
		case ValueType::Byte:
			switch (b.mType) {
				case ValueType::Integer:
					return a.mData.mByte >= b.mData.mInt;
				case ValueType::Float:
					return a.mData.mByte >= b.mData.mFloat;
				case ValueType::Short:
					return a.mData.mByte >= b.mData.mShort;
				case ValueType::Byte:
					return a.mData.mByte >= b.mData.mByte;
				case ValueType::String:
					return QString::number(a.mData.mByte) >= *b.mData.mString;
				default:
					return ConstantValue();
			}
		case ValueType::String:
			switch (b.mType) {
				case ValueType::Integer:
					return *a.mData.mString >= QString::number(b.mData.mInt);
				case ValueType::Float:
					return *a.mData.mString >= QString::number(b.mData.mFloat);
				case ValueType::Short:
					return *a.mData.mString >= QString::number(b.mData.mShort);
				case ValueType::Byte:
					return *a.mData.mString >= QString::number(b.mData.mByte);
				case ValueType::String:
					return *a.mData.mString >= *b.mData.mString;
				default:
					return ConstantValue();
			}
		default:
			return ConstantValue();
	}
}

ConstantValue ConstantValue::less(const ConstantValue &a, const ConstantValue &b) {
	switch (a.mType) {
		case ValueType::Integer:
			switch (b.mType) {
				case ValueType::Integer:
					return a.mData.mInt < b.mData.mInt;
				case ValueType::Float:
					return a.mData.mInt < b.mData.mFloat;
				case ValueType::Short:
					return a.mData.mInt < b.mData.mShort;
				case ValueType::Byte:
					return a.mData.mInt < b.mData.mByte;
				case ValueType::String:
					return QString::number(a.mData.mInt) < *b.mData.mString;
				default:
					return ConstantValue();
			}
		case ValueType::Float:
			switch (b.mType) {
				case ValueType::Integer:
					return a.mData.mFloat < b.mData.mInt;
				case ValueType::Float:
					return a.mData.mFloat < b.mData.mFloat;
				case ValueType::Short:
					return a.mData.mFloat < b.mData.mShort;
				case ValueType::Byte:
					return a.mData.mFloat < b.mData.mByte;
				case ValueType::String:
					return QString::number(a.mData.mFloat) < *b.mData.mString;
				default:
					return ConstantValue();
			}
		case ValueType::Short:
			switch (b.mType) {
				case ValueType::Integer:
					return a.mData.mShort < b.mData.mInt;
				case ValueType::Float:
					return a.mData.mShort < b.mData.mFloat;
				case ValueType::Short:
					return a.mData.mShort < b.mData.mShort;
				case ValueType::Byte:
					return a.mData.mShort < b.mData.mByte;
				case ValueType::String:
					return QString::number(a.mData.mShort) < *b.mData.mString;
				default:
					return ConstantValue();
			}
		case ValueType::Byte:
			switch (b.mType) {
				case ValueType::Integer:
					return a.mData.mByte < b.mData.mInt;
				case ValueType::Float:
					return a.mData.mByte < b.mData.mFloat;
				case ValueType::Short:
					return a.mData.mByte < b.mData.mShort;
				case ValueType::Byte:
					return a.mData.mByte < b.mData.mByte;
				case ValueType::String:
					return QString::number(a.mData.mByte) < *b.mData.mString;
				default:
					return ConstantValue();
			}
		case ValueType::String:
			switch (b.mType) {
				case ValueType::Integer:
					return *a.mData.mString < QString::number(b.mData.mInt);
				case ValueType::Float:
					return *a.mData.mString < QString::number(b.mData.mFloat);
				case ValueType::Short:
					return *a.mData.mString < QString::number(b.mData.mShort);
				case ValueType::Byte:
					return *a.mData.mString < QString::number(b.mData.mByte);
				case ValueType::String:
					return *a.mData.mString < *b.mData.mString;
				default:
					return ConstantValue();
			}
		default:
			return ConstantValue();
	}
}

ConstantValue ConstantValue::lessEqual(const ConstantValue &a, const ConstantValue &b) {
	switch (a.mType) {
		case ValueType::Integer:
			switch (b.mType) {
				case ValueType::Integer:
					return a.mData.mInt <= b.mData.mInt;
				case ValueType::Float:
					return a.mData.mInt <= b.mData.mFloat;
				case ValueType::Short:
					return a.mData.mInt <= b.mData.mShort;
				case ValueType::Byte:
					return a.mData.mInt <= b.mData.mByte;
				case ValueType::String:
					return QString::number(a.mData.mInt) <= *b.mData.mString;
				default:
					return ConstantValue();
			}
		case ValueType::Float:
			switch (b.mType) {
				case ValueType::Integer:
					return a.mData.mFloat <= b.mData.mInt;
				case ValueType::Float:
					return a.mData.mFloat <= b.mData.mFloat;
				case ValueType::Short:
					return a.mData.mFloat <= b.mData.mShort;
				case ValueType::Byte:
					return a.mData.mFloat <= b.mData.mByte;
				case ValueType::String:
					return QString::number(a.mData.mFloat) <= *b.mData.mString;
				default:
					return ConstantValue();
			}
		case ValueType::Short:
			switch (b.mType) {
				case ValueType::Integer:
					return a.mData.mShort <= b.mData.mInt;
				case ValueType::Float:
					return a.mData.mShort <= b.mData.mFloat;
				case ValueType::Short:
					return a.mData.mShort <= b.mData.mShort;
				case ValueType::Byte:
					return a.mData.mShort <= b.mData.mByte;
				case ValueType::String:
					return QString::number(a.mData.mShort) <= *b.mData.mString;
				default:
					return ConstantValue();
			}
		case ValueType::Byte:
			switch (b.mType) {
				case ValueType::Integer:
					return a.mData.mByte <= b.mData.mInt;
				case ValueType::Float:
					return a.mData.mByte <= b.mData.mFloat;
				case ValueType::Short:
					return a.mData.mByte <= b.mData.mShort;
				case ValueType::Byte:
					return a.mData.mByte <= b.mData.mByte;
				case ValueType::String:
					return QString::number(a.mData.mByte) <= *b.mData.mString;
				default:
					return ConstantValue();
			}
		case ValueType::String:
			switch (b.mType) {
				case ValueType::Integer:
					return *a.mData.mString <= QString::number(b.mData.mInt);
				case ValueType::Float:
					return *a.mData.mString <= QString::number(b.mData.mFloat);
				case ValueType::Short:
					return *a.mData.mString <= QString::number(b.mData.mShort);
				case ValueType::Byte:
					return *a.mData.mString <= QString::number(b.mData.mByte);
				case ValueType::String:
					return *a.mData.mString <= *b.mData.mString;
				default:
					return ConstantValue();
			}
		default:
			return ConstantValue();
	}
}

ConstantValue ConstantValue::add(const ConstantValue &a, const ConstantValue &b) {
	switch (a.mType) {
		case ValueType::Integer:
			switch (b.mType) {
				case ValueType::Integer:
					return a.mData.mInt + b.mData.mInt;
				case ValueType::Float:
					return a.mData.mInt + b.mData.mFloat;
				case ValueType::Short:
					return a.mData.mInt + b.mData.mShort;
				case ValueType::Byte:
					return a.mData.mInt + b.mData.mByte;
				case ValueType::String:
					return QString::number(a.mData.mInt) + *b.mData.mString;
				default:
					return ConstantValue();
			}
		case ValueType::Float:
			switch (b.mType) {
				case ValueType::Integer:
					return a.mData.mFloat + b.mData.mInt;
				case ValueType::Float:
					return a.mData.mFloat + b.mData.mFloat;
				case ValueType::Short:
					return a.mData.mFloat + b.mData.mShort;
				case ValueType::Byte:
					return a.mData.mFloat + b.mData.mByte;
				case ValueType::String:
					return QString::number(a.mData.mFloat) + *b.mData.mString;
				default:
					return ConstantValue();
			}
		case ValueType::Short:
			switch (b.mType) {
				case ValueType::Integer:
					return a.mData.mShort + b.mData.mInt;
				case ValueType::Float:
					return a.mData.mShort + b.mData.mFloat;
				case ValueType::Short:
					return a.mData.mShort + b.mData.mShort;
				case ValueType::Byte:
					return a.mData.mShort + b.mData.mByte;
				case ValueType::String:
					return QString::number(a.mData.mShort) + *b.mData.mString;
				default:
					return ConstantValue();
			}
		case ValueType::Byte:
			switch (b.mType) {
				case ValueType::Integer:
					return a.mData.mByte + b.mData.mInt;
				case ValueType::Float:
					return a.mData.mByte + b.mData.mFloat;
				case ValueType::Short:
					return a.mData.mByte + b.mData.mShort;
				case ValueType::Byte:
					return a.mData.mByte + b.mData.mByte;
				case ValueType::String:
					return QString::number(a.mData.mByte) + *b.mData.mString;
				default:
					return ConstantValue();
			}
		case ValueType::String:
			switch (b.mType) {
				case ValueType::Integer:
					return *a.mData.mString + QString::number(b.mData.mInt);
				case ValueType::Float:
					return *a.mData.mString + QString::number(b.mData.mFloat);
				case ValueType::Short:
					return *a.mData.mString + QString::number(b.mData.mShort);
				case ValueType::Byte:
					return *a.mData.mString + QString::number(b.mData.mByte);
				case ValueType::String:
					return *a.mData.mString + *b.mData.mString;
				default:
					return ConstantValue();
			}
		default:
			return ConstantValue();
	}
}

ConstantValue ConstantValue::subtract(const ConstantValue &a, const ConstantValue &b) {
	switch (a.mType) {
		case ValueType::Integer:
			switch (b.mType) {
				case ValueType::Integer:
					return a.mData.mInt - b.mData.mInt;
				case ValueType::Float:
					return a.mData.mInt - b.mData.mFloat;
				case ValueType::Short:
					return a.mData.mInt - b.mData.mShort;
				case ValueType::Byte:
					return a.mData.mInt - b.mData.mByte;
				default:
					return ConstantValue();
			}
		case ValueType::Float:
			switch (b.mType) {
				case ValueType::Integer:
					return a.mData.mFloat - b.mData.mInt;
				case ValueType::Float:
					return a.mData.mFloat - b.mData.mFloat;
				case ValueType::Short:
					return a.mData.mFloat - b.mData.mShort;
				case ValueType::Byte:
					return a.mData.mFloat - b.mData.mByte;
				default:
					return ConstantValue();
			}
		case ValueType::Short:
			switch (b.mType) {
				case ValueType::Integer:
					return a.mData.mShort - b.mData.mInt;
				case ValueType::Float:
					return a.mData.mShort - b.mData.mFloat;
				case ValueType::Short:
					return a.mData.mShort - b.mData.mShort;
				case ValueType::Byte:
					return a.mData.mShort - b.mData.mByte;
				default:
					return ConstantValue();
			}
		case ValueType::Byte:
			switch (b.mType) {
				case ValueType::Integer:
					return a.mData.mByte - b.mData.mInt;
				case ValueType::Float:
					return a.mData.mByte - b.mData.mFloat;
				case ValueType::Short:
					return a.mData.mByte - b.mData.mShort;
				case ValueType::Byte:
					return a.mData.mByte - b.mData.mByte;
				default:
					return ConstantValue();
			}
		default:
			return ConstantValue();
	}
}

ConstantValue ConstantValue::multiply(const ConstantValue &a, const ConstantValue &b) {
	switch (a.mType) {
		case ValueType::Integer:
			switch (b.mType) {
				case ValueType::Integer:
					return a.mData.mInt * b.mData.mInt;
				case ValueType::Float:
					return a.mData.mInt * b.mData.mFloat;
				case ValueType::Short:
					return a.mData.mInt * b.mData.mShort;
				case ValueType::Byte:
					return a.mData.mInt * b.mData.mByte;
				default:
					return ConstantValue();
			}
		case ValueType::Float:
			switch (b.mType) {
				case ValueType::Integer:
					return a.mData.mFloat * b.mData.mInt;
				case ValueType::Float:
					return a.mData.mFloat * b.mData.mFloat;
				case ValueType::Short:
					return a.mData.mFloat * b.mData.mShort;
				case ValueType::Byte:
					return a.mData.mFloat * b.mData.mByte;
				default:
					return ConstantValue();
			}
		case ValueType::Short:
			switch (b.mType) {
				case ValueType::Integer:
					return a.mData.mShort * b.mData.mInt;
				case ValueType::Float:
					return a.mData.mShort * b.mData.mFloat;
				case ValueType::Short:
					return a.mData.mShort * b.mData.mShort;
				case ValueType::Byte:
					return a.mData.mShort * b.mData.mByte;
				default:
					return ConstantValue();
			}
		case ValueType::Byte:
			switch (b.mType) {
				case ValueType::Integer:
					return a.mData.mByte * b.mData.mInt;
				case ValueType::Float:
					return a.mData.mByte * b.mData.mFloat;
				case ValueType::Short:
					return a.mData.mByte * b.mData.mShort;
				case ValueType::Byte:
					return a.mData.mByte * b.mData.mByte;
				default:
					return ConstantValue();
			}
		default:
			return ConstantValue();
	}
}

ConstantValue ConstantValue::divide(const ConstantValue &a, const ConstantValue &b) {
	switch (a.mType) {
		case ValueType::Integer:
			switch (b.mType) {
				case ValueType::Integer:
					assert(b.mData.mInt != 0 && "FIXME: Integer divided by Zero");
					return a.mData.mInt / b.mData.mInt;
				case ValueType::Float:
					return a.mData.mInt / b.mData.mFloat;
				case ValueType::Short:
					assert(b.mData.mShort != 0 && "FIXME: Integer divided by Zero");
					return a.mData.mInt / b.mData.mShort;
				case ValueType::Byte:
					assert(b.mData.mByte != 0 && "FIXME: Integer divided by Zero");
					return a.mData.mInt / b.mData.mByte;
				default:
					return ConstantValue();
			}
		case ValueType::Float:
			//TODO: Warnings if result is NaN or INF
			switch (b.mType) {
				case ValueType::Integer:
					return a.mData.mFloat / b.mData.mInt;
				case ValueType::Float:
					return a.mData.mFloat / b.mData.mFloat;
				case ValueType::Short:
					return a.mData.mFloat / b.mData.mShort;
				case ValueType::Byte:
					return a.mData.mFloat / b.mData.mByte;
				default:
					return ConstantValue();
			}
		case ValueType::Short:
			switch (b.mType) {
				case ValueType::Integer:
					assert(b.mData.mInt != 0 && "FIXME: Integer divided by Zero");
					return a.mData.mShort / b.mData.mInt;
				case ValueType::Float:
					return a.mData.mShort / b.mData.mFloat;
				case ValueType::Short:
					assert(b.mData.mShort != 0 && "FIXME: Integer divided by Zero");
					return a.mData.mShort / b.mData.mShort;
				case ValueType::Byte:
					assert(b.mData.mByte != 0 && "FIXME: Integer divided by Zero");
					return a.mData.mShort / b.mData.mByte;
				default:
					return ConstantValue();
			}
		case ValueType::Byte:
			switch (b.mType) {
				case ValueType::Integer:
					assert(b.mData.mInt != 0 && "FIXME: Integer divided by Zero");
					return a.mData.mByte / b.mData.mInt;
				case ValueType::Float:
					return a.mData.mByte / b.mData.mFloat;
				case ValueType::Short:
					assert(b.mData.mShort != 0 && "FIXME: Integer divided by Zero");
					return a.mData.mByte / b.mData.mShort;
				case ValueType::Byte:
					assert(b.mData.mByte != 0 && "FIXME: Integer divided by Zero");
					return a.mData.mByte / b.mData.mByte;
				default:
					return ConstantValue();
			}
		default:
			return ConstantValue();
	}
}

ConstantValue ConstantValue::power(const ConstantValue &a, const ConstantValue &b) {
	switch (a.mType) {
		case ValueType::Integer:
			switch (b.mType) {
				case ValueType::Integer:
					return cbPow(a.mData.mInt, b.mData.mInt);
				case ValueType::Float:
					return pow(a.mData.mInt, b.mData.mFloat);
				case ValueType::Short:
					return cbPow(a.mData.mInt, b.mData.mShort);
				case ValueType::Byte:
					return cbPow(a.mData.mInt, b.mData.mByte);
				default:
					return ConstantValue();
			}
		case ValueType::Float:
			switch (b.mType) {
				case ValueType::Integer:
					return pow(a.mData.mFloat, b.mData.mInt);
				case ValueType::Float:
					return pow(a.mData.mFloat, b.mData.mFloat);
				case ValueType::Short:
					return pow(a.mData.mFloat, b.mData.mShort);
				case ValueType::Byte:
					return pow(a.mData.mFloat, b.mData.mByte);
				default:
					return ConstantValue();
			}
		case ValueType::Short:
			switch (b.mType) {
				case ValueType::Integer:
					return cbPow(a.mData.mShort, b.mData.mInt);
				case ValueType::Float:
					return pow(a.mData.mShort, b.mData.mFloat);
				case ValueType::Short:
					return cbPow(a.mData.mShort, b.mData.mShort);
				case ValueType::Byte:
					return cbPow(a.mData.mShort, b.mData.mByte);
				default:
					return ConstantValue();
			}
		case ValueType::Byte:
			switch (b.mType) {
				case ValueType::Integer:
					return cbPow(a.mData.mByte, b.mData.mInt);
				case ValueType::Float:
					return pow(a.mData.mByte, b.mData.mFloat);
				case ValueType::Short:
					return cbPow(a.mData.mByte, b.mData.mShort);
				case ValueType::Byte:
					return cbPow(a.mData.mByte, b.mData.mByte);
				default:
					return ConstantValue();
			}
		default:
			return ConstantValue();
	}
}

ConstantValue ConstantValue::mod(const ConstantValue &a, const ConstantValue &b) {
	switch (a.mType) {
		case ValueType::Integer:
			switch (b.mType) {
				case ValueType::Integer:
					return imod(a.mData.mInt, b.mData.mInt);
				case ValueType::Float:
					return fmod(a.mData.mInt, b.mData.mFloat);
				case ValueType::Short:
					return imod(a.mData.mInt, b.mData.mShort);
				case ValueType::Byte:
					return imod(a.mData.mInt, b.mData.mByte);
				default:
					return ConstantValue();
			}
		case ValueType::Float:
			switch (b.mType) {
				case ValueType::Integer:
					return fmod(a.mData.mFloat, b.mData.mInt);
				case ValueType::Float:
					return fmod(a.mData.mFloat, b.mData.mFloat);
				case ValueType::Short:
					return fmod(a.mData.mFloat, b.mData.mShort);
				case ValueType::Byte:
					return fmod(a.mData.mFloat, b.mData.mByte);
				default:
					return ConstantValue();
			}
		case ValueType::Short:
			switch (b.mType) {
				case ValueType::Integer:
					return mod(a.mData.mShort, b.mData.mInt);
				case ValueType::Float:
					return fmod(a.mData.mShort, b.mData.mFloat);
				case ValueType::Short:
					return imod(a.mData.mShort, b.mData.mShort);
				case ValueType::Byte:
					return imod(a.mData.mShort, b.mData.mByte);
				default:
					return ConstantValue();
			}
		case ValueType::Byte:
			switch (b.mType) {
				case ValueType::Integer:
					return mod(a.mData.mByte, b.mData.mInt);
				case ValueType::Float:
					return fmod(a.mData.mByte, b.mData.mFloat);
				case ValueType::Short:
					return imod(a.mData.mByte, b.mData.mShort);
				case ValueType::Byte:
					return imod(a.mData.mByte, b.mData.mByte);
				default:
					return ConstantValue();
			}
		default:
			return ConstantValue();
	}
}

ConstantValue ConstantValue::shr(const ConstantValue &a, const ConstantValue &b) {
	switch (a.mType) {
		case ValueType::Integer:
			switch (b.mType) {
				case ValueType::Integer:
					return  int((unsigned int)a.mData.mInt >> (unsigned int)b.mData.mInt);
				case ValueType::Short:
					return int((unsigned int)a.mData.mInt >> b.mData.mShort);
				case ValueType::Byte:
					return int((unsigned int)a.mData.mInt >> b.mData.mByte);
				default:
					return ConstantValue();
			}
		case ValueType::Short:
			switch (b.mType) {
				case ValueType::Integer:
					return int(a.mData.mShort >> (unsigned int)b.mData.mInt);
				case ValueType::Short:
					return a.mData.mShort >> b.mData.mShort;
				case ValueType::Byte:
					return a.mData.mShort >> b.mData.mByte;
				default:
					return ConstantValue();
			}
		case ValueType::Byte:
			switch (b.mType) {
				case ValueType::Integer:
					return int(a.mData.mByte >> (unsigned int)b.mData.mInt);
				case ValueType::Short:
					return a.mData.mByte >> b.mData.mShort;
				case ValueType::Byte:
					return a.mData.mByte >> b.mData.mByte;
				default:
					return ConstantValue();
			}
		default:
			return ConstantValue();
	}
}

ConstantValue ConstantValue::shl(const ConstantValue &a, const ConstantValue &b) {
	switch (a.mType) {
		case ValueType::Integer:
			switch (b.mType) {
				case ValueType::Integer:
					return  int((unsigned int)a.mData.mInt << (unsigned int)b.mData.mInt);
				case ValueType::Short:
					return int((unsigned int)a.mData.mInt << b.mData.mShort);
				case ValueType::Byte:
					return int((unsigned int)a.mData.mInt << b.mData.mByte);
				default:
					return ConstantValue();
			}
		case ValueType::Short:
			switch (b.mType) {
				case ValueType::Integer:
					return int(a.mData.mShort << (unsigned int)b.mData.mInt);
				case ValueType::Short:
					return a.mData.mShort << b.mData.mShort;
				case ValueType::Byte:
					return a.mData.mShort << b.mData.mByte;
				default:
					return ConstantValue();
			}
		case ValueType::Byte:
			switch (b.mType) {
				case ValueType::Integer:
					return int(a.mData.mByte << (unsigned int)b.mData.mInt);
				case ValueType::Short:
					return a.mData.mByte << b.mData.mShort;
				case ValueType::Byte:
					return a.mData.mByte << b.mData.mByte;
				default:
					return ConstantValue();
			}
		default:
			return ConstantValue();
	}
}

ConstantValue ConstantValue::sar(const ConstantValue &a, const ConstantValue &b) {
	switch (a.mType) {
		case ValueType::Integer:
			switch (b.mType) {
				case ValueType::Integer:
					return  a.mData.mInt >> b.mData.mInt;
				case ValueType::Short:
					return a.mData.mInt >> b.mData.mShort;
				case ValueType::Byte:
					return a.mData.mInt >> b.mData.mByte;
				default:
					return ConstantValue();
			}
		case ValueType::Short:
			switch (b.mType) {
				case ValueType::Integer:
					return a.mData.mShort >> b.mData.mInt;
				case ValueType::Short:
					return a.mData.mShort >> b.mData.mShort;
				case ValueType::Byte:
					return a.mData.mShort >> b.mData.mByte;
				default:
					return ConstantValue();
			}
		case ValueType::Byte:
			switch (b.mType) {
				case ValueType::Integer:
					return a.mData.mByte >> b.mData.mInt;
				case ValueType::Short:
					return a.mData.mByte >> b.mData.mShort;
				case ValueType::Byte:
					return a.mData.mByte >> b.mData.mByte;
				default:
					return ConstantValue();
			}
		default:
			return ConstantValue();
	}
}

ConstantValue ConstantValue::and_(const ConstantValue &a, const ConstantValue &b) {
	if (a.mType == ValueType::TypePointerCommon || b.mType == ValueType::TypePointerCommon) return ConstantValue();
	return a.toBool() && b.toBool();
}

ConstantValue ConstantValue::or_(const ConstantValue &a, const ConstantValue &b) {
	if (a.mType == ValueType::TypePointerCommon || b.mType == ValueType::TypePointerCommon) return ConstantValue();
	return a.toBool() || b.toBool();
}

ConstantValue ConstantValue::xor_(const ConstantValue &a, const ConstantValue &b){
	if (a.mType == ValueType::TypePointerCommon || b.mType == ValueType::TypePointerCommon) return ConstantValue();
	return a.toBool() ^ b.toBool();
}

int ConstantValue::cbIntPower(int a, int b) {
	return cbPow(a, b);
}

QString ConstantValue::toString() const{
	switch(this->mType) {
		case ValueType::Boolean:
			return mData.mBool ? "1" : "0";
		case ValueType::Byte:
			return QString::number(mData.mByte);
		case ValueType::Short:
			return QString::number(mData.mShort);
		case ValueType::Integer:
			return QString::number(mData.mInt);
		case ValueType::Float:
			return QString::number(mData.mFloat);
		case ValueType::String:
			return *mData.mString;
		default:
			return QString();
	}
}

quint16 ConstantValue::toShort() const{
	switch(this->mType) {
		case ValueType::Boolean:
			return mData.mBool;
		case ValueType::Byte:
			return mData.mByte;
		case ValueType::Short:
			return mData.mShort;
		case ValueType::Integer:
			return mData.mInt;
		case ValueType::Float:
			return mData.mFloat;
		case ValueType::String:
			return mData.mString->toUShort();
		default:
			return 0;
	}
}

quint8 ConstantValue::toByte() const {
	switch(this->mType) {
		case ValueType::Boolean:
			return mData.mBool;
		case ValueType::Byte:
			return mData.mByte;
		case ValueType::Short:
			return mData.mShort;
		case ValueType::Integer:
			return mData.mInt;
		case ValueType::Float:
			return mData.mFloat;
		case ValueType::String:
			return (quint8)mData.mString->toUShort();
		default:
			return 0;
	}
}

float ConstantValue::toFloat() const{
	switch(this->mType) {
		case ValueType::Boolean:
			return mData.mBool;
		case ValueType::Byte:
			return mData.mByte;
		case ValueType::Short:
			return mData.mShort;
		case ValueType::Integer:
			return mData.mInt;
		case ValueType::Float:
			return mData.mFloat;
		case ValueType::String:
			return mData.mString->toFloat();
		default:
			return 0;
	}
}

int ConstantValue::toInt() const{
	switch(this->mType) {
		case ValueType::Boolean:
			return mData.mBool;
		case ValueType::Byte:
			return mData.mByte;
		case ValueType::Short:
			return mData.mShort;
		case ValueType::Integer:
			return mData.mInt;
		case ValueType::Float:
			return mData.mFloat;
		case ValueType::String:
			return mData.mString->toInt();
		default:
			return 0;
	}
}

bool ConstantValue::toBool() const{
	switch(this->mType) {
		case ValueType::Boolean:
			return mData.mBool;
		case ValueType::Byte:
			return mData.mByte != 0;
		case ValueType::Short:
			return mData.mShort != 0;
		case ValueType::Integer:
			return mData.mInt != 0;
		case ValueType::Float:
			return bool(mData.mFloat);
		case ValueType::String:
			return !mData.mString->isEmpty();
		default:
			return false;
	}
}

QString ConstantValue::typeName() const {
	switch(mType) {
		case ValueType::Integer:
			return "Integer";
		case ValueType::Float:
			return "Float";
		case ValueType::String:
			return "String";
		case ValueType::Byte:
			return "Byte";
		case ValueType::Short:
			return "Short";
		case ValueType::TypePointerCommon:
			return "NULL";
		default:
			return "Invalid";
	}
}

QString ConstantValue::valueInfo() const {
	switch(this->mType) {
		case ValueType::Boolean:
			return mData.mBool ? "true" : "false";
		case ValueType::Byte:
			return QString::number(mData.mByte);
		case ValueType::Short:
			return QString::number(mData.mShort);
		case ValueType::Integer:
			return QString::number(mData.mInt);
		case ValueType::Float:
			return QString::number(mData.mFloat);
		case ValueType::String:
			return "\"" + *mData.mString  + "\"";
		case ValueType::TypePointerCommon:
			return "NULL";
		default:
			return QString("Invalid constant value");
	}
}
