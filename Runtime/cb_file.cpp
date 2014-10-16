#include "fileinterface.h"

using namespace fileinterface;

void CBF_closeFile(File *f) {
	closeFile(f);
}

void CBF_seekFile(File *f, int pos) {
	seekFile(f, pos);
}

void CBF_startSearch() {
	startSearch();
}

void CBF_endSearch() {
	endSearch();
}

void CBF_chDir(LString dir) {
	chDir(dir);
}

void CBF_makeDir(LString dir) {
	makeDir(dir);
}

void CBF_copyFile(LString src, LString dest) {
	copyFile(src, dest);
}

void CBF_deleteFile(LString file) {
	deleteFile(file);
}

void CBF_execute(LString f) {
	execute(f);
}

void CBF_writeByte(File *file, uint8_t v) {
	writeByte(file, v);
}

void CBF_writeShort(File *file, uint16_t v) {
	writeShort(file, v);
}

void CBF_writeInt(File *file, int v) {
	writeInt(file, v);
}

void CBF_writeFloat(File *file, float v) {
	writeFloat(file, v);
}

void CBF_writeString(File *file, LString v) {
	writeString(file, v);
}

void CBF_writeLine(File *file, LString v) {
	writeLine(file, v);
}

uint8_t CBF_readByte(File *file) {
	return readByte(file);
}

uint16_t CBF_readShort(File *file) {
	return readShort(file);
}

int CBF_readInt(File *file) {
	return readInt(file);
}

float CBF_readFloat(File *file) {
	return readFloat(file);
}

LString CBF_readString(File *file) {
	return readString(file);
}

LString CBF_readLine(File *file) {
	return readLine(file);
}

File *CBF_openToRead(LString path) {
	return openToRead(path);
}

File *CBF_openToWrite(LString path) {
	return openToWrite(path);
}

File *CBF_openToEdit(LString path) {
	return openToEdit(path);
}

int CBF_fileOffset(File *f) {
	return fileOffset(f);
}

LString CBF_findFile() {
	return findFile();
}

LString CBF_currentDir(void) {
	return currentDir();
}

bool CBF_fileExists(LString path) {
	return fileExists(path);
}

bool CBF_isDirectory(LString path) {
	return isDirectory(path);
}

int CBF_fileSize(LString path) {
	return fileSize(path);
}

bool CBF_eof(File *f) {
	return eof(f);
}
