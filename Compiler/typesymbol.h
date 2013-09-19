#ifndef TYPESYMBOL_H
#define TYPESYMBOL_H
#include "symbol.h"
#include <QMap>
#include <QList>
#include "value.h"
#include "valuetypesymbol.h"

namespace llvm{
	class StructType;
	class Type;
	class Value;
	class Module;
	class GlobalVariable;
}
class Builder;
class TypePointerValueType;
class ValueType;
class Runtime;
class TypeField {
	public:
		TypeField(const QString &name, ValueType *valueType, const QString &file, int line);
		QString name()const{return mName;}
		ValueType *valueType()const{return mValueType;}
		QString info() const;
		int line() const { return mLine; }
		QString file() const { return mFile; }
	private:
		QString mName;
		ValueType *mValueType;
		int mLine;
		QString mFile;
};

class TypeSymbol : public ValueTypeSymbol {
	public:
		TypeSymbol(const QString &name, Runtime *r, const QString &file, int line);
		Type type()const{return stType;}
		QString info() const;
		bool addField(const TypeField &field);
		bool hasField(const QString &name);
		const TypeField &field(const QString &name) const;
		int fieldIndex(const QString &name) const;
		llvm::StructType *llvmMemberType() const {return mMemberType;}
		ValueType *valueType() const;

		void initializeType(Builder *b);
		void createOpaqueTypes(Builder *b);
		void createTypePointerValueType(Builder *b);
		TypePointerValueType *typePointerValueType()const{return mTypePointerValueType;}
		llvm::GlobalVariable *globalTypeVariable() { return mGlobalTypeVariable; }

		Value typeValue();
	private:
		void createLLVMMemberType();
		QList<TypeField> mFields;
		QMap<QString, int> mFieldSearch;
		Runtime *mRuntime;
		llvm::StructType *mMemberType;
		llvm::GlobalVariable *mGlobalTypeVariable;
		TypePointerValueType *mTypePointerValueType;
		unsigned mFirstFieldIndex;
		int mMemberSize;
};

#endif // TYPESYMBOL_H
