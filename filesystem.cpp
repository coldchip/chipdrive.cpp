#include "filesystem.h"

FileSystem::FileSystem() {

}

bool FileSystem::IsDir(string id, mutex *lock_) {
	std::unique_lock<std::mutex> lock(*lock_);

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

vector<Object> FileSystem::List(string id, mutex *lock_) {
	std::unique_lock<std::mutex> lock(*lock_);

	mutex dummy_lock;
	if(FileSystem::IsDir(id, &dummy_lock)) {
		vector<Object> rootlist = FileSystem::Load();
		vector<Object> list;
		for(auto t = rootlist.begin(); t != rootlist.end(); ++t) {
			if(t->parent.compare(id) == 0) {
				list.push_back(*t);
			}
		}
		return list;
	} else {
		throw FileSystemException("Folder Does Not Exist");
	}
}

Object FileSystem::CreateFile(string name, string parent, mutex *lock_) {
	std::unique_lock<std::mutex> lock(*lock_);

	mutex dummy_lock;
	if(FileSystem::IsDir(parent, &dummy_lock)) {
		if(name.length() < ENTRY_SIZE) {
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
			throw FileSystemException("Object Name is Too Long, Max: " + to_string(ENTRY_SIZE));
		}
	} else {
		throw FileSystemException("Folder Does Not Exist");
	}
}

Object FileSystem::CreateFolder(string name, string parent, mutex *lock_) {
	std::unique_lock<std::mutex> lock(*lock_);

	mutex dummy_lock;
	if(FileSystem::IsDir(parent, &dummy_lock)) {
		if(name.length() < ENTRY_SIZE) {
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
			throw FileSystemException("Object Name is Too Long, Max: " + to_string(ENTRY_SIZE));
		}
	} else {
		throw FileSystemException("Folder Does Not Exist");
	}
}

Object FileSystem::Rename(string name, string id, mutex *lock_) {
	std::unique_lock<std::mutex> lock(*lock_);

	vector<Object> rootlist = FileSystem::Load();
	for(auto &t : rootlist) {
		if(t.id == id) {
			t.name = name;
			FileSystem::Save(rootlist);
			return t;
		}
	}
	throw FileSystemException("Unable to Find Object");
}

Object FileSystem::Delete(string id, mutex *lock_) {
	std::unique_lock<std::mutex> lock(*lock_);

	vector<Object> rootlist = FileSystem::Load();

	auto t = rootlist.begin();

	while(t != rootlist.end()) {
		if(t->id == id) {
			rootlist.erase(t);
			FileSystem::Save(rootlist);
			return *t;
		} else {
			t++;
		}
	}
	throw FileSystemException("Unable to Find Object");
}

Object FileSystem::GetByID(string id, mutex *lock_) {
	std::unique_lock<std::mutex> lock(*lock_);

	vector<Object> rootlist = FileSystem::Load();
	for(auto t = rootlist.begin(); t != rootlist.end(); ++t) {
		if(t->id.compare(id) == 0) {
			return *t;
		}
	}
	throw FileSystemException("Object Not Found");
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
	FileStream fs;
	if(fs.open(path, "rb")) {
		while(!fs.eof()) {
			Object o;

			if(fs.read((char*)&o.type, sizeof(int)) == sizeof(int)) {
				int parent_size;
				fs.read((char*)&parent_size, sizeof(int));
				char parent[parent_size + 1];
				if(fs.read((char*)&parent, parent_size) == parent_size) {
					parent[parent_size] = '\0';
					o.parent = string(parent);

					int name_size;
					fs.read((char*)&name_size, sizeof(int));
					char name[name_size + 1];
					if(fs.read((char*)&name, name_size) == name_size) {
						name[name_size] = '\0';
						o.name = string(name);

						int id_size;
						fs.read((char*)&id_size, sizeof(int));
						char id[id_size + 1];
						if(fs.read((char*)&id, id_size) == id_size) {
							id[id_size] = '\0';
							o.id = string(id);
							
							list.push_back(o);
							
						}
					}
				}
			}

		}
		fs.close();
	}
	return list;
}

void FileSystem::Save(vector<Object> list) {
	string path = "./objects/node.bin";
	FileStream fs;
	if(fs.open(path, "wb")) {
		Node n;
		for(auto t = list.begin(); t != list.end(); ++t) {
			memset(&n, 0, sizeof(Node));
			n.type   = t->type;
			int type = (int)(t->type);
			char *parent = (char*)(t->parent.c_str());
			char *name = (char*)(t->name.c_str());
			char *id = (char*)(t->id.c_str());

			int parent_size = t->parent.length();
			int name_size = t->name.length();
			int id_size = t->id.length();

			fs.write((char*)&type, sizeof(type));

			fs.write((char*)&parent_size, sizeof(parent_size));
			fs.write(parent, parent_size);

			fs.write((char*)&name_size, sizeof(name_size));
			fs.write(name, name_size);

			fs.write((char*)&id_size, sizeof(id_size));
			fs.write(id, id_size);
		}
		fs.close();
	}
}

FileSystem::~FileSystem() {
	
}