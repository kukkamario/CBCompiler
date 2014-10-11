#ifndef VALUETYPECOLLECTION_H
#define VALUETYPECOLLECTION_H
#include <QMap>
#include "constantvalue.h"
#include "valuetype.h"

class ArrayValueType;
class Runtime;
class StructValueType;
class TypePointerValueType;
class FunctionValueType;
namespace llvm { class Type; }
class ValueTypeCollection {
	public:
		ValueTypeCollection(Runtime *r);
		~ValueTypeCollection();
		void addValueType(ValueType *valType);
		void addTypePointerValueType(TypePointerValueType *typePointer);
		void addStructValueType(StructValueType *structValueType);
		ValueType *valueTypeForLLVMType(llvm::Type *type);
		ValueType *findNamedType(const QString &name);

		ArrayValueType *arrayValueType(ValueType *baseValueType, int dimensions);
		FunctionValueType *functionValueType(ValueType *returnType, const QList<ValueType*> &paramTypes);

		ValueType *constantValueType(ConstantValue::Type type) const;
		QList<ValueType*> namedTypes() const;

		const QList<StructValueType*> structValueTypes() const { return mStructs; }
		const QList<TypePointerValueType*> typePointerValueTypes() const { return mTypes; }
	private:
		struct FunctionType {
				ValueType *mReturnType;
				QList<ValueType*> mParamTypes;
				bool operator < (const FunctionType &ft) const;
		};

		ValueType *generateArrayValueType(llvm::StructType *arrayDataType);
		ValueType *generateFunctionValueType(llvm::FunctionType *funcTy);

		QMap<QPair<ValueType*, int> , ArrayValueType *> mArrayMapping;
		QMap<FunctionType, FunctionValueType *> mFunctionTypeMapping;
		QMap<llvm::Type*, ValueType*> mLLVMTypeMapping;
		QMap<QString, ValueType *> mNamedType;
		QList<StructValueType*> mStructs;
		QList<TypePointerValueType*> mTypes;
		Runtime *mRuntime;
};

#endif // VALUETYPECOLLECTION_H
