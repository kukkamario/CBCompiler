#ifndef STRINGVALUETYPE_H
#define STRINGVALUETYPE_H
#include "valuetype.h"
#include "llvm.h"
class StringValueType : public ValueType {
	public:
		StringValueType(llvm::Module *mod);
		QString name() const {return "String";}
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

		/** Calculates cost for casting given ValueType to this ValueType.
		  * If returned cost is over maxCastCost, cast cannot be done. */
		CastCostType castCost(ValueType *to) const;
		Value cast(const Value &v) const;
	private:
		llvm::Function *mConstructFunction;
		llvm::Function *mAssignmentFunction;
		llvm::Function *mDestructFunction;
		llvm::Function *mAdditionFunction;
		llvm::Function *mFloatToStringFunction;
		llvm::Function *mIntToStringFunction;
		llvm::Function *mStringToIntFunction;
		llvm::Function *mStringToFloatFunction;
};

#endif // STRINGVALUETYPE_H
