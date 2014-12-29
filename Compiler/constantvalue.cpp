#include "constantvalue.h"
#include "valuetype.h"
#include <math.h>
#include <boost/lexical_cast.hpp>

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
	mType(Invalid)
{
}


ConstantValue::ConstantValue(bool t) :
	mType(Boolean) {
	mData.mBool = t;
}

ConstantValue::ConstantValue(int i):
	mType(Integer) {
	mData.mInt = i;
}

ConstantValue::ConstantValue(double d) :
	mType(Float){
	mData.mFloat = d;
}

ConstantValue::ConstantValue(float f) :
	mType(Float){
	mData.mFloat = f;
}

ConstantValue::ConstantValue(uint8_t b) :
	mType(Byte){
	mData.mByte = b;
}

ConstantValue::ConstantValue(uint16_t s) :
	mType(Short){
	mData.mShort = s;
}

ConstantValue::ConstantValue(const std::string s) :
	mType(String){
	mData.mString = s;
}

ConstantValue::ConstantValue(const ConstantValue &o) :
	mType(o.mType) {
	this->mData = o.mData;
}

ConstantValue::ConstantValue(ConstantValue::Type type) :
	mType(type) {
	switch (type) {
		case Byte:
			mData.mByte = 0; break;
		case Short:
			mData.mShort = 0; break;
		case Integer:
			mData.mInt = 0; break;
		case Float:
			mData.mFloat = 0; break;
		case Boolean:
			mData.mBool = 0; break;
		case String:
			mData.mString = std::string(); break;
		default:
			mData.mInt = 0; break;
	}

}

ConstantValue::~ConstantValue() {
}

ConstantValue &ConstantValue::operator =(const ConstantValue &v) {
	this->mType = v.mType;
	this->mData = v.mData;
	return *this;
}

bool ConstantValue::operator ==(const ConstantValue &o) {
	if (this->mType != o.mType) return false;
	switch( this->mType) {
		case Boolean:
			return this->mData.mBool == o.mData.mBool;
		case Byte:
			return this->mData.mByte == o.mData.mByte;
		case Short:
			return this->mData.mShort == o.mData.mShort;
		case Integer:
			return this->mData.mInt == o.mData.mInt;
		case Float:
			return this->mData.mFloat == o.mData.mFloat;
		case String:
			return this->mData.mString == o.mData.mString;
		default:
			return true;
	}
}

bool ConstantValue::operator !=(const ConstantValue &o) {
	if (this->mType != o.mType) return true;
	switch( this->mType) {
		case Boolean:
			return this->mData.mBool != o.mData.mBool;
		case Byte:
			return this->mData.mByte != o.mData.mByte;
		case Short:
			return this->mData.mShort != o.mData.mShort;
		case Integer:
			return this->mData.mInt != o.mData.mInt;
		case Float:
			return this->mData.mFloat != o.mData.mFloat;
		case String:
			return this->mData.mString != o.mData.mString;
		default:
			return true;
	}
}

ConstantValue ConstantValue::plus(const ConstantValue &a, OperationFlags &flags) {
	switch (a.mType) {
		case Integer:
			return abs(a.mData.mInt);
		case Float:
			return fabs(a.mData.mFloat);
		case Short:
			return a.mData.mShort;
		case Byte:
			return a.mData.mByte;
		default:
			flags |= OperationFlag::NoSuchOperation;
			return ConstantValue();
	}
}

ConstantValue ConstantValue::minus(const ConstantValue &a, OperationFlags &flags) {
	switch (a.mType) {
		case Integer:
			return -a.mData.mInt;
		case Float:
			return -a.mData.mFloat;
		case Short:
			return -a.mData.mShort;
		case Byte:
			return -a.mData.mByte;
		default:
			flags |= OperationFlag::NoSuchOperation;
			return ConstantValue();
	}
}

ConstantValue ConstantValue::not_(const ConstantValue &a, OperationFlags &flags) {
	if (a.mType == Null) {
		flags |= OperationFlag::NoSuchOperation;
		return ConstantValue();
	}

	return !a.toBool();
}

ConstantValue ConstantValue::equal(const ConstantValue &a, const ConstantValue &b, OperationFlags &flags) {
	if ((a.mType == String) ^ (b.mType == String) ) {
		flags |= OperationFlag::CastToString;
	}
	switch (a.mType) {
		case Integer:
			switch (b.mType) {
				case Integer:
					return a.mData.mInt == b.mData.mInt;
				case Float:
					return a.mData.mInt == b.mData.mFloat;
				case Short:
					return a.mData.mInt == b.mData.mShort;
				case Byte:
					return a.mData.mInt == b.mData.mByte;
				case String:
					return boost::lexical_cast<std::string>(a.mData.mInt) == b.mData.mString;
				default:
					break;
			}
			break;
		case Float:
			switch (b.mType) {
				case Integer:
					return a.mData.mFloat == b.mData.mInt;
				case Float:
					return a.mData.mFloat == b.mData.mFloat;
				case Short:
					return a.mData.mFloat == b.mData.mShort;
				case Byte:
					return a.mData.mFloat == b.mData.mByte;
				case String:
					return boost::lexical_cast<std::string>(a.mData.mFloat) == b.mData.mString;
				default:
					break;
			}
			break;
		case Short:
			switch (b.mType) {
				case Integer:
					return a.mData.mShort == b.mData.mInt;
				case Float:
					return a.mData.mShort == b.mData.mFloat;
				case Short:
					return a.mData.mShort == b.mData.mShort;
				case Byte:
					return a.mData.mShort == b.mData.mByte;
				case String:
					return boost::lexical_cast<std::string>(a.mData.mShort) == b.mData.mString;
				default:
					break;
			}
			break;
		case Byte:
			switch (b.mType) {
				case Integer:
					return a.mData.mByte == b.mData.mInt;
				case Float:
					return a.mData.mByte == b.mData.mFloat;
				case Short:
					return a.mData.mByte == b.mData.mShort;
				case Byte:
					return a.mData.mByte == b.mData.mByte;
				case String:
					return boost::lexical_cast<std::string>(a.mData.mByte) == b.mData.mString;
				default:
					break;
			}
			break;
		case String:
			switch (b.mType) {
				case Integer:
					return a.mData.mString == boost::lexical_cast<std::string>(b.mData.mInt);
				case Float:
					return a.mData.mString == boost::lexical_cast<std::string>(b.mData.mFloat);
				case Short:
					return a.mData.mString == boost::lexical_cast<std::string>(b.mData.mShort);
				case Byte:
					return a.mData.mString == boost::lexical_cast<std::string>(b.mData.mByte);
				case String:
					return a.mData.mString == b.mData.mString;
				default:
					break;
			}
		case Null:
			if (b.mType == Null) { // NULL = NULL
				return true;
			}
		default:
			break;
	}
	flags |= OperationFlag::NoSuchOperation;
	return ConstantValue();
}

ConstantValue ConstantValue::notEqual(const ConstantValue &a, const ConstantValue &b, OperationFlags &flags) {
	if ((a.mType == String) ^ (b.mType == String) ) {
		flags |= OperationFlag::CastToString;
	}
	switch (a.mType) {
		case Integer:
			switch (b.mType) {
				case Integer:
					return a.mData.mInt != b.mData.mInt;
				case Float:
					return a.mData.mInt != b.mData.mFloat;
				case Short:
					return a.mData.mInt != b.mData.mShort;
				case Byte:
					return a.mData.mInt != b.mData.mByte;
				case String:
					return boost::lexical_cast<std::string>(a.mData.mInt) != b.mData.mString;
				default:
					break;
			}
			break;
		case Float:
			switch (b.mType) {
				case Integer:
					return a.mData.mFloat != b.mData.mInt;
				case Float:
					return a.mData.mFloat != b.mData.mFloat;
				case Short:
					return a.mData.mFloat != b.mData.mShort;
				case Byte:
					return a.mData.mFloat != b.mData.mByte;
				case String:
					return boost::lexical_cast<std::string>(a.mData.mFloat) != b.mData.mString;
				default:
					break;
			}
			break;
		case Short:
			switch (b.mType) {
				case Integer:
					return a.mData.mShort != b.mData.mInt;
				case Float:
					return a.mData.mShort != b.mData.mFloat;
				case Short:
					return a.mData.mShort != b.mData.mShort;
				case Byte:
					return a.mData.mShort != b.mData.mByte;
				case String:
					return boost::lexical_cast<std::string>(a.mData.mShort) != b.mData.mString;
				default:
					break;
			}
			break;
		case Byte:
			switch (b.mType) {
				case Integer:
					return a.mData.mByte != b.mData.mInt;
				case Float:
					return a.mData.mByte != b.mData.mFloat;
				case Short:
					return a.mData.mByte != b.mData.mShort;
				case Byte:
					return a.mData.mByte != b.mData.mByte;
				case String:
					return boost::lexical_cast<std::string>(a.mData.mByte) != b.mData.mString;
				default:
					break;
			}
			break;
		case String:
			switch (b.mType) {
				case Integer:
					return a.mData.mString != boost::lexical_cast<std::string>(b.mData.mInt);
				case Float:
					return a.mData.mString != boost::lexical_cast<std::string>(b.mData.mFloat);
				case Short:
					return a.mData.mString != boost::lexical_cast<std::string>(b.mData.mShort);
				case Byte:
					return a.mData.mString != boost::lexical_cast<std::string>(b.mData.mByte);
				case String:
					return a.mData.mString != b.mData.mString;
				default:
					break;
			}
			break;
		case Null:
			if (b.mType == Null) {
				return false;
			}
		default:
			break;
	}
	flags |= OperationFlag::NoSuchOperation;
	return ConstantValue();
}

ConstantValue ConstantValue::greater(const ConstantValue &a, const ConstantValue &b, OperationFlags &flags) {
	if ((a.mType == String) ^ (b.mType == String) ) {
		flags |= OperationFlag::CastToString;
	}
	switch (a.mType) {
		case Integer:
			switch (b.mType) {
				case Integer:
					return a.mData.mInt > b.mData.mInt;
				case Float:
					return a.mData.mInt > b.mData.mFloat;
				case Short:
					return a.mData.mInt > b.mData.mShort;
				case Byte:
					return a.mData.mInt > b.mData.mByte;
				case String:
					return boost::lexical_cast<std::string>(a.mData.mInt) > b.mData.mString;
				default:
					break;
			}
			break;
		case Float:
			switch (b.mType) {
				case Integer:
					return a.mData.mFloat > b.mData.mInt;
				case Float:
					return a.mData.mFloat > b.mData.mFloat;
				case Short:
					return a.mData.mFloat > b.mData.mShort;
				case Byte:
					return a.mData.mFloat > b.mData.mByte;
				case String:
					return boost::lexical_cast<std::string>(a.mData.mFloat) > b.mData.mString;
				default:
					break;
			}
			break;
		case Short:
			switch (b.mType) {
				case Integer:
					return a.mData.mShort > b.mData.mInt;
				case Float:
					return a.mData.mShort > b.mData.mFloat;
				case Short:
					return a.mData.mShort > b.mData.mShort;
				case Byte:
					return a.mData.mShort > b.mData.mByte;
				case String:
					return boost::lexical_cast<std::string>(a.mData.mShort) > b.mData.mString;
				default:
					break;
			}
			break;
		case Byte:
			switch (b.mType) {
				case Integer:
					return a.mData.mByte > b.mData.mInt;
				case Float:
					return a.mData.mByte > b.mData.mFloat;
				case Short:
					return a.mData.mByte > b.mData.mShort;
				case Byte:
					return a.mData.mByte > b.mData.mByte;
				case String:
					return boost::lexical_cast<std::string>(a.mData.mByte) > b.mData.mString;
				default:
					break;
			}
			break;
		case String:
			switch (b.mType) {
				case Integer:
					return a.mData.mString > boost::lexical_cast<std::string>(b.mData.mInt);
				case Float:
					return a.mData.mString > boost::lexical_cast<std::string>(b.mData.mFloat);
				case Short:
					return a.mData.mString > boost::lexical_cast<std::string>(b.mData.mShort);
				case Byte:
					return a.mData.mString > boost::lexical_cast<std::string>(b.mData.mByte);
				case String:
					return a.mData.mString > b.mData.mString;
				default:
					break;
			}
			break;
		default:
			break;
	}
	flags |= OperationFlag::NoSuchOperation;
	return ConstantValue();
}

ConstantValue ConstantValue::greaterEqual(const ConstantValue &a, const ConstantValue &b, OperationFlags &flags) {
	if ((a.mType == String) ^ (b.mType == String) ) {
		flags |= OperationFlag::CastToString;
	}
	switch (a.mType) {
		case Integer:
			switch (b.mType) {
				case Integer:
					return a.mData.mInt >= b.mData.mInt;
				case Float:
					return a.mData.mInt >= b.mData.mFloat;
				case Short:
					return a.mData.mInt >= b.mData.mShort;
				case Byte:
					return a.mData.mInt >= b.mData.mByte;
				case String:
					return boost::lexical_cast<std::string>(a.mData.mInt) >= b.mData.mString;
				default:
					break;
			}
			break;
		case Float:
			switch (b.mType) {
				case Integer:
					return a.mData.mFloat >= b.mData.mInt;
				case Float:
					return a.mData.mFloat >= b.mData.mFloat;
				case Short:
					return a.mData.mFloat >= b.mData.mShort;
				case Byte:
					return a.mData.mFloat >= b.mData.mByte;
				case String:
					return boost::lexical_cast<std::string>(a.mData.mFloat) >= b.mData.mString;
				default:
					break;
			}
			break;
		case Short:
			switch (b.mType) {
				case Integer:
					return a.mData.mShort >= b.mData.mInt;
				case Float:
					return a.mData.mShort >= b.mData.mFloat;
				case Short:
					return a.mData.mShort >= b.mData.mShort;
				case Byte:
					return a.mData.mShort >= b.mData.mByte;
				case String:
					return boost::lexical_cast<std::string>(a.mData.mShort) >= b.mData.mString;
				default:
					break;
			}
			break;
		case Byte:
			switch (b.mType) {
				case Integer:
					return a.mData.mByte >= b.mData.mInt;
				case Float:
					return a.mData.mByte >= b.mData.mFloat;
				case Short:
					return a.mData.mByte >= b.mData.mShort;
				case Byte:
					return a.mData.mByte >= b.mData.mByte;
				case String:
					return boost::lexical_cast<std::string>(a.mData.mByte) >= b.mData.mString;
				default:
					break;
			}
			break;
		case String:
			switch (b.mType) {
				case Integer:
					return a.mData.mString >= boost::lexical_cast<std::string>(b.mData.mInt);
				case Float:
					return a.mData.mString >= boost::lexical_cast<std::string>(b.mData.mFloat);
				case Short:
					return a.mData.mString >= boost::lexical_cast<std::string>(b.mData.mShort);
				case Byte:
					return a.mData.mString >= boost::lexical_cast<std::string>(b.mData.mByte);
				case String:
					return a.mData.mString >= b.mData.mString;
				default:
					break;
			}
			break;
		default:
			break;
	}
	flags |= OperationFlag::NoSuchOperation;
	return ConstantValue();
}

ConstantValue ConstantValue::less(const ConstantValue &a, const ConstantValue &b, OperationFlags &flags) {
	if ((a.mType == String) ^ (b.mType == String) ) {
		flags |= OperationFlag::CastToString;
	}
	switch (a.mType) {
		case Integer:
			switch (b.mType) {
				case Integer:
					return a.mData.mInt < b.mData.mInt;
				case Float:
					return a.mData.mInt < b.mData.mFloat;
				case Short:
					return a.mData.mInt < b.mData.mShort;
				case Byte:
					return a.mData.mInt < b.mData.mByte;
				case String:
					return boost::lexical_cast<std::string>(a.mData.mInt) < b.mData.mString;
				default:
					break;
			}
			break;
		case Float:
			switch (b.mType) {
				case Integer:
					return a.mData.mFloat < b.mData.mInt;
				case Float:
					return a.mData.mFloat < b.mData.mFloat;
				case Short:
					return a.mData.mFloat < b.mData.mShort;
				case Byte:
					return a.mData.mFloat < b.mData.mByte;
				case String:
					return boost::lexical_cast<std::string>(a.mData.mFloat) < b.mData.mString;
				default:
					break;
			}
			break;
		case Short:
			switch (b.mType) {
				case Integer:
					return a.mData.mShort < b.mData.mInt;
				case Float:
					return a.mData.mShort < b.mData.mFloat;
				case Short:
					return a.mData.mShort < b.mData.mShort;
				case Byte:
					return a.mData.mShort < b.mData.mByte;
				case String:
					return boost::lexical_cast<std::string>(a.mData.mShort) < b.mData.mString;
				default:
					break;
			}
			break;
		case Byte:
			switch (b.mType) {
				case Integer:
					return a.mData.mByte < b.mData.mInt;
				case Float:
					return a.mData.mByte < b.mData.mFloat;
				case Short:
					return a.mData.mByte < b.mData.mShort;
				case Byte:
					return a.mData.mByte < b.mData.mByte;
				case String:
					return boost::lexical_cast<std::string>(a.mData.mByte) < b.mData.mString;
				default:
					break;
			}
			break;
		case String:
			switch (b.mType) {
				case Integer:
					return a.mData.mString < boost::lexical_cast<std::string>(b.mData.mInt);
				case Float:
					return a.mData.mString < boost::lexical_cast<std::string>(b.mData.mFloat);
				case Short:
					return a.mData.mString < boost::lexical_cast<std::string>(b.mData.mShort);
				case Byte:
					return a.mData.mString < boost::lexical_cast<std::string>(b.mData.mByte);
				case String:
					return a.mData.mString < b.mData.mString;
				default:
					break;
			}
			break;
		default:
			break;
	}
	flags |= OperationFlag::NoSuchOperation;
	return ConstantValue();
}

ConstantValue ConstantValue::lessEqual(const ConstantValue &a, const ConstantValue &b, OperationFlags &flags) {
	if ((a.mType == String) ^ (b.mType == String) ) {
		flags |= OperationFlag::CastToString;
	}
	switch (a.mType) {
		case Integer:
			switch (b.mType) {
				case Integer:
					return a.mData.mInt <= b.mData.mInt;
				case Float:
					return a.mData.mInt <= b.mData.mFloat;
				case Short:
					return a.mData.mInt <= b.mData.mShort;
				case Byte:
					return a.mData.mInt <= b.mData.mByte;
				case String:
					return boost::lexical_cast<std::string>(a.mData.mInt) <= b.mData.mString;
				default:
					break;
			}
			break;
		case Float:
			switch (b.mType) {
				case Integer:
					return a.mData.mFloat <= b.mData.mInt;
				case Float:
					return a.mData.mFloat <= b.mData.mFloat;
				case Short:
					return a.mData.mFloat <= b.mData.mShort;
				case Byte:
					return a.mData.mFloat <= b.mData.mByte;
				case String:
					return boost::lexical_cast<std::string>(a.mData.mFloat) <= b.mData.mString;
				default:
					break;
			}
			break;
		case Short:
			switch (b.mType) {
				case Integer:
					return a.mData.mShort <= b.mData.mInt;
				case Float:
					return a.mData.mShort <= b.mData.mFloat;
				case Short:
					return a.mData.mShort <= b.mData.mShort;
				case Byte:
					return a.mData.mShort <= b.mData.mByte;
				case String:
					return boost::lexical_cast<std::string>(a.mData.mShort) <= b.mData.mString;
				default:
					break;
			}
			break;
		case Byte:
			switch (b.mType) {
				case Integer:
					return a.mData.mByte <= b.mData.mInt;
				case Float:
					return a.mData.mByte <= b.mData.mFloat;
				case Short:
					return a.mData.mByte <= b.mData.mShort;
				case Byte:
					return a.mData.mByte <= b.mData.mByte;
				case String:
					return boost::lexical_cast<std::string>(a.mData.mByte) <= b.mData.mString;
				default:
					break;
			}
			break;
		case String:
			switch (b.mType) {
				case Integer:
					return a.mData.mString <= boost::lexical_cast<std::string>(b.mData.mInt);
				case Float:
					return a.mData.mString <= boost::lexical_cast<std::string>(b.mData.mFloat);
				case Short:
					return a.mData.mString <= boost::lexical_cast<std::string>(b.mData.mShort);
				case Byte:
					return a.mData.mString <= boost::lexical_cast<std::string>(b.mData.mByte);
				case String:
					return a.mData.mString <= b.mData.mString;
				default:
					break;
			}
			break;
		default:
			break;
	}
	flags |= OperationFlag::NoSuchOperation;
	return ConstantValue();
}

ConstantValue ConstantValue::add(const ConstantValue &a, const ConstantValue &b, OperationFlags &flags) {
	if ((a.mType == String) ^ (b.mType == String) ) {
		flags |= OperationFlag::CastToString;
	}
	switch (a.mType) {
		case Integer:
			switch (b.mType) {
				case Integer:
					return a.mData.mInt + b.mData.mInt;
				case Float:
					return a.mData.mInt + b.mData.mFloat;
				case Short:
					return a.mData.mInt + b.mData.mShort;
				case Byte:
					return a.mData.mInt + b.mData.mByte;
				case String:
					return boost::lexical_cast<std::string>(a.mData.mInt) + b.mData.mString;
				default:
					break;
			}
			break;
		case Float:
			switch (b.mType) {
				case Integer:
					return a.mData.mFloat + b.mData.mInt;
				case Float:
					return a.mData.mFloat + b.mData.mFloat;
				case Short:
					return a.mData.mFloat + b.mData.mShort;
				case Byte:
					return a.mData.mFloat + b.mData.mByte;
				case String:
					return boost::lexical_cast<std::string>(a.mData.mFloat) + b.mData.mString;
				default:
					break;
			}
			break;
		case Short:
			switch (b.mType) {
				case Integer:
					return a.mData.mShort + b.mData.mInt;
				case Float:
					return a.mData.mShort + b.mData.mFloat;
				case Short:
					return a.mData.mShort + b.mData.mShort;
				case Byte:
					return a.mData.mShort + b.mData.mByte;
				case String:
					return boost::lexical_cast<std::string>(a.mData.mShort) + b.mData.mString;
				default:
					break;
			}
		case Byte:
			switch (b.mType) {
				case Integer:
					return a.mData.mByte + b.mData.mInt;
				case Float:
					return a.mData.mByte + b.mData.mFloat;
				case Short:
					return a.mData.mByte + b.mData.mShort;
				case Byte:
					return a.mData.mByte + b.mData.mByte;
				case String:
					return boost::lexical_cast<std::string>(a.mData.mByte) + b.mData.mString;
				default:
					break;
			}
			break;
		case String:
			switch (b.mType) {
				case Integer:
					return a.mData.mString + boost::lexical_cast<std::string>(b.mData.mInt);
				case Float:
					return a.mData.mString + boost::lexical_cast<std::string>(b.mData.mFloat);
				case Short:
					return a.mData.mString + boost::lexical_cast<std::string>(b.mData.mShort);
				case Byte:
					return a.mData.mString + boost::lexical_cast<std::string>(b.mData.mByte);
				case String:
					return a.mData.mString + b.mData.mString;
				default:
					break;
			}
			break;
		default:
			break;
	}
	flags |= OperationFlag::NoSuchOperation;
	return ConstantValue();
}

ConstantValue ConstantValue::subtract(const ConstantValue &a, const ConstantValue &b, OperationFlags &flags) {
	if ((a.mType == String) ^ (b.mType == String) ) {
		flags |= OperationFlag::CastToString;
	}
	switch (a.mType) {
		case Integer:
			switch (b.mType) {
				case Integer:
					return a.mData.mInt - b.mData.mInt;
				case Float:
					return a.mData.mInt - b.mData.mFloat;
				case Short:
					return a.mData.mInt - b.mData.mShort;
				case Byte:
					return a.mData.mInt - b.mData.mByte;
				default:
					break;
			}
			break;
		case Float:
			switch (b.mType) {
				case Integer:
					return a.mData.mFloat - b.mData.mInt;
				case Float:
					return a.mData.mFloat - b.mData.mFloat;
				case Short:
					return a.mData.mFloat - b.mData.mShort;
				case Byte:
					return a.mData.mFloat - b.mData.mByte;
				default:
					break;
			}
			break;
		case Short:
			switch (b.mType) {
				case Integer:
					return a.mData.mShort - b.mData.mInt;
				case Float:
					return a.mData.mShort - b.mData.mFloat;
				case Short:
					return a.mData.mShort - b.mData.mShort;
				case Byte:
					return a.mData.mShort - b.mData.mByte;
				default:
					break;
			}
			break;
		case Byte:
			switch (b.mType) {
				case Integer:
					return a.mData.mByte - b.mData.mInt;
				case Float:
					return a.mData.mByte - b.mData.mFloat;
				case Short:
					return a.mData.mByte - b.mData.mShort;
				case Byte:
					return a.mData.mByte - b.mData.mByte;
				default:
					break;
			}
		default:
			break;
	}
	flags |= OperationFlag::NoSuchOperation;
	return ConstantValue();
}

ConstantValue ConstantValue::multiply(const ConstantValue &a, const ConstantValue &b, OperationFlags &flags) {
	if ((a.mType == String) ^ (b.mType == String) ) {
		flags |= OperationFlag::CastToString;
	}
	switch (a.mType) {
		case Integer:
			switch (b.mType) {
				case Integer:
					return a.mData.mInt * b.mData.mInt;
				case Float:
					return a.mData.mInt * b.mData.mFloat;
				case Short:
					return a.mData.mInt * b.mData.mShort;
				case Byte:
					return a.mData.mInt * b.mData.mByte;
				default:
					break;
			}
			break;
		case Float:
			switch (b.mType) {
				case Integer:
					return a.mData.mFloat * b.mData.mInt;
				case Float:
					return a.mData.mFloat * b.mData.mFloat;
				case Short:
					return a.mData.mFloat * b.mData.mShort;
				case Byte:
					return a.mData.mFloat * b.mData.mByte;
				default:
					break;
			}
			break;
		case Short:
			switch (b.mType) {
				case Integer:
					return a.mData.mShort * b.mData.mInt;
				case Float:
					return a.mData.mShort * b.mData.mFloat;
				case Short:
					return a.mData.mShort * b.mData.mShort;
				case Byte:
					return a.mData.mShort * b.mData.mByte;
				default:
					break;
			}
			break;
		case Byte:
			switch (b.mType) {
				case Integer:
					return a.mData.mByte * b.mData.mInt;
				case Float:
					return a.mData.mByte * b.mData.mFloat;
				case Short:
					return a.mData.mByte * b.mData.mShort;
				case Byte:
					return a.mData.mByte * b.mData.mByte;
				default:
					break;
			}
			break;
		default:
			break;
	}
	flags |= OperationFlag::NoSuchOperation;
	return ConstantValue();
}

ConstantValue ConstantValue::divide(const ConstantValue &a, const ConstantValue &b, OperationFlags &flags) {
	if ((a.mType == String) ^ (b.mType == String) ) {
		flags |= OperationFlag::CastToString;
	}
	switch (a.mType) {
		case Integer:
			switch (b.mType) {
				case Integer:
					if (b.mData.mInt == 0 ) { flags |= OperationFlag::IntegerDividedByZero; return ConstantValue(); }

					return a.mData.mInt / b.mData.mInt;
				case Float:
					return a.mData.mInt / b.mData.mFloat;
				case Short:
					if (b.mData.mShort != 0 ) { flags |= OperationFlag::IntegerDividedByZero; return ConstantValue(); }
					return a.mData.mInt / b.mData.mShort;
				case Byte:
					if (b.mData.mByte != 0 ) { flags |= OperationFlag::IntegerDividedByZero; return ConstantValue(); }
					return a.mData.mInt / b.mData.mByte;
				default:
					break;
			}
			break;
		case Float:
			//TODO: Warnings if result is NaN or INF
			switch (b.mType) {
				case Integer:
					return a.mData.mFloat / b.mData.mInt;
				case Float:
					return a.mData.mFloat / b.mData.mFloat;
				case Short:
					return a.mData.mFloat / b.mData.mShort;
				case Byte:
					return a.mData.mFloat / b.mData.mByte;
				default:
					break;
			}
			break;
		case Short:
			switch (b.mType) {
				case Integer:
					if (b.mData.mInt != 0 ) { flags |= OperationFlag::IntegerDividedByZero; return ConstantValue(); }
					return a.mData.mShort / b.mData.mInt;
				case Float:
					return a.mData.mShort / b.mData.mFloat;
				case Short:
					if (b.mData.mShort != 0 ) { flags |= OperationFlag::IntegerDividedByZero; return ConstantValue(); }
					return a.mData.mShort / b.mData.mShort;
				case Byte:
					if (b.mData.mByte != 0 ) { flags |= OperationFlag::IntegerDividedByZero; return ConstantValue(); }
					return a.mData.mShort / b.mData.mByte;
				default:
					break;
			}
			break;
		case Byte:
			switch (b.mType) {
				case Integer:
					if (b.mData.mInt != 0 ) { flags |= OperationFlag::IntegerDividedByZero; return ConstantValue(); }
					return a.mData.mByte / b.mData.mInt;
				case Float:
					return a.mData.mByte / b.mData.mFloat;
				case Short:
					if (b.mData.mShort != 0 ) { flags |= OperationFlag::IntegerDividedByZero; return ConstantValue(); }
					return a.mData.mByte / b.mData.mShort;
				case Byte:
					if (b.mData.mByte != 0 ) { flags |= OperationFlag::IntegerDividedByZero; return ConstantValue(); }
					return a.mData.mByte / b.mData.mByte;
				default:
					break;
			}
			break;
		default:
			break;
	}
	flags |= OperationFlag::NoSuchOperation;
	return ConstantValue();
}

ConstantValue ConstantValue::power(const ConstantValue &a, const ConstantValue &b, OperationFlags &flags) {
	switch (a.mType) {
		case Integer:
			switch (b.mType) {
				case Integer:
					return cbPow(a.mData.mInt, b.mData.mInt);
				case Float:
					return pow(a.mData.mInt, b.mData.mFloat);
				case Short:
					return cbPow(a.mData.mInt, b.mData.mShort);
				case Byte:
					return cbPow(a.mData.mInt, b.mData.mByte);
				default:
					break;
			}
		case Float:
			switch (b.mType) {
				case Integer:
					return pow(a.mData.mFloat, b.mData.mInt);
				case Float:
					return pow(a.mData.mFloat, b.mData.mFloat);
				case Short:
					return pow(a.mData.mFloat, b.mData.mShort);
				case Byte:
					return pow(a.mData.mFloat, b.mData.mByte);
				default:
					break;
			}
		case Short:
			switch (b.mType) {
				case Integer:
					return cbPow(a.mData.mShort, b.mData.mInt);
				case Float:
					return pow(a.mData.mShort, b.mData.mFloat);
				case Short:
					return cbPow(a.mData.mShort, b.mData.mShort);
				case Byte:
					return cbPow(a.mData.mShort, b.mData.mByte);
				default:
					break;
			}
		case Byte:
			switch (b.mType) {
				case Integer:
					return cbPow(a.mData.mByte, b.mData.mInt);
				case Float:
					return pow(a.mData.mByte, b.mData.mFloat);
				case Short:
					return cbPow(a.mData.mByte, b.mData.mShort);
				case Byte:
					return cbPow(a.mData.mByte, b.mData.mByte);
				default:
					break;
			}
		default:
			break;
	}
	flags |= OperationFlag::NoSuchOperation;
	return ConstantValue();
}

ConstantValue ConstantValue::mod(const ConstantValue &a, const ConstantValue &b, OperationFlags &flags) {
	switch (a.mType) {
		case Integer:
			switch (b.mType) {
				case Integer:
					return imod(a.mData.mInt, b.mData.mInt);
				case Float:
					return fmod(a.mData.mInt, b.mData.mFloat);
				case Short:
					return imod(a.mData.mInt, b.mData.mShort);
				case Byte:
					return imod(a.mData.mInt, b.mData.mByte);
				default:
					break;
			}
			break;
		case Float:
			switch (b.mType) {
				case Integer:
					return fmod(a.mData.mFloat, b.mData.mInt);
				case Float:
					return fmod(a.mData.mFloat, b.mData.mFloat);
				case Short:
					return fmod(a.mData.mFloat, b.mData.mShort);
				case Byte:
					return fmod(a.mData.mFloat, b.mData.mByte);
				default:
					break;
			}
			break;
		case Short:
			switch (b.mType) {
				case Integer:
					return imod(a.mData.mShort, b.mData.mInt);
				case Float:
					return fmod(a.mData.mShort, b.mData.mFloat);
				case Short:
					return imod(a.mData.mShort, b.mData.mShort);
				case Byte:
					return imod(a.mData.mShort, b.mData.mByte);
				default:
					break;
			}
			break;
		case Byte:
			switch (b.mType) {
				case Integer:
					return imod(a.mData.mByte, b.mData.mInt);
				case Float:
					return fmod(a.mData.mByte, b.mData.mFloat);
				case Short:
					return imod(a.mData.mByte, b.mData.mShort);
				case Byte:
					return imod(a.mData.mByte, b.mData.mByte);
				default:
					break;
			}
			break;
		default:
			break;
	}
	flags |= OperationFlag::NoSuchOperation;
	return ConstantValue();
}

ConstantValue ConstantValue::shr(const ConstantValue &a, const ConstantValue &b, OperationFlags &flags) {
	switch (a.mType) {
		case Integer:
			switch (b.mType) {
				case Integer:
					return  int((unsigned int)a.mData.mInt >> (unsigned int)b.mData.mInt);
				case Short:
					return int((unsigned int)a.mData.mInt >> b.mData.mShort);
				case Byte:
					return int((unsigned int)a.mData.mInt >> b.mData.mByte);
				default:
					break;
			}
		case Short:
			switch (b.mType) {
				case Integer:
					return int(a.mData.mShort >> (unsigned int)b.mData.mInt);
				case Short:
					return a.mData.mShort >> b.mData.mShort;
				case Byte:
					return a.mData.mShort >> b.mData.mByte;
				default:
					break;
			}
			break;
		case Byte:
			switch (b.mType) {
				case Integer:
					return int(a.mData.mByte >> (unsigned int)b.mData.mInt);
				case Short:
					return a.mData.mByte >> b.mData.mShort;
				case Byte:
					return a.mData.mByte >> b.mData.mByte;
				default:
					break;
			}
			break;
		default:
			break;
	}
	flags |= OperationFlag::NoSuchOperation;
	return ConstantValue();
}

ConstantValue ConstantValue::shl(const ConstantValue &a, const ConstantValue &b, OperationFlags &flags) {
	switch (a.mType) {
		case Integer:
			switch (b.mType) {
				case Integer:
					return  int((unsigned int)a.mData.mInt << (unsigned int)b.mData.mInt);
				case Short:
					return int((unsigned int)a.mData.mInt << b.mData.mShort);
				case Byte:
					return int((unsigned int)a.mData.mInt << b.mData.mByte);
				default:
					break;
			}
		case Short:
			switch (b.mType) {
				case Integer:
					return int(a.mData.mShort << (unsigned int)b.mData.mInt);
				case Short:
					return a.mData.mShort << b.mData.mShort;
				case Byte:
					return a.mData.mShort << b.mData.mByte;
				default:
					break;
			}
			break;
		case Byte:
			switch (b.mType) {
				case Integer:
					return int(a.mData.mByte << (unsigned int)b.mData.mInt);
				case Short:
					return a.mData.mByte << b.mData.mShort;
				case Byte:
					return a.mData.mByte << b.mData.mByte;
				default:
					break;
			}
			break;
		default:
			break;
	}
	flags |= OperationFlag::NoSuchOperation;
	return ConstantValue();
}

ConstantValue ConstantValue::sar(const ConstantValue &a, const ConstantValue &b, OperationFlags &flags) {
	switch (a.mType) {
		case Integer:
			switch (b.mType) {
				case Integer:
					return  a.mData.mInt >> b.mData.mInt;
				case Short:
					return a.mData.mInt >> b.mData.mShort;
				case Byte:
					return a.mData.mInt >> b.mData.mByte;
				default:
					break;
			}
		case Short:
			switch (b.mType) {
				case Integer:
					return a.mData.mShort >> b.mData.mInt;
				case Short:
					return a.mData.mShort >> b.mData.mShort;
				case Byte:
					return a.mData.mShort >> b.mData.mByte;
				default:
					break;
			}
			break;
		case Byte:
			switch (b.mType) {
				case Integer:
					return a.mData.mByte >> b.mData.mInt;
				case Short:
					return a.mData.mByte >> b.mData.mShort;
				case Byte:
					return a.mData.mByte >> b.mData.mByte;
				default:
					break;
			}
		default:
			break;
	}
	flags |= OperationFlag::NoSuchOperation;
	return ConstantValue();
}

ConstantValue ConstantValue::and_(const ConstantValue &a, const ConstantValue &b, OperationFlags &flags) {
	if (a.mType ==  Null || b.mType ==  Null) {
		flags |= OperationFlag::NoSuchOperation;
		return ConstantValue();
	}

	return a.toBool() && b.toBool();
}

ConstantValue ConstantValue::or_(const ConstantValue &a, const ConstantValue &b, OperationFlags &flags) {
	if (a.mType ==  Null || b.mType ==  Null) {
		flags |= OperationFlag::NoSuchOperation;
		return ConstantValue();
	}
	return a.toBool() || b.toBool();
}

ConstantValue ConstantValue::xor_(const ConstantValue &a, const ConstantValue &b, OperationFlags &flags){
	if (a.mType ==  Null || b.mType ==  Null) {
		flags |= OperationFlag::NoSuchOperation;
		return ConstantValue();
	}
	return a.toBool() ^ b.toBool();
}

int ConstantValue::cbIntPower(int a, int b) {
	return cbPow(a, b);
}

ConstantValue ConstantValue::to(ConstantValue::Type type) {
	switch(type) {
		case Boolean:
			return toBool();
		case Byte:
			return toByte();
		case Short:
			return toShort();
		case Integer:
			return toInt();
		case Float:
			return toFloat();
		case String:
			return toString();
		default:
			assert("Invalid cast" && 0);
	}
}

std::string ConstantValue::toString() const{
	switch(this->mType) {
		case Boolean:
			return mData.mBool ? "1" : "0";
		case Byte:
			return boost::lexical_cast<std::string>(mData.mByte);
		case Short:
			return boost::lexical_cast<std::string>(mData.mShort);
		case Integer:
			return boost::lexical_cast<std::string>(mData.mInt);
		case Float:
			return boost::lexical_cast<std::string>(mData.mFloat);
		case String:
			return mData.mString;
		default:
			return std::string();
	}
}

uint16_t ConstantValue::toShort() const{
	switch(this->mType) {
		case Boolean:
			return mData.mBool;
		case Byte:
			return mData.mByte;
		case Short:
			return mData.mShort;
		case Integer:
			return mData.mInt;
		case Float:
			return mData.mFloat + 0.5;
		case String:
			return boost::lexical_cast<uint16_t>(mData.mString);
		default:
			return 0;
	}
}

uint8_t ConstantValue::toByte() const {
	switch(this->mType) {
		case Boolean:
			return mData.mBool;
		case Byte:
			return mData.mByte;
		case Short:
			return mData.mShort;
		case Integer:
			return mData.mInt;
		case Float:
			return mData.mFloat + 0.5;
		case String:
			return boost::lexical_cast<uint8_t>(mData.mString);
		default:
			return 0;
	}
}

float ConstantValue::toFloat() const{
	switch(this->mType) {
		case Boolean:
			return mData.mBool;
		case Byte:
			return mData.mByte;
		case Short:
			return mData.mShort;
		case Integer:
			return mData.mInt;
		case Float:
			return mData.mFloat;
		case String:
			return boost::lexical_cast<float>(mData.mString);
		default:
			return 0;
	}
}

int ConstantValue::toInt() const{
	switch(this->mType) {
		case Boolean:
			return mData.mBool;
		case Byte:
			return mData.mByte;
		case Short:
			return mData.mShort;
		case Integer:
			return mData.mInt;
		case Float:
			return mData.mFloat + 0.5;
		case String:
			return boost::lexical_cast<int>(mData.mString);
		default:
			return 0;
	}
}

bool ConstantValue::toBool() const{
	switch(this->mType) {
		case Boolean:
			return mData.mBool;
		case Byte:
			return mData.mByte != 0;
		case Short:
			return mData.mShort != 0;
		case Integer:
			return mData.mInt != 0;
		case Float:
			return bool(mData.mFloat);
		case String:
			return !mData.mString.empty();
		default:
			return false;
	}
}

std::string ConstantValue::typeName() const {
	switch(mType) {
		case Integer:
			return "Integer";
		case Float:
			return "Float";
		case String:
			return "String";
		case Byte:
			return "Byte";
		case Short:
			return "Short";
		case Boolean:
			return "Boolean";
		case Null:
			return "NULL";
		default:
			return "Invalid";
	}
}

std::string ConstantValue::valueInfo() const {
	switch(this->mType) {
		case Boolean:
			return mData.mBool ? "true" : "false";
		case Byte:
			return boost::lexical_cast<std::string>(mData.mByte);
		case Short:
			return boost::lexical_cast<std::string>(mData.mShort);
		case Integer:
			return boost::lexical_cast<std::string>(mData.mInt);
		case Float:
			return boost::lexical_cast<std::string>(mData.mFloat);
		case String:
			return "\"" + mData.mString  + "\"";
		case  Null:
			return "NULL";
		default:
			return std::string("Invalid constant value");
	}
}
