#include "chipdrive.h"

ChipDrive::ChipDrive(int port) {
	this->chiphttp = new ChipHttp(port);

	this->chiphttp->route([&](Request &request, Response &response) {
		this->Router(request, response);
	});
}

void ChipDrive::start()	{
	this->chiphttp->start();
}

string ChipDrive::Random(int len) {
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

void ChipDrive::Router(Request &request, Response &response) {
	if(request.path.compare("/api/v1/drive/config") == 0) {
		this->ServeConfig(request, response);
	} else if(request.path.compare("/api/v1/drive/list") == 0) {
		this->ServeList(request, response);
	} else if(request.path.compare("/api/v1/drive/stream") == 0) {
		this->ServeStream(request, response);
	} else {
		this->ServeRoot(request, response);
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

	ifstream file(path, ifstream::binary);

	if(file) {
		file.exceptions(ifstream::badbit);
		try {
			file.seekg(0, file.end);
			int length = file.tellg();
			file.seekg(0, file.beg);

			response.PutHeader("Content-Type", mime);
			response.PutHeader("Content-Length", to_string(length));
			char buf[8192 * 4];
			while(!file.eof()) {
				file.read(buf, sizeof(buf));
				response.write(buf, file.gcount());
			}
		} catch (ifstream::failure &e) {
			cout << e.what() << endl;
		}
	} else {
		string data = "404 Not Found";

		response.PutHeader("Content-Length", to_string(data.size()));
		response.PutHeader("Content-Type", "text/html");
		response.write(data);
	}

	file.close();
}

void ChipDrive::ServeConfig(Request &request, Response &response) {
	json config = {
		{ "root", "chipdrive.internal.0" },
	};
	string raw = this->MakeJSON(false, "", config);

	response.PutHeader("Content-Length", to_string(raw.size()));
	response.PutHeader("Content-Type", "application/json");
	response.write(raw);
}

void ChipDrive::ServeList(Request &request, Response &response) {
	string folderid = request.GetQuery("folderid");

	if(folderid.length() > 0) {
		json config;
		for(int i = 0; i < 100; i++) {
			json item = {
				{ "type", 1 },
				{ "name", "test.mp3" },
				{ "id", ChipDrive::Random(64) }
			};
			config["list"].push_back(item);
		}
		string raw = this->MakeJSON(false, "", config);

		response.PutHeader("Content-Length", to_string(raw.size()));
		response.PutHeader("Content-Type", "application/json");
		response.write(raw);
	} else {
		string raw = this->MakeJSON(true, "Params Not Satisfiable", json());

		response.PutHeader("Content-Length", to_string(raw.size()));
		response.PutHeader("Content-Type", "application/json");
		response.write(raw);
	}
}

void ChipDrive::ServeStream(Request &request, Response &response) {
	string id = request.GetQuery("id");

	if(id.length() > 0) {
		string path = "./objects/test.mp3";
		ifstream file(path, ifstream::binary);

		if(file) {
			file.exceptions(ifstream::badbit);
			try {
				file.seekg(0, file.end);
				int length = file.tellg();
				file.seekg(0, file.beg);

				string range = request.GetHeader("Range");
				int start = 0;
				int end = length - 1;
				sscanf(range.c_str(), "bytes=%i-%i", &start, &end);

				string mime = ChipHttp::GetMIME(path);
				response.PutHeader("Content-Type", mime);
				response.PutHeader("Content-Length", to_string(length));
				char buf[8192 * 4];
				while(!file.eof()) {
					file.read(buf, sizeof(buf));
					response.write(buf, file.gcount());
				}
				file.close();
			} catch (ifstream::failure &e) {
				cout << e.what() << endl;
			}
		} else {
			string raw = this->MakeJSON(true, "Object Not Found", json());

			response.PutHeader("Content-Length", to_string(raw.size()));
			response.PutHeader("Content-Type", "application/json");
			response.write(raw);
		}
	} else {
		string raw = this->MakeJSON(true, "Params Not Satisfiable", json());

		response.PutHeader("Content-Length", to_string(raw.size()));
		response.PutHeader("Content-Type", "application/json");
		response.write(raw);
	}
}

ChipDrive::~ChipDrive() {
	delete this->chiphttp;
}