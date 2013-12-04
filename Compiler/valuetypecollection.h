#ifndef VALUETYPECOLLECTION_H
#define VALUETYPECOLLECTION_H
#include <QMap>
class ValueType;
class ArrayValueType;
namespace llvm { class Type; }
class ValueTypeCollection {
	public:
		ValueTypeCollection();
		~ValueTypeCollection();
		void addValueType(ValueType *valType);
		ValueType *valueTypeForLLVMType(llvm::Type *type);
		ValueType *findNamedType(const QString &name);

		ArrayValueType *arrayValueType(ValueType *baseValueType, int dimensions);
	private:
		QMap<QPair<ValueType*, int> , ValueType *> mArrayMapping;
		QMap<llvm::Type*, ValueType*> mLLVMTypeMapping;
		QMap<QString, ValueType *> mNamedType;
};

#endif // VALUETYPECOLLECTION_H
