#ifndef VALUETYPECOLLECTION_H
#define VALUETYPECOLLECTION_H
#include <QMap>
#include "constantvalue.h"

class ValueType;
class ArrayValueType;
class Runtime;
namespace llvm { class Type; }
class ValueTypeCollection {
	public:
		ValueTypeCollection(Runtime *r);
		~ValueTypeCollection();
		void addValueType(ValueType *valType);
		ValueType *valueTypeForLLVMType(llvm::Type *type);
		ValueType *findNamedType(const QString &name);

		ArrayValueType *arrayValueType(ValueType *baseValueType, int dimensions);

		ValueType *constantValueType(ConstantValue::Type type) const;
		QList<ValueType*> namedTypes() const;
	private:
		QMap<QPair<ValueType*, int> , ArrayValueType *> mArrayMapping;
		QMap<llvm::Type*, ValueType*> mLLVMTypeMapping;
		QMap<QString, ValueType *> mNamedType;
		Runtime *mRuntime;
};

#endif // VALUETYPECOLLECTION_H
