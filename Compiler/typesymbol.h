#ifndef TYPESYMBOL_H
#define TYPESYMBOL_H
#include "symbol.h"
#include <QMap>
#include <QLinkedList>
namespace llvm{
	class Type;
	class Value;
	class Module;
}
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
		llvm::Type *llvmType()const{return mType;}
		bool createTypePointerValueType(Runtime *r);
		TypePointerValueType *typePointerValueType()const{return mTypePointerValueType;}
	private:
		bool createLLVMType(llvm::Module *mod);
		QLinkedList<TypeField> mFields;
		QMap<QString, QLinkedList<TypeField>::Iterator> mFieldSearch;
		llvm::Type *mType;
		llvm::Type *mMemberType;
		TypePointerValueType *mTypePointerValueType;
};

#endif // TYPESYMBOL_H
