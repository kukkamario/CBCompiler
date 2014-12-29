#ifndef TYPESYMBOL_H
#define TYPESYMBOL_H
#include "symbol.h"
#include <map>
#include <vector>
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
		TypeField(const std::string &name, ValueType *valueType, const CodePoint &cp);
		std::string name()const{return mName;}
		ValueType *valueType()const{return mValueType;}
		std::string info() const;
		int line() const { return mCodePoint.line(); }
		boost::string_ref file() const { return mCodePoint.file(); }
		int column() const { return mCodePoint.column(); }
		const CodePoint &codePoint() const { return mCodePoint; }
	private:
		std::string mName;
		ValueType *mValueType;
		CodePoint mCodePoint;
};

class TypeSymbol : public ValueTypeSymbol {
	public:
		TypeSymbol(const std::string &name, Runtime *r, const CodePoint &cp);
		Type type()const{return stType;}
		std::string info() const;
		bool addField(const TypeField &field);
		bool hasField(const std::string &name);
		const TypeField &field(const std::string &name) const;
		int fieldIndex(const std::string &name) const;
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
		std::vector<TypeField> mFields;
		std::map<std::string, int> mFieldSearch;
		Runtime *mRuntime;
		llvm::StructType *mMemberType;
		llvm::GlobalVariable *mGlobalTypeVariable;
		TypePointerValueType *mTypePointerValueType;
		unsigned mFirstFieldIndex;
		int mMemberSize;
};

#endif // TYPESYMBOL_H
