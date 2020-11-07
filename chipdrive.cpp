#include "chipdrive.h"

ChipDrive::ChipDrive(int port) {
	this->lock = new mutex();
	this->chiphttp = new ChipHttp(port);

	this->chiphttp->route([&](Request &request, Response &response) {
		this->Router(request, response);
	});
}

void ChipDrive::start()	{
	cout << "ChipDrive C++" << endl;
	this->chiphttp->start();
}

void ChipDrive::Router(Request &request, Response &response) {
	try {
		this->lock->lock();
		cout << "#" + to_string(this->hits) + " Request: " << request.path << endl;
		this->hits++;
		this->lock->unlock();

		string token = request.GetCookie("token");
		
		if(request.path.compare("/api/v1/loginotp") == 0) {
			this->ServeLogin(request, response);
		} else if(request.path.compare("/api/v1/drive/config") == 0) {
			if(token.length() > 0 && token.compare("12345678") == 0) {
				this->ServeConfig(request, response);
			} else {
				response.SetStatus(401);
				throw ChipDriveException("Auth Required");
			}
		} else if(request.path.compare("/api/v1/drive/list") == 0) {
			if(token.length() > 0 && token.compare("12345678") == 0) {
				this->ServeList(request, response);
			} else {
				response.SetStatus(401);
				throw ChipDriveException("Auth Required");
			}
		} else if(request.path.compare("/api/v1/drive/folder") == 0) {
			if(token.length() > 0 && token.compare("12345678") == 0) {
				this->ServeCreateFolder(request, response);
			} else {
				response.SetStatus(401);
				throw ChipDriveException("Auth Required");
			}
		} else if(request.path.compare("/api/v1/drive/rename") == 0) {
			if(token.length() > 0 && token.compare("12345678") == 0) {
				this->ServeRename(request, response);
			} else {
				response.SetStatus(401);
				throw ChipDriveException("Auth Required");
			}
		} else if(request.path.compare("/api/v1/drive/delete") == 0) {
			if(token.length() > 0 && token.compare("12345678") == 0) {
				this->ServeDelete(request, response);
			} else {
				response.SetStatus(401);
				throw ChipDriveException("Auth Required");
			}
		} else if(request.path.compare("/api/v1/drive/upload") == 0) {
			if(token.length() > 0 && token.compare("12345678") == 0) {
				this->ServeUpload(request, response);
			} else {
				response.SetStatus(401);
				throw ChipDriveException("Auth Required");
			}
		} else if(request.path.compare("/api/v1/drive/stream") == 0) {
			if(token.length() > 0 && token.compare("12345678") == 0) {
				this->ServeStream(request, response);
			} else {
				response.SetStatus(401);
				throw ChipDriveException("Auth Required");
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
	}
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

void ChipDrive::ServeRoot(Request &request, Response &response) {
	string path = "./htdocs/bin/" + ChipHttp::CleanPath(request.path);

	struct stat path_stat;
	stat(path.c_str(), &path_stat);

	if(!S_ISREG(path_stat.st_mode)) {
		path.append("/index.html");
	}

	string mime = ChipHttp::GetMIME(path);

	FileStream fs;
	if(fs.open(path, "rb") == true) {
		long int size = fs.size();

		response.SetHeader("Content-Type", mime);
		response.SetHeader("Content-Length", to_string(size));
		char buf[8192];
		while(!fs.eof()) {
			int read = fs.read(buf, sizeof(buf));
			response.write(buf, read);
		}
		fs.close();
	} else {
		string data = "404 Not Found";

		response.SetHeader("Content-Length", to_string(data.size()));
		response.SetHeader("Content-Type", "text/html");
		response.write(data);
	}
}

void ChipDrive::ServeLogin(Request &request, Response &response) {
	string otp = request.GetQuery("otp");

	if(otp.compare("lol") == 0) {
		string raw = this->MakeJSON(true, "", json());

		response.SetCookie("token", "12345678");
		response.SetHeader("Content-Length", to_string(raw.size()));
		response.SetHeader("Content-Type", "application/json");
		response.write(raw);
	} else {
		throw ChipDriveException("Invalid OTP");
	}
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
		this->lock->lock();
		vector<Object> list = FileSystem::List(folderid);
		this->lock->unlock();

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
		this->lock->lock();
		FileSystem::CreateFolder(name, folderid);
		this->lock->unlock();

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
		this->lock->lock();
		FileSystem::Rename(name, itemid);
		this->lock->unlock();

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
		this->lock->lock();
		FileSystem::Delete(itemid);
		this->lock->unlock();

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
		this->lock->lock();
		Object o = FileSystem::CreateFile(name, folderid);
		this->lock->unlock();

		FileStream fs;

		if(fs.open("objects/" + o.id, "wb")) {
			int i = 0;
			int read = 0;
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
		this->lock->lock();
		Object o = FileSystem::GetByID(id);
		this->lock->unlock();

		string path = "./objects/" + id;
		FileStream fs;
		if(fs.open(path, "rb") == true) {
			uint64_t size = fs.size();

			uint64_t start = 0;
			uint64_t end = size - 1;

			string range = request.GetHeader("Range");
			if(range.length() > 0) {
				sscanf(range.c_str(), "bytes=%li-%li", &start, &end);
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
			uint64_t read = 0;
			for(uint64_t p = start; p < end; p += sizeof(buf)) {
				uint64_t to_read = min((int)sizeof(buf), (int)(end - p) + 1);
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
	delete this->lock;
	delete this->chiphttp;
}