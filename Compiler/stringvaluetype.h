#ifndef STRINGVALUETYPE_H
#define STRINGVALUETYPE_H
#include "valuetype.h"
#include "llvm.h"
class StringPool;
class StringValueType : public ValueType {
	public:
		StringValueType(StringPool *strPool,Runtime *r, llvm::Module *mod);
		QString name() const {return "string";}
		eType type() const{return String;}
		void setStringType(llvm::Type *t) {mType = t;}
		bool setConstructFunction(llvm::Function *func);
		bool setAssignmentFunction(llvm::Function *func);
		bool setDestructFunction(llvm::Function *func);
		bool setAdditionFunction(llvm::Function *func);
		bool setFloatToStringFunction(llvm::Function *func);
		bool setIntToStringFunction(llvm::Function *func);
		bool setStringToIntFunction(llvm::Function *func);
		bool setStringToFloatFunction(llvm::Function *func);
		bool setEqualityFunction(llvm::Function *func);
		bool setRefFunction(llvm::Function *func);

		void assignString(llvm::IRBuilder<> *builder, llvm::Value *var, llvm::Value *string);
		llvm::Value *constructString(llvm::IRBuilder<> *builder, llvm::Value *globalStrPtr);
		void destructString(llvm::IRBuilder<> *builder, llvm::Value *stringVar);
		llvm::Value *stringToIntCast(llvm::IRBuilder<> *builder, llvm::Value *str);
		llvm::Value *stringToFloatCast(llvm::IRBuilder<> *builder, llvm::Value *str);
		llvm::Value *intToStringCast(llvm::IRBuilder<> *builder, llvm::Value *i);
		llvm::Value *floatToStringCast(llvm::IRBuilder<> *builder, llvm::Value *f);
		llvm::Value *stringAddition(llvm::IRBuilder<> *builder, llvm::Value *str1, llvm::Value *str2);
		llvm::Value *stringEquality(llvm::IRBuilder<> *builder, llvm::Value *a, llvm::Value *b);
		void refString(llvm::IRBuilder<> *builder, llvm::Value *a);

		bool isValid() const;
		/** Calculates cost for casting given ValueType to this ValueType.
		  * If returned cost is over maxCastCost, cast cannot be done. */
		CastCostType castingCostToOtherValueType(ValueType *to) const;
		Value cast(Builder *builder, const Value &v) const;
		bool isTypePointer() const{return false;}
		bool isNumber() const{return false;}
		int size() const;

		llvm::Constant *defaultValue() const;
	private:
		llvm::Function *mConstructFunction;
		llvm::Function *mAssignmentFunction;
		llvm::Function *mDestructFunction;
		llvm::Function *mAdditionFunction;
		llvm::Function *mFloatToStringFunction;
		llvm::Function *mIntToStringFunction;
		llvm::Function *mStringToIntFunction;
		llvm::Function *mStringToFloatFunction;
		llvm::Function *mEqualityFunction;
		llvm::Function *mRefFunction;
		StringPool *mStringPool;
};

#endif // STRINGVALUETYPE_H
