#ifndef TYPEVALUETYPE_H
#define TYPEVALUETYPE_H
#include "valuetype.h"

class TypeValueType : public ValueType {
	public:
		TypeValueType(Runtime *r, llvm::Type *type);
		QString name() const { return QObject::tr("Type"); }
		llvm::Type *llvmType() { return mType; }
		/** Calculates cost for casting given ValueType to this ValueType.
		  * If returned cost is over maxCastCost, cast cannot be done. */
		CastCost castingCostToOtherValueType(const ValueType *to) const;
		Value cast(Builder *, const Value &v) const;
		llvm::Constant *defaultValue() const;
		bool setConstructTypeFunction(llvm::Function * func);
		bool setNewFunction(llvm::Function *func);
		bool setFirstFunction(llvm::Function *func);
		bool setLastFunction(llvm::Function *func);
		bool setBeforeFunction(llvm::Function *func);
		bool setAfterFunction(llvm::Function *func);
		llvm::Function *constructTypeFunction() const { return mConstructTypeFunction; }
		llvm::Function *newFunction() const { return mNewFunction; }
		llvm::Function *firstFunction() const { return mFirstFunction; }
		llvm::Function *lastFunction() const { return mLastFunction; }
		llvm::Function *beforeFunction() const { return mBeforeFunction; }
		llvm::Function *afterFunction() const { return mAfterFunction; }
		bool isTypePointer() const{return false;}
		bool isNumber() const{return false;}
		int size() const;
		bool isValid();
		bool isNamedValueType() const { return true; }
	private:
	llvm::Function *mConstructTypeFunction;
	llvm::Function *mNewFunction;
	llvm::Function *mFirstFunction;
	llvm::Function *mLastFunction;
	llvm::Function *mBeforeFunction;
	llvm::Function *mAfterFunction;
};

#endif // TYPEVALUETYPE_H
