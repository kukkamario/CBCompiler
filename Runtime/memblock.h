#ifndef MEMBLOCK_H
#define MEMBLOCK_H
#include "common.h"
class Memblock {
	public:
		Memblock(int size);
		~Memblock();
		int peekInt(int index) const;
		float peekFloat(int index) const;
		uint16_t peekShort(int index) const;
		uint8_t peekByte(int index) const;
		void pokeInt(int index, int value);
		void pokeFloat(int index, float value);
		void pokeShort(int index, uint16_t value);
		void pokeByte(int index, uint8_t value);
	private:
		char *mData;
};

#endif // MEMBLOCK_H
