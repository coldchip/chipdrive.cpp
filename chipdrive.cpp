#include "chipdrive.h"

ChipDrive::ChipDrive(int port) {
	this->lock = new mutex();
	this->chiphttp = new ChipHttp(port);
	this->session = new Session();

	this->chiphttp->route([&](Request &request, Response &response) {
		this->Router(request, response);
	});
}

void ChipDrive::start()	{
	cout << "ChipDrive C++" << endl;
	cout << "Version: 1.0.0 beta 1" << endl;
	this->chiphttp->start();
}

bool ChipDrive::auth(string username, string password) {
	if(username.compare("a") == 0 && password.compare("a") == 0) {
		return true;
	}
	return false;
}

string ChipDrive::MakeJSON(bool success, string reason, json data) {
	json stub = {
		{ "success", success },
		{ "reason", reason },
		{ "data", data }
	};
	string res = stub.dump(4);
	return res;
}

void ChipDrive::Router(Request &request, Response &response) {
	try {
		this->lock->lock();
		cout << "#" + to_string(this->hits) + " Request: " << request.path << endl;
		this->hits++;
		this->lock->unlock();

		string token = request.GetCookie("token");
		bool validation = session->ValidateToken(token, this->lock);
		
		if(request.path.compare("/api/v1/login") == 0) {
			this->ServeLogin(request, response);
		} else if(request.path.compare("/api/v1/logout") == 0) {
			if(validation == true) {
				this->ServeLogout(request, response);
			} else {
				throw ChipDriveAuthException("Auth Required");
			}
		} else if(request.path.compare("/api/v1/drive/config") == 0) {
			if(validation == true) {
				this->ServeConfig(request, response);
			} else {
				throw ChipDriveAuthException("Auth Required");
			}
		} else if(request.path.compare("/api/v1/drive/list") == 0) {
			if(validation == true) {
				this->ServeList(request, response);
			} else {
				throw ChipDriveAuthException("Auth Required");
			}
		} else if(request.path.compare("/api/v1/drive/folder") == 0) {
			if(validation == true) {
				this->ServeCreateFolder(request, response);
			} else {
				throw ChipDriveAuthException("Auth Required");
			}
		} else if(request.path.compare("/api/v1/drive/rename") == 0) {
			if(validation == true) {
				this->ServeRename(request, response);
			} else {
				throw ChipDriveAuthException("Auth Required");
			}
		} else if(request.path.compare("/api/v1/drive/delete") == 0) {
			if(validation == true) {
				this->ServeDelete(request, response);
			} else {
				throw ChipDriveAuthException("Auth Required");
			}
		} else if(request.path.compare("/api/v1/drive/upload") == 0) {
			if(validation == true) {
				this->ServeUpload(request, response);
			} else {
				throw ChipDriveAuthException("Auth Required");
			}
		} else if(request.path.compare("/api/v1/drive/stream") == 0) {
			if(validation == true) {
				this->ServeStream(request, response);
			} else {
				throw ChipDriveAuthException("Auth Required");
			}
		} else {
			this->ServeRoot(request, response);
		}
	} catch(FileSystemException &e) {
		string raw = this->MakeJSON(false, e.what(), json());

		response.SetHeader("Content-Length", to_string(raw.size()));
		response.SetHeader("Content-Type", "application/json");
		response.write(raw);
	} catch(ChipDriveException &e) {
		string raw = this->MakeJSON(false, e.what(), json());

		response.SetHeader("Content-Length", to_string(raw.size()));
		response.SetHeader("Content-Type", "application/json");
		response.write(raw);
	} catch(ChipDriveAuthException &e) {
		string raw = this->MakeJSON(false, e.what(), json());

		response.SetStatus(401);
		response.SetHeader("Content-Length", to_string(raw.size()));
		response.SetHeader("Content-Type", "application/json");
		response.write(raw);
	}
}

char *ChipDrive::GetTarFile(string path, long long *size) {
	path = ChipHttp::CleanPath(path);

	extern char _binary_bin_tmp_rootdocs_tar_start[];
	extern char _binary_bin_tmp_rootdocs_tar_end[];

	char *root_start = _binary_bin_tmp_rootdocs_tar_start;
	char *root_end   = _binary_bin_tmp_rootdocs_tar_end;

    char *p = root_start;

    while(p <= root_end) {
    	TarHeader *header = (TarHeader*)p;

		long long blob_size = 0;
		int read = sscanf(header->size, "%llo", &blob_size);
		string name = string(header->name);
        if(path.compare(name) == 0 && path.length() > 0 && read == 1 && (header->typeflag & 0xFF) == 48) {
        	p += 512; // skip header
        	*size = blob_size;
        	return p;
        }

        p += (1 + blob_size / 512) * 512;
		if((blob_size % 512) > 0) {
			p += 512;
		}
    }
    return NULL;
}

void ChipDrive::ServeRoot(Request &request, Response &response) {
	string path = ChipHttp::CleanPath(request.path);

	long long size;
	char *p = this->GetTarFile((char*)path.c_str(), &size);

	if(!p) {
		path.append("/index.html");
		p = this->GetTarFile((char*)path.c_str(), &size);
	}

	string mime = ChipHttp::GetMIME(path);

	if(p) {
		response.SetHeader("Content-Type", mime);
		response.SetHeader("Content-Length", to_string(size));
		int chunk_size = 8192;
		char *p_start = p;
		char *p_end   = p + size;
		while(p_start < p_end) {
			int to_write = min((long long)chunk_size, (long long)(p_end - p_start));
			int written = response.write(p_start, to_write);
			p_start += written;
		}
	} else {
		string data = "404 Not Found";

		response.SetHeader("Content-Length", to_string(data.size()));
		response.SetHeader("Content-Type", "text/html");
		response.write(data);
	}
}

void ChipDrive::ServeLogin(Request &request, Response &response) {
	string username = request.GetQuery("username");
	string password = request.GetQuery("password");

	if(this->auth(username, password) == true) {
		string raw = this->MakeJSON(true, "", json());

		string token = session->GenerateToken(this->lock);

		response.SetCookie("token", token);
		response.SetHeader("Content-Length", to_string(raw.size()));
		response.SetHeader("Content-Type", "application/json");
		response.write(raw);
	} else {
		throw ChipDriveException("Invalid Username or Password");
	}
}

void ChipDrive::ServeLogout(Request &request, Response &response) {
	string raw = this->MakeJSON(true, "", json());

	response.SetHeader("Content-Length", to_string(raw.size()));
	response.SetHeader("Content-Type", "application/json");
	response.write(raw);
}

void ChipDrive::ServeConfig(Request &request, Response &response) {
	json config = {
		{ "root", "root" },
	};
	string raw = this->MakeJSON(true, "", config);

	response.SetHeader("Content-Length", to_string(raw.size()));
	response.SetHeader("Content-Type", "application/json");
	response.write(raw);
}

void ChipDrive::ServeList(Request &request, Response &response) {
	string folderid = request.GetQuery("folderid");

	if(folderid.length() > 0) {
		vector<Object> list = FileSystem::List(folderid, this->lock);

		json config;
		config["list"] = json::array();
		for(auto t = list.begin(); t != list.end(); ++t) {
			json item = {
				{ "type", t->type },
				{ "name", t->name },
				{ "id", t->id }
			};
			config["list"].push_back(item);
		}
		string raw = this->MakeJSON(true, "", config);

		response.SetHeader("Content-Length", to_string(raw.size()));
		response.SetHeader("Content-Type", "application/json");
		response.write(raw);
	} else {
		throw ChipDriveException("Params Not Satisfiable");
	}
}

void ChipDrive::ServeCreateFolder(Request &request, Response &response) {
	string folderid = request.GetQuery("folderid");
	string name = request.GetQuery("name");

	if(folderid.length() > 0 && name.length() > 0) {
		FileSystem::CreateFolder(name, folderid, this->lock);

		string raw = this->MakeJSON(true, "", json());

		response.SetHeader("Content-Length", to_string(raw.size()));
		response.SetHeader("Content-Type", "application/json");
		response.write(raw);
	} else {
		throw ChipDriveException("Params Not Satisfiable");
	}
}

void ChipDrive::ServeRename(Request &request, Response &response) {
	string itemid = request.GetQuery("itemid");
	string name = request.GetQuery("name");

	if(itemid.length() > 0 && name.length() > 0) {
		FileSystem::Rename(name, itemid, this->lock);

		string raw = this->MakeJSON(true, "", json());

		response.SetHeader("Content-Length", to_string(raw.size()));
		response.SetHeader("Content-Type", "application/json");
		response.write(raw);
	} else {
		throw ChipDriveException("Params Not Satisfiable");
	}
}

void ChipDrive::ServeDelete(Request &request, Response &response) {
	string itemid = request.GetQuery("itemid");

	if(itemid.length() > 0) {
		Object o = FileSystem::Delete(itemid, this->lock);

		string raw = this->MakeJSON(true, "", json());

		response.SetHeader("Content-Length", to_string(raw.size()));
		response.SetHeader("Content-Type", "application/json");
		response.write(raw);
	} else {
		throw ChipDriveException("Params Not Satisfiable");
	}
}

void ChipDrive::ServeUpload(Request &request, Response &response) {
	string folderid = request.GetQuery("folderid");
	string name     = request.GetQuery("name");
	int length      = atoi(request.GetHeader("Content-Length").c_str());

	if(folderid.length() > 0 && name.length() > 0 && length > 0) {
		Object o = FileSystem::CreateFile(name, folderid, this->lock);

		FileStream fs;

		if(fs.open("objects/" + o.id, "wb")) {
			long long i = 0;
			long long read = 0;
			char buf[8192];
			while(i < length) {
				read = request.read(buf, sizeof(buf));
				if(fs.write(buf, read) != read) {
					throw FileSystemException("Unable to Write to Object Further");
				}
				i += read;
			}
		}

		string raw = this->MakeJSON(true, "", json());

		response.SetHeader("Content-Length", to_string(raw.size()));
		response.SetHeader("Content-Type", "application/json");
		response.write(raw);
	} else {
		throw ChipDriveException("Params Not Satisfiable");
	}
}

void ChipDrive::ServeStream(Request &request, Response &response) {
	string id = request.GetQuery("id");

	if(id.length() > 0) {
		Object o = FileSystem::GetByID(id, this->lock);

		string path = "./objects/" + id;
		FileStream fs;
		if(fs.open(path, "rb") == true) {
			long long size = fs.size();

			long long start = 0;
			long long end = size - 1;

			string range = request.GetHeader("Range");
			if(range.length() > 0) {
				sscanf(range.c_str(), "bytes=%llu-%llu", &start, &end);
				response.SetHeader("Content-Range", "bytes " + to_string(start) + "-" + to_string(end) + "/" + to_string(size));
				response.SetStatus(206);
			} else {
				response.SetStatus(200);
			}

			if((start < 0 || start > size) || (end <= 0 || end > size)) {
				throw ChipDriveException("Out of Range");
			}

			fs.seek(start, SEEK_SET);

			string mime = ChipHttp::GetMIME(o.name);
			response.SetHeader("Accept-Ranges", "bytes");
			response.SetHeader("Content-Type", mime);
			response.SetHeader("Content-Length", to_string((end - start) + 1));

			char buf[8192 * 4];
			long long read = 0;
			for(long long p = start; p < end; p += sizeof(buf)) {
				long long to_read = min((int)sizeof(buf), (int)(end - p) + 1);
				read = fs.read(buf, to_read);
				if(read != to_read) {
					throw FileSystemException("Unable to Read Object");
				}
				response.write(buf, read);
			}
			fs.close();
		} else {
			throw ChipDriveException("Object Not Found");
		}
	} else {
		throw ChipDriveException("Params Not Satisfiable");
	}
}

ChipDrive::~ChipDrive() {
	delete this->session;
	delete this->lock;
	delete this->chiphttp;
}