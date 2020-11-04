#ifndef FILESYSTEM
#define FILESYSTEM value

#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <cstring>

using namespace std;

typedef struct _Node {
	int type;
	char parent[1024];
	char name[1024];
	char id[1024];
} Node;

typedef struct _Object {
	int type;
	string parent;
	string name;
	string id;
} Object;

struct FileSystemException : public exception {
	const char *what() const throw() {
		return "FileSystemException";
	}
};

class FileSystem {
	public:
		FileSystem();
		static bool IsDir(string id);
		static vector<Object> List(string id);
		static Object CreateFile(string name, string parent);
		static Object CreateFolder(string name, string parent);
		~FileSystem();
	private:
		static string Random(int len);
		static vector<Object> Load();
		static void Save(vector<Object> list);
};

#endif