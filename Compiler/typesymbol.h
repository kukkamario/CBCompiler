#ifndef TYPESYMBOL_H
#define TYPESYMBOL_H
#include "symbol.h"
#include <QMap>
#include <QList>

namespace llvm{
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
		TypeField(const QString &name, ValueType *valueType, QFile *file, int line);
		QString name()const{return mName;}
		ValueType *valueType()const{return mValueType;}
		QString info() const;
	private:
		QString mName;
		ValueType *mValueType;
		int mLine;
		QFile *mFile;
};

class TypeSymbol : public Symbol {
	public:
		TypeSymbol(const QString &name, QFile *file, int line);
		Type type()const{return stType;}
		QString info() const;
		bool addField(const TypeField &field);
		bool hasField(const QString &name);
		const TypeField &field(const QString &name) const;
		llvm::Type *llvmMemberType() const {return mMemberType;}
		void initializeType(Builder *b);
		void createTypePointerValueType(Builder *b);
		TypePointerValueType *typePointerValueType()const{return mTypePointerValueType;}
		llvm::Value *globalTypeVariable() { return mGlobalTypeVariable; }
	private:
		void createLLVMMemberType();
		QList<TypeField> mFields;
		QMap<QString, QList<TypeField>::Iterator> mFieldSearch;
		Runtime *mRuntime;
		llvm::Type *mMemberType;
		llvm::GlobalVariable *mGlobalTypeVariable;
		TypePointerValueType *mTypePointerValueType;
		unsigned mFirstFieldIndex;
		int mMemberSize;
};

#endif // TYPESYMBOL_H
