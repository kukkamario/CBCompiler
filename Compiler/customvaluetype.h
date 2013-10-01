#ifndef CUSTOMVALUETYPE_H
#define CUSTOMVALUETYPE_H
#include "valuetype.h"

class CustomValueType : public ValueType {
	public:
		CustomValueType(const QString &name, llvm::Type *type, Runtime *r);
		QString name() const { return mName; }
		eType type() const { return Custom; }
		CastCostType castingCostToOtherValueType(ValueType *to) const;
		Value cast(Builder *builder, const Value &v) const;
		llvm::Constant *defaultValue() const;
		bool isTypePointer() const { return false; }
		bool isNumber() const { return false; }
		int size() const;
	protected:
		QString mName;
};


#endif // CUSTOMVALUETYPE_H
