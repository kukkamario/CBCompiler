#ifndef VALUETYPECOLLECTION_H
#define VALUETYPECOLLECTION_H
#include <QMap>
#include "constantvalue.h"

class ValueType;
class ArrayValueType;
class Runtime;
class StructValueType;
class TypePointerValueType;
namespace llvm { class Type; }
class ValueTypeCollection {
	public:
		ValueTypeCollection(Runtime *r);
		~ValueTypeCollection();
		void addValueType(ValueType *valType);
		void addTypePointerValueType(TypePointerValueType *typePointer);
		void addStructValueType(StructValueType *classValueType);
		ValueType *valueTypeForLLVMType(llvm::Type *type);
		ValueType *findNamedType(const QString &name);

		ArrayValueType *arrayValueType(ValueType *baseValueType, int dimensions);

		ValueType *constantValueType(ConstantValue::Type type) const;
		QList<ValueType*> namedTypes() const;
	private:
		ValueType *generateArrayValueType(llvm::StructType *arrayDataType);

		QMap<QPair<ValueType*, int> , ArrayValueType *> mArrayMapping;
		QMap<llvm::Type*, ValueType*> mLLVMTypeMapping;
		QMap<QString, ValueType *> mNamedType;
		QList<StructValueType*> mStructes;
		QList<TypePointerValueType*> mTypes;
		Runtime *mRuntime;
};

#endif // VALUETYPECOLLECTION_H
