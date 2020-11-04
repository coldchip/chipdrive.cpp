#include "filesystem.h"

FileSystem::FileSystem() {

}

bool FileSystem::IsDir(string id) {
	if(id.compare("root") == 0) {
		return true;
	}

	vector<Object> rootlist = FileSystem::Load();
	for(auto t = rootlist.begin(); t != rootlist.end(); ++t) {
		if(t->id.compare(id) == 0) {
			if(t->type == 2) {
				return true;
			}
		}
	}
	return false;
}

vector<Object> FileSystem::List(string id) {
	if(FileSystem::IsDir(id)) {
		vector<Object> rootlist = FileSystem::Load();
		vector<Object> list;
		for(auto t = rootlist.begin(); t != rootlist.end(); ++t) {
			if(t->parent.compare(id) == 0) {
				list.push_back(*t);
			}
		}
		return list;
	} else {
		throw FileSystemException();
	}
}

Object FileSystem::CreateFile(string name, string parent) {
	if(FileSystem::IsDir(parent)) {
		vector<Object> rootlist = FileSystem::Load();
		Object o;
		o.type   = 1;
		o.parent = parent;
		o.name   = name;
		o.id     = FileSystem::Random(64);
		rootlist.push_back(o);
		FileSystem::Save(rootlist);
		return o;
	} else {
		throw FileSystemException();
	}
}

Object FileSystem::CreateFolder(string name, string parent) {
	if(FileSystem::IsDir(parent)) {
		vector<Object> rootlist = FileSystem::Load();
		Object o;
		o.type   = 2;
		o.parent = parent;
		o.name   = name;
		o.id     = FileSystem::Random(64);
		rootlist.push_back(o);
		FileSystem::Save(rootlist);
		return o;
	} else {
		throw FileSystemException();
	}
}

string FileSystem::Random(int len) {
	string tmp_s;
	static const char alphanum[] =
	"0123456789"
	"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
	"abcdefghijklmnopqrstuvwxyz";

	for (int i = 0; i < len; ++i) {
		tmp_s += alphanum[rand() % (sizeof(alphanum) - 1)];
	}
	return tmp_s;
}

vector<Object> FileSystem::Load() {
	vector<Object> list;

	string path = "./objects/node.bin";
	ifstream file(path, ifstream::binary);
	if(file) {
		Node n;
		while(!file.eof()) {
			file.read((char*)&n, sizeof(Node));
			if(file.gcount() == sizeof(Node)) {
				Object o;
				o.type   = n.type;
				o.parent = string(n.parent);
				o.name   = string(n.name);
				o.id     = string(n.id);
				list.push_back(o);
			}
		}
		file.close();
	}
	return list;
}

void FileSystem::Save(vector<Object> list) {
	string path = "./objects/node.bin";
	ofstream file(path, ofstream::binary);
	if(file) {
		Node n;
		for(auto t = list.begin(); t != list.end(); ++t) {
			memset(&n, 0, sizeof(Node));
			n.type   = t->type;
			strcpy(n.parent, t->parent.c_str());
			strcpy(n.name, t->name.c_str());
			strcpy(n.id, t->id.c_str());
			file.write((char*)&n, sizeof(Node));
		}
		file.close();
	}
}

FileSystem::~FileSystem() {
	
}