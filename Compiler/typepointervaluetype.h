#ifndef TYPEPOINTERVALUETYPE_H
#define TYPEPOINTERVALUETYPE_H
#include "valuetype.h"
class TypeSymbol;
class TypePointerValueType : public ValueType {
	public:
		TypePointerValueType(Runtime *r, TypeSymbol *s);
		virtual QString name()const;
		virtual CastCost castingCostToOtherValueType(ValueType *to) const;
		virtual Value cast(Builder *builder, const Value &v) const;
		TypeSymbol *typeSymbol() const {return mTypeSymbol;}
		bool isTypePointer() const{return true;}
		bool isNumber() const{return false;}
		llvm::Constant *defaultValue() const;
		int size() const;
		void setLLVMType(llvm::Type *type) { mType = type; }
	private:
		TypeSymbol *mTypeSymbol;
};

class TypePointerCommonValueType : public ValueType {
	public:
		TypePointerCommonValueType(Runtime *r, llvm::Type *type) : ValueType(r) { mType = type; }
		virtual QString name() const { return "TypePointerCommon"; }
		virtual CastCost castingCostToOtherValueType(ValueType *to) const;
		virtual Value cast(Builder *builder, const Value &v) const;
		TypeSymbol *typeSymbol() const {return mTypeSymbol;}
		bool isTypePointer() const{return true;}
		bool isNumber() const{return false;}
		llvm::Constant *defaultValue() const;
		int size() const;
	private:
		TypeSymbol *mTypeSymbol;
};
#endif // TYPEPOINTERVALUETYPE_H
