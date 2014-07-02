#ifndef FUNCTIONVALUETYPE_H
#define FUNCTIONVALUETYPE_H
#include "valuetype.h"
class Function;
class FunctionValueType : public ValueType {
	public:
		FunctionValueType(Runtime *r, Function *function);
		~FunctionValueType() {}
		QString name() const;
		virtual bool isNamedValueType() const { return false; }
		llvm::Type *llvmType() const {return mType;}
		bool isTypePointer() const { return false; }
		bool isNumber() const { return false; }
		virtual llvm::Constant *defaultValue() const;

		virtual int size() const;

		virtual CastCost castingCostToOtherValueType(const ValueType *to) const;

		virtual Value cast(Builder *, const Value &v) const;
		
		virtual bool isCallable() const { return true; }
		
		const QList<ValueType*> &paramTypes() const { return mParamTypes; }
		ValueType *returnType() const { return mReturnType; }
		Function *function() const { return mFunction;  }
	protected:
		ValueType *mReturnType;
		QList<ValueType*> mParamTypes;
		Function *mFunction;
};

#endif // FUNCTIONVALUETYPE_H
