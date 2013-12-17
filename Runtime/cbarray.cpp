#include "cbarray.h"
#include <cstddef>
#include <array>
#include <cstdint>
#include <cstring>

void *CB_createArray(int dimCount, ArraySizeType *arrSizes, ArraySizeType dataTypeSize) {
	size_t size = CB_GenericArrayDataHeader::sizeOfCompleteHeader(dimCount) + dataTypeSize;
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
	return arrData;
}

void CB_refArray(CB_GenericArrayDataHeader *arr) {
	if (arr) atomicIncrease(arr->mRefCounter);
}

void CB_destructArray(CB_GenericArrayDataHeader *arr) {
	if (arr && atomicDecrease(arr->mRefCounter)) {
		atomicThreadFenceAcquire();
		delete[] reinterpret_cast<char*>(arr);
	}
}

void CB_assignArray(CB_GenericArrayDataHeader **target, CB_GenericArrayDataHeader *source) {
	CB_refArray(source); //increase reference counter
	CB_destructArray(*target); //decrease reference counter
	*target = source;
}





