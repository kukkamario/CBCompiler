#ifndef CBARRAY_H
#define CBARRAY_H
#include "atomicint.h"

typedef uint64_t ArraySizeType;
template <class T, size_t D>
struct  CB_ArrayData {
	ArraySizeType mFullSize;
	ArraySizeType mSizes[D];
	AtomicInt mRefCounter;
	T *mDummyIdStuff;
	int64_t mOffset;
	T *begin() {
		return reinterpret_cast<T*>(reinterpret_cast<char*>(this) + 4 * (1 /*mFullSize*/ + D /*mSizes */ + 1 /*mRefCounter*/ + 1 /*mOffset*/) + mOffset);
	}
	T *end() {
		return reinterpret_cast<T*>(reinterpret_cast<char*>(this) + 4 * (1 /*mFullSize*/ + D /*mSizes */ + 1 /*mRefCounter*/ + 1 /*mOffset*/) + mOffset + mFullSize * sizeof(T));
	}
};

template <class T, size_t D>
class Array {
		Array() : mData(0) { }

	public:
		Array(CB_ArrayData<T, D> *d) : mData(d) { }

		T *begin() { return mData->begin(); }


	private:

		CB_ArrayData<T, D> *mData;
};


template <size_t D>
inline size_t multDimArraySize(const int *arr) {
	return arr[D] * multDimArraySize<D - 1>(arr);
}

template <>
inline size_t multDimArraySize<0>(const int *arr) {
	return arr[0];
}

template <class T, size_t D>
Array<T, D> createArray(std::array<size_t, D> dims) {
	char *buffer = new char[sizeof(CB_ArrayData<T, D>) + multDimArraySize<D - 1>(dims) * sizeof(T)];
	CB_ArrayData<T, D> *arrData = static_cast<CB_ArrayData<T, D>*>(buffer);
	arrData->mOffset = 0;
	arrData->mSizes = dims;
	arrData->mRefCounter = 1;
	return CBArray<T, D>(arrData);
}






#endif // CBARRAY_H
