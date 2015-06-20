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
#include "functionvaluetype.h"
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

void ValueTypeCollection::addValueType(const QString &name, ValueType *valType) {
	mLLVMTypeMapping[valType->llvmType()] = valType;
	mNamedType[name.toLower()] = valType;
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

	//Check if type is ArrayValueType or FunctionValueType
	if (type->isPointerTy()) {
		llvm::PointerType *ptrType = llvm::cast<llvm::PointerType>(type);
		llvm::Type *elementType = ptrType->getElementType();
		if (elementType->isStructTy()) {
			llvm::StructType *structType = llvm::cast<llvm::StructType>(elementType);
			if (structType->getName().startswith("struct.CB_ArrayData")) {
				return generateArrayValueType(structType);
			}
		} else if (elementType->isFunctionTy()) {
			llvm::FunctionType *ft = llvm::cast<llvm::FunctionType>(elementType);
			return generateFunctionValueType(ft);
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

	llvm::StructType *arrayDataHeaderType = llvm::StructType::get(genericArrayHeaderType, dimType, dimType, NULL);

	llvm::StructType *arrayType = llvm::StructType::get(arrayDataHeaderType, baseValueType->llvmType(), NULL);
	ArrayValueType *valTy = new ArrayValueType(baseValueType, arrayType->getPointerTo(), dimensions);
	mArrayMapping[QPair<ValueType*, int>(baseValueType, dimensions)] = valTy;
	mLLVMTypeMapping[valTy->llvmType()] = valTy;
	return valTy;
}

FunctionValueType *ValueTypeCollection::functionValueType(ValueType *returnType, const QList<ValueType *> &paramTypes) {
	FunctionType ft;
	ft.mReturnType = returnType;
	ft.mParamTypes = paramTypes;
	QMap<FunctionType, FunctionValueType*>::ConstIterator i = mFunctionTypeMapping.find(ft);

	if (i != mFunctionTypeMapping.end()) {
		return i.value();
	}

	FunctionValueType *fvt = new FunctionValueType(mRuntime, returnType, paramTypes);
	mFunctionTypeMapping[ft] = fvt;
	mLLVMTypeMapping[fvt->llvmType()] = fvt;
	return fvt;
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

const QMap<QString, ValueType *> &ValueTypeCollection::namedTypesMap() const {
	return mNamedType;
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
	mArrayMapping[QPair<ValueType*, int>(baseType, dims)] = arrayValueType;
	return arrayValueType;
}

ValueType *ValueTypeCollection::generateFunctionValueType(llvm::FunctionType *funcTy) {
	ValueType *retType = 0;
	if (!funcTy->getReturnType()->isVoidTy()) {
		retType = valueTypeForLLVMType(funcTy->getReturnType());
		if (!retType) return 0;
	}
	QList<ValueType*> paramTypes;
	for (llvm::FunctionType::param_iterator i = funcTy->param_begin(); i != funcTy->param_end(); i++) {
		ValueType *vt = valueTypeForLLVMType(*i);
		if (!vt) return 0;
		paramTypes.append(vt);
	}
	return functionValueType(retType, paramTypes);
}


bool ValueTypeCollection::FunctionType::operator <(const ValueTypeCollection::FunctionType &ft) const {
	if (this->mReturnType < ft.mReturnType) return true;
	if (this->mReturnType > ft.mReturnType) return false;
	if (this->mParamTypes.size() < ft.mParamTypes.size()) return true;
	if (this->mParamTypes.size() > ft.mParamTypes.size()) return false;
	QList<ValueType*>::ConstIterator i = ft.mParamTypes.begin();
	for (ValueType *p : this->mParamTypes) {
		if (p < *i) {
			return true;
		}
		if (p > *i) {
			return false;
		}
		i++;
	}
	return false;
}
