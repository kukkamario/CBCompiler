#ifndef FUNCTIONVALUETYPE_H
#define FUNCTIONVALUETYPE_H
#include "valuetype.h"
class Function;
class FunctionValueType : public ValueType {
	public:
		FunctionValueType(Runtime *r, ValueType *returnValue, const QList<ValueType*> paramTypes);
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
		
		QList<ValueType*> paramTypes() const { return mParamTypes; }
		ValueType *returnType() const { return mReturnType; }
	protected:
		ValueType *mReturnType;
		QList<ValueType*> mParamTypes;
};

#endif // FUNCTIONVALUETYPE_H
