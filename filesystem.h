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
	char parent[256];
	char name[256];
	char id[256];
} Node;

typedef struct _Object {
	int type;
	string parent;
	string name;
	string id;
} Object;

class FileSystem {
	public:
		FileSystem();
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