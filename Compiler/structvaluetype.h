#ifndef CLASSVALUETYPE_H
#define CLASSVALUETYPE_H
#include "valuetype.h"
#include "codepoint.h"
#include <QMap>
#include <QList>

class StructField {
	public:
		StructField(const QString &name, ValueType *valueType, const CodePoint &cp);
		QString name()const{return mName;}
		ValueType *valueType()const{return mValueType;}
		QString info() const;
		int line() const { return mCodePoint.line(); }
		QString file() const { return mCodePoint.file(); }
		int column() const { return mCodePoint.column(); }
		const CodePoint &codePoint() const { return mCodePoint; }
	private:
		QString mName;
		ValueType *mValueType;
		CodePoint mCodePoint;
};


class StructValueType : public ValueType {
	public:
		StructValueType(const QString &name, const CodePoint &cp, Runtime *runtime);
		StructValueType(const QString &name, const CodePoint &cp, const QList<StructField> &fields, Runtime *runtime);
		~StructValueType();
		bool containsValueType(const ValueType *valueType) const;
		bool containsItself() const;

		virtual QString name() const;
		virtual CastCost castingCostToOtherValueType(const ValueType *to) const;
		virtual Value cast(Builder *builder, const Value &v) const;
		bool isTypePointer() const{return false;}
		bool isNumber() const{return false;}
		bool isStruct() const { return true; }
		llvm::Constant *defaultValue() const;
		int size() const;
		void setFields(const QList<StructField> &fields);
		const CodePoint &codePoint() const { return mCodePoint; }
		void createOpaqueType(Builder *builder);
		void generateLLVMType();
		bool isNamedValueType() const { return true; }
		virtual Value generateOperation(Builder *builder, int opType, const Value &operand1, const Value &operand2, OperationFlags &operationFlags) const;
		void generateDestructor(Builder *builder, const Value &v);
		Value generateLoad(Builder *builder, const Value &var) const;
		virtual Value member(Builder *builder, const Value &a, const QString &memberName) const;
		virtual ValueType *memberType(const QString &memberName) const;
		Value field(Builder *builder, const Value &v, int fieldIndex) const;

		llvm::StructType *structType() const { return mStructType; }
	protected:
		QString mName;
		CodePoint mCodePoint;
		llvm::StructType *mStructType;
		QList<StructField> mFields;
		QMap<QString, QList<StructField>::ConstIterator> mFieldSearch;
};

#endif // CLASSVALUETYPE_H
