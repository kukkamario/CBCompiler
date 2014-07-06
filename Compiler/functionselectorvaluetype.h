#ifndef FUNCTIONSELECTORVALUETYPE_H
#define FUNCTIONSELECTORVALUETYPE_H
#include "valuetype.h"

class FunctionSelectorValueType : public ValueType {
	public:
		FunctionSelectorValueType(Runtime *runtime, const QList<Function*> &functions);
		~FunctionSelectorValueType() {}

		QString name() const;
		virtual bool isNamedValueType() const { return false; }
		llvm::Type *llvmType() const {return mType;}
		bool isTypePointer() const { return false; }
		bool isNumber() const { return false; }
		bool isFunctionSelector() const { return true; }
		virtual llvm::Constant *defaultValue() const;

		virtual int size() const;

		virtual CastCost castingCostToOtherValueType(const ValueType *to) const;

		virtual Value cast(Builder *, const Value &v) const;

		virtual bool isCallable() const { return true; }

		QList<Function*> overloads() const;

	protected:
		QList<Function*> mFunctions;
};

#endif // FUNCTIONSELECTORVALUETYPE_H
