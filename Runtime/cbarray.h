#ifndef CBARRAY_H
#define CBARRAY_H
#include "atomicint.h"
#include <cstdint>
#include <cstddef>
#include <array>
#include <cassert>
#include <cstring>

typedef uintptr_t ArraySizeType;
typedef intptr_t OffsetType;
struct CB_GenericArrayDataHeader {
	ArraySizeType mFullSize;
	ArraySizeType mSizeOfDataType;
	AtomicInt mRefCounter;
	ArraySizeType mDimensions;
	OffsetType mOffset;
	ArraySizeType *startOfSizes() {
		return reinterpret_cast<ArraySizeType*>(reinterpret_cast<char*>(this) + sizeof(CB_GenericArrayDataHeader));
	}
	ArraySizeType *startOfMults() {
		return reinterpret_cast<ArraySizeType*>(reinterpret_cast<char*>(this) + sizeof(CB_GenericArrayDataHeader)) + mDimensions;
	}
	size_t sizeOfCompleteHeader() {
		return sizeof(CB_GenericArrayDataHeader) + 2 * mDimensions * sizeof(ArraySizeType);
	}

	static size_t sizeOfCompleteHeader(size_t D) {
		return sizeof(CB_GenericArrayDataHeader) + 2 * D * sizeof(ArraySizeType);
	}
};

template <ArraySizeType D>
struct CB_ArrayDataHeader : CB_GenericArrayDataHeader {
	ArraySizeType mSizes[D];
	ArraySizeType mMults[D];
	static ArraySizeType headerSize() {
		return sizeof(CB_GenericArrayDataHeader) + sizeof(ArraySizeType) * (D /*mSizes */ + D /* mMults */ );
	}
};

template <class T, ArraySizeType D>
struct  CB_ArrayData : CB_ArrayDataHeader<D> {
	T mDummyIdStuff;
	T *begin() {
		return reinterpret_cast<T*>(reinterpret_cast<char*>(this) + this->mOffset);
	}
	T *end() {
		return begin() + this->mFullSize;
	}
	const T *begin() const {
		return reinterpret_cast<const T*>(reinterpret_cast<char*>(this) + this->mOffset);
	}
	const T *end() const {
		return begin() + this->mFullSize;
	}
};


namespace array_priv {
template <size_t D>
inline ArraySizeType multDimArraySize(const ArraySizeType *arr) {
	return arr[D] * multDimArraySize<D - 1>(arr);
}

template <>
inline ArraySizeType multDimArraySize<0>(const ArraySizeType *arr) {
	return arr[0];
}
}

template <class T, size_t D, size_t I>
class ArraySubscript {
	public:
		ArraySubscript(T *ptr, ArraySizeType *sizes) : mPtr(ptr), mSizes(sizes) { }
		typedef ArraySubscript<T, D, I - 1> RetType;
		RetType operator[] (ArraySizeType i) {
			assert(i >= 0 && i < mSizes[I]);
			return ArraySubscript<T, D, I - 1>(mPtr + array_priv::multDimArraySize<I - 1>(mSizes) * i, mSizes);
		}
	private:
		T *mPtr;
		ArraySizeType *mSizes;

};

template <class T, size_t D>
class ArraySubscript<T, D, 0> {
	public:
		typedef T& RetType;

		ArraySubscript(T *ptr, ArraySizeType *sizes) : mPtr(ptr), mSizes(sizes) { }

		RetType operator[] (ArraySizeType i) {
			assert(i >= 0 && i < mSizes[0]);
			return mPtr[i];
		}
	private:
		T *mPtr;
		ArraySizeType *mSizes;
};



template <class T, size_t D>
class CBArray {
	public:
		typedef T *Iterator;
		typedef const T *ConstIterator;
		typedef T *iterator;
		typedef T *const_iterator;

		CBArray() : mData(0) { }
		CBArray(std::array<ArraySizeType, D> dims);

		CBArray(CB_ArrayData<T, D> *d) : mData(d) { if (d) atomicIncrease(mData->mRefCounter); }
		CBArray(const CBArray &o) : mData(o.mData) { if (mData) atomicIncrease(mData->mRefCounter); }
		~CBArray();

		Iterator begin() { return mData->begin(); }
		Iterator end() { return mData->end(); }
		ConstIterator begin() const { return mData->begin(); }
		ConstIterator end() const { return mData->end(); }
		ConstIterator cbegin() const { return mData->begin(); }
		ConstIterator cend() const { return mData->end(); }

		ArraySizeType elements() { if (mData) return mData->mFullSize; return 0; }

		ArraySizeType dimensionSize(int dim) {
			if (!mData) return 0;
			assert(dim >= 0 && dim < mData->mDimensions);
			return mData->mSizes[dim];
		}

		CBArray &operator = (const CBArray &o) {
			if (o.mData) atomicIncrease(o.mData->mRefCounter);
			if (this->mData && atomicDecrease(o.mData->mRefCounter)) {
				atomicThreadFenceAcquire();
				delete[] reinterpret_cast<char*>(this->mData);
			}
			this->mData = o.mData;
		}

		typename ArraySubscript<T, D, D - 1>::RetType operator[] (int i) {
			return ArraySubscript<T, D, D - 1>(mData->begin(), mData->mSizes)[i];
		}

		typename ArraySubscript<const T, D, D - 1>::RetType operator[] (int i) const {
			return ArraySubscript<T, D, D - 1>(mData->begin(), mData->mSizes)[i];
		}


	private:

		CB_ArrayData<T, D> *mData;
};

template <class T, size_t D>
CBArray<T, D>::CBArray(std::array<ArraySizeType, D> dims) {
	ArraySizeType totalElements = array_priv::multDimArraySize<D - 1>(dims.data());
	size_t size = CB_ArrayDataHeader<D>::headerSize() + totalElements * sizeof(T);
	char *buffer = new char[size];
	memset(buffer, 0, size);
	CB_ArrayData<T, D> *arrData = reinterpret_cast<CB_ArrayData<T, D>*>(buffer);
	arrData->mDimensions = D;
	arrData->mFullSize = totalElements;
	ArraySizeType mult = 1;
	for (int i = D - 1; i >= 0; i--) {
		arrData->mSizes[i] = dims[i];
		arrData->mMults[i] = mult;
		mult *= dims[i];
	}
	arrData->mRefCounter = 1;
	arrData->mOffset = arrData->sizeOfCompleteHeader();
	mData = arrData;
}


template <class T, size_t D>
CBArray<T, D>::~CBArray() {
	if (mData && atomicDecrease(mData->mRefCounter)) {
		atomicThreadFenceAcquire();
		delete [] reinterpret_cast<char*>(mData);
	}
}


template <class T, typename... D>
CBArray<T, sizeof...(D)> createArray(D... dims) {
	return CBArray<T, sizeof...(D)>(std::array<ArraySizeType, sizeof...(D)>{ {dims...} });
}



template <class T, size_t D>
using Array = CBArray<T, D>;
template <class T, size_t D>
using CBArrayData = CB_ArrayData<T, D>*;


#endif // CBARRAY_H
