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

void CBF_chDir(CBString dir) {
	chDir(dir);
}

void CBF_makeDir(CBString dir) {
	makeDir(dir);
}

void CBF_copyFile(CBString src, CBString dest) {
	copyFile(src, dest);
}

void CBF_deleteFile(CBString file) {
	deleteFile(file);
}

void CBF_execute(CBString f) {
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

void CBF_writeString(File *file, CBString v) {
	writeString(file, v);
}

void CBF_writeLine(File *file, CBString v) {
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

CBString CBF_readString(File *file) {
	return readString(file);
}

CBString CBF_readLine(File *file) {
	return readLine(file);
}

File *CBF_openToRead(CBString path) {
	return openToRead(path);
}

File *CBF_openToWrite(CBString path) {
	return openToWrite(path);
}

File *CBF_openToEdit(CBString path) {
	return openToEdit(path);
}

int CBF_fileOffset(File *f) {
	return fileOffset(f);
}

CBString CBF_findFile() {
	return findFile();
}

CBString CBF_currentDir(void) {
	return currentDir();
}

bool CBF_fileExists(CBString path) {
	return fileExists(path);
}

bool CBF_isDirectory(CBString path) {
	return isDirectory(path);
}

int CBF_fileSize(CBString path) {
	return fileSize(path);
}

bool CBF_eof(File *f) {
	return eof(f);
}
