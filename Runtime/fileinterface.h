#ifndef FILEINTERFACE_H
#define FILEINTERFACE_H
#include "common.h"
#include <cstdio>
#include <allegro5/file.h>

namespace fileinterface {
	typedef ALLEGRO_FILE File;
	void closeFile(File *f);
	void seekFile(File *f, int pos);
	void startSearch();
	void endSearch();
	void chDir(const LString &dir);
	void makeDir(const LString &dir);
	void copyFile(const LString &src, const LString &dest);
	void deleteFile(const LString &file);
	void execute(const LString &f);
	void writeByte(File *file, uint8_t v);
	void writeShort(File *file, uint16_t v);
	void writeInt(File *file, int v);
	void writeFloat(File *file, float v);
	void writeString(File *file, const LString &v);
	void writeLine(File *file, const LString &v);
	uint8_t readByte(File *file);
	uint16_t readShort(File *file);
	int readInt(File *file);
	float readFloat(File *file);
	LString readString(File *file);
	LString readLine(File *file);

	File *openToRead(const LString &path);
	File *openToWrite(const LString &path);
	File *openToEdit(const LString &path);
	int fileOffset(File *f);
	LString findFile();
	LString currentDir(void);
	bool fileExists(const LString &path);
	bool isDirectory(const LString &path);
	int fileSize(const LString &path);
	bool eof(File *f);

}

#endif // FILEINTERFACE_H
