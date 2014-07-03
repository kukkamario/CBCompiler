#include "valuetypecollection.h"
#include "valuetype.h"
#include "arrayvaluetype.h"
#include "intvaluetype.h"
#include "floatvaluetype.h"
#include "shortvaluetype.h"
#include "bytevaluetype.h"
#include "booleanvaluetype.h"
#include "stringvaluetype.h"
#include "typepointervaluetype.h"
#include "genericarrayvaluetype.h"
#include "structvaluetype.h"
#include "nullvaluetype.h"
#include "runtime.h"

ValueTypeCollection::ValueTypeCollection(Runtime *r) :
mRuntime(r) {
}

ValueTypeCollection::~ValueTypeCollection() {
}

void ValueTypeCollection::addValueType(ValueType *valType) {
	mLLVMTypeMapping[valType->llvmType()] = valType;
	if (valType->isNamedValueType())
		mNamedType[valType->name().toLower()] = valType;
}

void ValueTypeCollection::addTypePointerValueType(TypePointerValueType *typePointer) {
	mTypes.append(typePointer);
	addValueType(typePointer);
}

void ValueTypeCollection::addStructValueType(StructValueType *structValueType) {
	mStructs.append(structValueType);
	addValueType(structValueType);
}

ValueType *ValueTypeCollection::valueTypeForLLVMType(llvm::Type *type) {
	ValueType *valType = mLLVMTypeMapping.value(type, 0);
	if (valType) return valType;

	//Check if type is ArrayValueType
	if (type->isPointerTy()) {
		llvm::PointerType *ptrType = llvm::cast<llvm::PointerType>(type);
		llvm::Type *elementType = ptrType->getElementType();
		if (elementType->isStructTy()) {
			llvm::StructType *structType = llvm::cast<llvm::StructType>(elementType);
			if (structType->getName().startswith("struct.CB_ArrayData")) {
				return generateArrayValueType(structType);
			}
		}
	}

	return 0;
}

ValueType *ValueTypeCollection::findNamedType(const QString &name) {
	return mNamedType.value(name.toLower(), 0);
}

ArrayValueType *ValueTypeCollection::arrayValueType(ValueType *baseValueType, int dimensions) {
	QMap<QPair<ValueType*, int> , ArrayValueType *>::Iterator i = mArrayMapping.find(QPair<ValueType*, int>(baseValueType, dimensions));
	if (i != mArrayMapping.end()) {
		return i.value();
	}
	llvm::StructType *genericArrayHeaderType = mRuntime->genericArrayValueType()->structType();
	llvm::Type *intT = llvm::IntegerType::get(mRuntime->module()->getContext(), mRuntime->dataLayout().getPointerSizeInBits());
	llvm::Type *dimType = llvm::ArrayType::get(intT, dimensions);

	llvm::StructType *arrayDataHeaderType = llvm::StructType::get(genericArrayHeaderType, dimType, dimType, 0);

	llvm::StructType *arrayType = llvm::StructType::get(arrayDataHeaderType, baseValueType->llvmType(), 0);
	ArrayValueType *valTy = new ArrayValueType(baseValueType, arrayType->getPointerTo(), dimensions);
	mArrayMapping[QPair<ValueType*, int>(baseValueType, dimensions)] = valTy;
	return valTy;
}

ValueType *ValueTypeCollection::constantValueType(ConstantValue::Type type) const {
	switch (type) {
		case ConstantValue::Byte:
			return mRuntime->byteValueType();
		case ConstantValue::Short:
			return mRuntime->shortValueType();
		case ConstantValue::Integer:
			return mRuntime->intValueType();
		case ConstantValue::Float:
			return mRuntime->floatValueType();
		case ConstantValue::Boolean:
			return mRuntime->booleanValueType();
		case ConstantValue::String:
			return mRuntime->stringValueType();
		case ConstantValue::Null:
			return mRuntime->nullValueType();
		default:
			assert("Invalid ConstantValue::Type" && 0);
			return 0;
	}
}

QList<ValueType *> ValueTypeCollection::namedTypes() const {
	return mNamedType.values();
}

ValueType *ValueTypeCollection::generateArrayValueType(llvm::StructType *arrayDataType) {
	assert(arrayDataType->getStructNumElements() == 2);

	llvm::Type *element1 = arrayDataType->getStructElementType(0);
	llvm::Type *element2 = arrayDataType->getStructElementType(1);
	ValueType *baseType = valueTypeForLLVMType(element2);
	if (!baseType) {
		return 0;
	}

	llvm::StructType *arrayHeaderType = llvm::cast<llvm::StructType>(element1);
	assert(arrayHeaderType->getStructNumElements() == 3);

	assert(llvm::cast<llvm::StructType>(arrayHeaderType->getStructElementType(0))->getName() == "struct.CB_GenericArrayDataHeader");
	llvm::ArrayType *dimSizes = llvm::cast<llvm::ArrayType>(arrayHeaderType->getStructElementType(1));
	llvm::ArrayType *mults = llvm::cast<llvm::ArrayType>(arrayHeaderType->getStructElementType(2));
	assert(dimSizes->getArrayNumElements() == mults->getArrayNumElements());
	int dims = dimSizes->getArrayNumElements();

	ArrayValueType *arrayValueType = new ArrayValueType(baseType, arrayDataType->getPointerTo(), dims);
	mLLVMTypeMapping[arrayDataType->getPointerTo()] = arrayValueType;
}
