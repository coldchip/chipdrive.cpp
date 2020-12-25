#ifndef FILESYSTEM
#define FILESYSTEM value

#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <cstring>
#include <mutex>

#include "filestream.h"

using namespace std;

#define ENTRY_SIZE 256
#define META_SIZE 8192

typedef struct _Node {
	int type;
	char parent[ENTRY_SIZE];
	char name[ENTRY_SIZE];
	char id[ENTRY_SIZE]; 
	char meta[ENTRY_SIZE]; 
} Node;

typedef struct _Object {
	int type;
	string parent;
	string name;
	string id;
	string meta;
} Object;

class FileSystemException : public exception {
	public:
		FileSystemException(string msg) {
			this->msg = msg;
		}
		const char *what() const throw() {
			return this->msg.c_str();
		}
	private:
		string msg;
};

class FileSystem {
	public:
		FileSystem();
		static bool IsDir(string id, mutex *lock_);
		static vector<Object> List(string id, mutex *lock_);
		static Object CreateFile(string name, string parent, mutex *lock_);
		static Object CreateFolder(string name, string parent, mutex *lock_);
		static Object Rename(string name, string id, mutex *lock_);
		static Object Delete(string id, mutex *lock_);
		static Object GetByID(string id, mutex *lock_);
		~FileSystem();
	private:
		static string Random(int len);
		static vector<Object> Load();
		static void Save(vector<Object> list);
};

#endif