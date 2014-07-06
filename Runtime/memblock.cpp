#include "memblock.h"

Memblock::Memblock(int size) :
	mData(new char[size]){
}

Memblock::~Memblock() {
	delete[] mData;
}

int Memblock::peekInt(int index) const {
	return *reinterpret_cast<int*>(mData + index);
}

float Memblock::peekFloat(int index) const {
	return *reinterpret_cast<float*>(mData + index);
}

uint16_t Memblock::peekShort(int index) const
{
	return *reinterpret_cast<uint16_t*>(mData + index);
}

uint8_t Memblock::peekByte(int index) const{
	return *reinterpret_cast<uint8_t*>(mData + index);
}

void Memblock::pokeInt(int index, int value) {
	*reinterpret_cast<int*>(mData + index) = value;
}

void Memblock::pokeFloat(int index, float value) {
	*reinterpret_cast<float*>(mData + index) = value;
}

void Memblock::pokeShort(int index, uint16_t value) {
	*reinterpret_cast<uint16_t*>(mData + index) = value;
}

void Memblock::pokeByte(int index, uint8_t value) {
	*reinterpret_cast<uint8_t*>(mData + index) = value;
}
