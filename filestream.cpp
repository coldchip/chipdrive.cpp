#include "filestream.h"

FileStream::FileStream() {

}

bool FileStream::open(string file, string mode) {
	this->file = fopen(file.c_str(), mode.c_str());
	if(this->file) {
		return true;
	}
	return false;
}

long int FileStream::tell() {
	return ftell(this->file);
}

int FileStream::seek(long int offset, int whence) {
	return fseek(this->file, offset, whence);
}

int FileStream::read(char *buf, int length) {
	return fread(buf, sizeof(char), length, this->file);
}

int FileStream::write(char *buf, int length) {
	return fwrite(buf, sizeof(char), length, this->file);
}

void FileStream::close() {
	if(this->file) {
		fclose(this->file);
		this->file = NULL;
	}
}

FileStream::~FileStream() {
	this->close();
}