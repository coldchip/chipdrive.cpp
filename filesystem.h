#ifndef FILESYSTEM
#define FILESYSTEM value

#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <cstring>
#include "filestream.h"

using namespace std;

#define ENTRY_SIZE 256

typedef struct _Node {
	int type;
	char parent[ENTRY_SIZE];
	char name[ENTRY_SIZE];
	char id[ENTRY_SIZE];
} Node;

typedef struct _Object {
	int type;
	string parent;
	string name;
	string id;
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
		static bool IsDir(string id);
		static vector<Object> List(string id);
		static Object CreateFile(string name, string parent);
		static Object CreateFolder(string name, string parent);
		static Object GetByID(string id);
		~FileSystem();
	private:
		static string Random(int len);
		static vector<Object> Load();
		static void Save(vector<Object> list);
};

#endif