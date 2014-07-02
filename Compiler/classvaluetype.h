#ifndef CLASSVALUETYPE_H
#define CLASSVALUETYPE_H
#include "valuetype.h"
#include "codepoint.h"
#include <QMap>
#include <QList>

class ClassField {
	public:
		ClassField(const QString &name, ValueType *valueType, const CodePoint &cp);
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


class ClassValueType : public ValueType {
	public:
		ClassValueType(const QString &name, const CodePoint &cp, Runtime *runtime);
		ClassValueType(const QString &name, const CodePoint &cp, const QList<ClassField> &fields, Runtime *runtime);
		~ClassValueType();
		virtual QString name() const;
		virtual CastCost castingCostToOtherValueType(const ValueType *to) const;
		virtual Value cast(Builder *builder, const Value &v) const;
		bool isTypePointer() const{return false;}
		bool isNumber() const{return false;}
		bool isClass() const { return true; }
		llvm::Constant *defaultValue() const;
		int size() const;
		void setFields(const QList<ClassField> &fields);
		const CodePoint &codePoint() const { return mCodePoint; }
		void createOpaqueType(Builder *builder);
		void generateLLVMType();
		bool isNamedValueType() const { return true; }
		virtual Value generateOperation(Builder *builder, int opType, const Value &operand1, const Value &operand2, OperationFlags &operationFlags) const;
		virtual Value member(Builder *builder, const Value &a, const QString &memberName) const;
		virtual ValueType *memberType(const QString &memberName) const;
		llvm::StructType *structType() const { return mStructType; }
	protected:
		QString mName;
		CodePoint mCodePoint;
		llvm::StructType *mStructType;
		QList<ClassField> mFields;
		QMap<QString, QList<ClassField>::ConstIterator> mFieldSearch;
};

#endif // CLASSVALUETYPE_H
