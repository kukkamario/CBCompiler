#ifndef CONSTANTVALUE_H
#define CONSTANTVALUE_H
#include <QString>
#include "unionwrapper.h"

class ValueType;
class ConstantValue {
	public:
		enum Type {
			Byte,
			Short,
			Integer,
			Float,
			Boolean,
			String,
			Null,
			Invalid
		};

		ConstantValue();
		ConstantValue(bool t);
		ConstantValue(int i);
		ConstantValue(double d);
		ConstantValue(float f);
		ConstantValue(quint8 b);
		ConstantValue(quint16 s);
		ConstantValue(const QString s);
		ConstantValue(const ConstantValue &o);
		ConstantValue(Type type);
		~ConstantValue();
		ConstantValue &operator=(const ConstantValue &v);
		bool operator== (const ConstantValue &o);
		bool operator!= (const ConstantValue &o);
		bool isValid() const{return mType != Invalid;}
		Type type() const {return mType;}

		static ConstantValue plus(const ConstantValue &a);
		static ConstantValue minus(const ConstantValue &a);
		static ConstantValue not_(const ConstantValue &a);
		static ConstantValue equal(const ConstantValue &a, const ConstantValue &b);
		static ConstantValue notEqual(const ConstantValue &a, const ConstantValue &b);
		static ConstantValue greater(const ConstantValue &a, const ConstantValue &b);
		static ConstantValue greaterEqual(const ConstantValue &a, const ConstantValue &b);
		static ConstantValue less(const ConstantValue &a, const ConstantValue &b);
		static ConstantValue lessEqual(const ConstantValue &a, const ConstantValue &b);
		static ConstantValue add(const ConstantValue &a, const ConstantValue &b);
		static ConstantValue subtract(const ConstantValue &a, const ConstantValue &b);
		static ConstantValue multiply(const ConstantValue &a, const ConstantValue &b);
		static ConstantValue divide(const ConstantValue &a, const ConstantValue &b);
		static ConstantValue power(const ConstantValue &a, const ConstantValue &b);
		static ConstantValue mod(const ConstantValue &a, const ConstantValue &b);
		static ConstantValue shr(const ConstantValue &a, const ConstantValue &b);
		static ConstantValue shl(const ConstantValue &a, const ConstantValue &b);
		static ConstantValue sar(const ConstantValue &a, const ConstantValue &b);
		static ConstantValue and_(const ConstantValue &a, const ConstantValue &b);
		static ConstantValue or_(const ConstantValue &a, const ConstantValue &b);
		static ConstantValue xor_(const ConstantValue &a, const ConstantValue &b);

		static int cbIntPower(int a, int b);

		ConstantValue to(Type type);
		QString toString() const;
		quint16 toShort() const;
		quint8 toByte() const;
		float toFloat() const;
		int toInt() const;
		bool toBool() const;
		QString typeName() const;
		QString valueInfo() const;
	private:
		Type mType;
		union {
				bool mBool;
				int mInt;
				float mFloat;
				quint16 mShort;
				quint8 mByte;
				UnionWrapper<QString> mString;
		} mData;
};

#endif // CONSTANTVALUE_H
