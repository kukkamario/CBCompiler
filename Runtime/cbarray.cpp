#include "cbarray.h"
#include <cstddef>
#include <array>
#include <cstdint>
#include <cstring>
#include "common.h"
#include <cstdio>

CBEXPORT CB_GenericArrayDataHeader *CB_ArrayConstruct(ArraySizeType dimCount, ArraySizeType *arrSizes, ArraySizeType dataTypeSize) {
	ArraySizeType elements = 1;
	for (ArraySizeType i = 0; i < dimCount; i++) {
		elements *= arrSizes[i];
	}
	size_t size = CB_GenericArrayDataHeader::sizeOfCompleteHeader(dimCount) + dataTypeSize * elements;
	char *buffer = new char[size];
	memset(buffer, 0, size);
	CB_GenericArrayDataHeader *arrData = reinterpret_cast<CB_GenericArrayDataHeader*>(buffer);
	arrData->mDimensions = dimCount;
	ArraySizeType mult = 1;
	ArraySizeType *sizes = arrData->startOfSizes();
	ArraySizeType *mults = arrData->startOfMults();
	for (int i = dimCount - 1; i >= 0; i--) {
		sizes[i] = arrSizes[i];
		mults[i] = mult;
		mult *= arrSizes[i];
	}
	arrData->mSizeOfDataType = dataTypeSize;
	arrData->mRefCounter = 1;
	arrData->mOffset = arrData->sizeOfCompleteHeader();
	arrData->mFullSize = elements;
	return reinterpret_cast<CB_GenericArrayDataHeader*>(arrData);
}

CBEXPORT void CB_ArrayRef(CB_GenericArrayDataHeader *arr) {
	if (arr) atomicIncrease(arr->mRefCounter);
}

CBEXPORT void CB_ArrayDestruct(CB_GenericArrayDataHeader *arr) {
	if (arr && atomicDecrease(arr->mRefCounter)) {
		atomicThreadFenceAcquire();
		delete[] reinterpret_cast<char*>(arr);
	}
}

CBEXPORT void CB_ArrayAssign(CB_GenericArrayDataHeader **target, CB_GenericArrayDataHeader *source) {
	CB_ArrayRef(source); //increase reference counter
	CB_ArrayRef(*target); //decrease reference counter
	*target = source;
}





