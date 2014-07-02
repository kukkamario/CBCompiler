#include "fileinterface.h"
#include "error.h"
#include <allegro5/file.h>
#include <allegro5/path.h>

namespace fileinterface {
static int findFileCount = 0;
static ALLEGRO_FS_ENTRY *cur_dir = 0;

void closeFile(File *f) {
	al_fclose(f);
}

void seekFile(File *f, int pos) {
	al_fseek(f, pos, ALLEGRO_SEEK_SET);
}

void startSearch(void) {
	std::string dir_str = std::string(al_get_current_directory());
	cur_dir = al_create_fs_entry(dir_str.c_str());
	if(!al_open_directory(cur_dir))
		error("StartSearch failed! Path: \"" + dir_str + "\"");

	findFileCount = 0;
}

void endSearch(void) {
	al_close_directory(cur_dir);
	al_destroy_fs_entry(cur_dir);
}

void chDir(const LString &path) {
	const std::string &path_s = path.toUtf8();
	if(!al_change_directory(path_s.c_str()))
		error("ChDir failed! Path: \"" + path_s + "\"");
}

void makeDir(const LString &path) {
	const std::string &dir_s = path.toUtf8();
	if(!al_make_directory(dir_s.c_str()))
		error("MakeDir failed! Directory: \"" + dir_s + "\"");
}

void copyFile(const LString &src, const LString &dest) {
	const std::string &file_s2 = dest.toUtf8();
	const std::string &file_s1 = src.toUtf8();

	if(al_filename_exists(file_s2.c_str())) {
		error("CopyFile failed! File \"" + file_s2 + "\" already exists!");
		return;
	}

	uint32_t size = 0;
	char * buffer;

	File * file1 = al_fopen(file_s1.c_str(), "rb");
	if(file1 == NULL) {
		error("CopyFile failed! Can't open file \"" + file_s1 + "\"");
		return;
	}

	al_fseek(file1, 0, ALLEGRO_SEEK_END);
	size = al_ftell(file1) * sizeof(char);
	al_fseek(file1, 0, ALLEGRO_SEEK_SET);

	buffer = new (std::nothrow)char[size];
	if(buffer == NULL) {
		error("CopyFile failed! Memory error!");
		return;
	}

	if(al_fread(file1, buffer, size) != size) {
		error("CopyFile failed!");
		return;
	}

	al_fclose(file1);

	File * file2 = al_fopen(file_s2.c_str(), "wb");
	if(file2 == NULL) {
		error("CopyFile failed! Can't open file \"" + file_s2 + "\"");
		return;
	}
	al_fwrite(file2, buffer, size);
	al_fclose(file2);

	delete[] buffer;
}

void deleteFile(const LString &s) {
	const std::string &file_s = s.toUtf8();
	if(!al_remove_filename(file_s.c_str()))
		error("DeleteFile failed! File: \"" + file_s + "\"");
}

void execute(const LString &e) {
	const std::string &cmd = e.toUtf8();
	std::string scmd;

	#ifdef WIN32
		scmd = "start " + cmd;
	#else
		scmd = "xdg-open " + cmd;
	#endif

	system(scmd.c_str());
}

void writeByte(File *f, uint8_t v) {
	al_fputc(f,(int) v);
}

void writeShort(File *f, uint16_t v) {
	al_fwrite(f, &v, sizeof(uint16_t));
}

void writeInt(File *f, int32_t v) {
	al_fwrite(f, &v, sizeof(int32_t));
}

void writeFloat(File *f, float v) {
	al_fwrite(f, &v, sizeof(float));
}

void writeString(File *f, const LString &s) {
	int l = int(s.length());

	al_fwrite(f, &l, sizeof(int));
	al_fwrite(f, s.begin(), sizeof(char32_t));
}

void writeLine(File *f, const LString &s) {
	const std::string &line = s.toUtf8();
	al_fputs(f, line.c_str());
	#ifdef WIN32
		al_fputs(f, "\r\n");
	#else
		al_fputs(f, "\n");
	#endif


}


File *openToRead(const LString &s) {
	const std::string &file = s.toUtf8();

	File *f = al_fopen(file.c_str(), "rb");
	if (f == NULL) {
		error("OpenToRead failed! File: \"" + file + "\"");
		return 0;
	}

	return f;
}

File *openToWrite(const LString &s) {
	const std::string &file = s.toUtf8();

	File *f = al_fopen(file.c_str(), "wb");
	if (f == NULL) {
		error("OpenToWrite failed! File: \"" + file + "\"");
		return 0;
	}

	return f;
}

File *openToEdit(const LString &s) {
	const std::string &file = s.toUtf8();

	File *f = 0;
	if(al_filename_exists(file.c_str())) {
		f = al_fopen(file.c_str(), "rb+");
	}
	else {
		f = al_fopen(file.c_str(), "wb+");
	}
	if (f == NULL) {
		error("OpenToEdit failed! File: \"" + file + "\"");
		return 0;
	}

	return f;
}


int fileOffset(File *f) {
	return int32_t(al_ftell(f));
}

LString findFile(void) {
	++findFileCount;
	if (findFileCount == 1) {
		ALLEGRO_PATH * path;
		path = al_create_path(al_get_fs_entry_name(cur_dir));
		if(al_get_path_num_components(path) >= 2) {
			al_destroy_path(path);
			return LString(U".");
		}
		else {
			al_destroy_path(path);
			return findFile();
		}
	}
	else if (findFileCount == 2) {
		ALLEGRO_PATH * path;
		path = al_create_path(al_get_fs_entry_name(cur_dir));
		if(al_get_path_num_components(path) >= 2) {
			al_destroy_path(path);
			return LString(U"..");
		}
		else {
			al_destroy_path(path);
			return findFile();
		}
	}

	ALLEGRO_FS_ENTRY * file = al_read_directory(cur_dir);

	if(file == NULL) {
		return LString();
	}

	std::string file_s;

	ALLEGRO_PATH * path = NULL;

	if(al_get_fs_entry_mode(file) & ALLEGRO_FILEMODE_ISDIR) {
		path = al_create_path_for_directory(al_get_fs_entry_name(file));
		file_s = std::string(al_get_path_tail(path));
	}
	else {
		path = al_create_path(al_get_fs_entry_name(file));
		file_s = std::string(al_get_path_filename(path));
	}

	al_destroy_path(path);
	al_destroy_fs_entry(file);

	return LString::fromUtf8(file_s);
}

LString currentDir(void) {
	char * dir = al_get_current_directory();

	std::string dir_s = std::string(dir) + "\\";
	al_free(dir);

	return LString::fromUtf8(dir_s);
}

bool fileExists(const LString &f) {
	ALLEGRO_PATH *filePath = al_create_path(f.toUtf8().c_str());
	bool e = al_filename_exists(al_path_cstr(filePath, ALLEGRO_NATIVE_PATH_SEP));
	al_destroy_path(filePath);
	return e;
}

bool isDirectory(const LString &path) {
	ALLEGRO_PATH *filePath = al_create_path(path.toUtf8().c_str());
	ALLEGRO_FS_ENTRY * file = al_create_fs_entry(al_path_cstr(filePath, ALLEGRO_NATIVE_PATH_SEP));
	bool s = bool(al_get_fs_entry_mode(file) & ALLEGRO_FILEMODE_ISDIR);
	al_destroy_fs_entry(file);
	al_destroy_path(filePath);
	return s;
}

int fileSize(const LString &path) {
	ALLEGRO_FS_ENTRY * file = al_create_fs_entry(path.toUtf8().c_str());
	int32_t i = int32_t(al_get_fs_entry_size(file));
	al_destroy_fs_entry(file);
	return i;
}

bool eof(File *f) {
	al_fgetc(f);
	bool e = al_feof(f) != 0;
	al_fseek(f, -1, ALLEGRO_SEEK_CUR);
	return e;
}

uint8_t readByte(File *f) {
	return uint8_t(al_fgetc(f));
}

uint16_t readShort(File *f) {
	uint16_t sh;

	al_fread(f, &sh, sizeof(uint16_t));

	return sh;
}

int readInt(File *f) {
	int32_t i;

	al_fread(f, &i, sizeof(int32_t));

	return i;
}

float readFloat(File *f) {
	float fl;

	al_fread(f, &fl, sizeof(float));

	return fl;
}

LString readString(File *f) {
	int32_t l;
	al_fread(f, &l, sizeof(int32_t));

	LString str;
	str.resize(l);
	al_fread(f, str.begin(), sizeof(char32_t));

	return str;
}

LString readLine(File *f) {
	std::string line;
	while(1) {
		int c = al_fgetc(f);
		if (c != '\r' && c != EOF && c != '\n') {
			line = line + char(c);
		}
		else {
			break;
		}
	}
	return LString::fromUtf8(line);
}


}

