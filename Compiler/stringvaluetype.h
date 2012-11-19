#ifndef STRINGVALUETYPE_H
#define STRINGVALUETYPE_H
#include "valuetype.h"
#include "llvm.h"
class StringPool;
class StringValueType : public ValueType {
	public:
		StringValueType(StringPool *strPool,Runtime *r, llvm::Module *mod);
		QString name() const {return QObject::tr("String");}
		Type type() const{return String;}
		void setStringType(llvm::Type *t) {mType = t;}
		bool setConstructFunction(llvm::Function *func);
		bool setAssignmentFunction(llvm::Function *func);
		bool setDestructFunction(llvm::Function *func);
		bool setAdditionFunction(llvm::Function *func);
		bool setFloatToStringFunction(llvm::Function *func);
		bool setIntToStringFunction(llvm::Function *func);
		bool setStringToIntFunction(llvm::Function *func);
		bool setStringToFloatFunction(llvm::Function *func);
		llvm::Value *constructString(llvm::IRBuilder<> *builder, QString *str);
		llvm::Value *constructString(llvm::IRBuilder<> *builder, llvm::Value *globalStrPtr);
		void destructString(llvm::IRBuilder<> *builder, llvm::Value *stringVar);
		llvm::Value *stringToIntCast(llvm::IRBuilder<> *builder, llvm::Value *str);
		llvm::Value *stringToFloatCast(llvm::IRBuilder<> *builder, llvm::Value *str);
		llvm::Value *intToStringCast(llvm::IRBuilder<> *builder, llvm::Value *i);
		llvm::Value *floatToStringCast(llvm::IRBuilder<> *builder, llvm::Value *f);
		llvm::Value *stringAddition(llvm::IRBuilder<> *builder, llvm::Value *str1, llvm::Value *str2);

		/** Calculates cost for casting given ValueType to this ValueType.
		  * If returned cost is over maxCastCost, cast cannot be done. */
		CastCostType castCost(ValueType *to) const;
		Value cast(llvm::IRBuilder<> *builder, const Value &v) const;
	private:
		llvm::Function *mConstructFunction;
		llvm::Function *mAssignmentFunction;
		llvm::Function *mDestructFunction;
		llvm::Function *mAdditionFunction;
		llvm::Function *mFloatToStringFunction;
		llvm::Function *mIntToStringFunction;
		llvm::Function *mStringToIntFunction;
		llvm::Function *mStringToFloatFunction;
		StringPool *mStringPool;
};

#endif // STRINGVALUETYPE_H
