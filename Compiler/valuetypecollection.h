#ifndef VALUETYPECOLLECTION_H
#define VALUETYPECOLLECTION_H
#include <map>
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
		ValueType *findNamedType(const std::string &name);

		ArrayValueType *arrayValueType(ValueType *baseValueType, int dimensions);
		FunctionValueType *functionValueType(ValueType *returnType, const std::vector<ValueType*> &paramTypes);

		ValueType *constantValueType(ConstantValue::Type type) const;
		std::vector<ValueType*> namedTypes() const;

		const std::vector<StructValueType*> structValueTypes() const { return mStructs; }
		const std::vector<TypePointerValueType*> typePointerValueTypes() const { return mTypes; }
	private:
		struct FunctionType {
				ValueType *mReturnType;
				std::vector<ValueType*> mParamTypes;
				bool operator < (const FunctionType &ft) const;
		};

		ValueType *generateArrayValueType(llvm::StructType *arrayDataType);
		ValueType *generateFunctionValueType(llvm::FunctionType *funcTy);

		std::map<std::pair<ValueType*, int> , ArrayValueType *> mArrayMapping;
		std::map<FunctionType, FunctionValueType *> mFunctionTypeMapping;
		std::map<llvm::Type*, ValueType*> mLLVMTypeMapping;
		std::map<std::string, ValueType *> mNamedType;
		std::vector<StructValueType*> mStructs;
		std::vector<TypePointerValueType*> mTypes;
		Runtime *mRuntime;
};

#endif // VALUETYPECOLLECTION_H
