#include "memblock.h"


Memblock *CBF_makeMemblock(int size) {
	return new Memblock(size);
}

void CBF_deleteMemblock(Memblock *mem) {
	delete mem;
}

int CBF_peekInt(Memblock *mem, int index) {
	return mem->peekInt(index);
}

float CBF_peekFloat(Memblock *mem, int index) {
	return mem->peekFloat(index);
}

uint16_t CBF_peekShort(Memblock *mem, int index) {
	return mem->peekShort(index);
}


uint8_t CBF_peekByte(Memblock *mem, int index) {
	return mem->peekByte(index);
}

void CBF_pokeInt(Memblock *mem, int index, int value) {
	mem->pokeInt(index, value);
}

void CBF_pokeFloat(Memblock *mem, int index, float value) {
	mem->pokeFloat(index, value);
}


void CBF_pokeShort(Memblock *mem, int index, uint16_t value) {
	mem->pokeShort(index, value);
}

void CBF_pokeByte(Memblock *mem, int index, uint8_t value) {
	mem->pokeByte(index, value);
}
