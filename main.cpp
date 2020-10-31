#include <iostream>
#include <fstream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "json.hpp"
#include "chiphttp.h"

using json = nlohmann::json;

void serve_root(Request &request, Response &response);

int main(int argc, char const *argv[]) {

	ChipHttp chttp = ChipHttp(8080);

	chttp.route([](Request &request, Response &response) {
		//if(request.path.compare("/api/v1/config") == 0) {
		json j = {
			{ "success", true },
			{ "time", time(NULL) }
		};
		string data = j.dump(4);

		response.insert("Content-Length", to_string(data.size()));
		response.insert("Content-Type", "application/json");
		response.write(data);
		//} else {
		//	serve_root(request, response);
		//}
	});

	chttp.start();

	return 0;
}

void serve_root(Request &request, Response &response) {
	string path = "./htdocs/bin/" + ChipHttp::CleanPath(request.path);

	struct stat path_stat;
	stat(path.c_str(), &path_stat);

	if(!S_ISREG(path_stat.st_mode)) {
		path.append("/index.html");
	}

	string mime = "application/octet-stream";
	if(path.find(".html") != string::npos) {
		mime = "text/html";
	} else if(path.find(".css") != string::npos) {
		mime = "text/css";
	} else if(path.find(".js") != string::npos) {
		mime = "application/javascript";
	} else if(path.find(".png") != string::npos) {
		mime = "image/png";
	} else if(path.find(".jpg") != string::npos) {
		mime = "image/jpeg";
	} else if(path.find(".jpeg") != string::npos) {
		mime = "image/jpeg";
	} else if(path.find(".svg") != string::npos) {
		mime = "image/svg+xml";
	} else if(path.find(".mp3") != string::npos) {
		mime = "audio/mp3";
	} else if(path.find(".mp4") != string::npos) {
		mime = "video/mp4";
	} else if(path.find(".mov") != string::npos) {
		mime = "video/quicktime";
	}

	ifstream file(path, ifstream::binary);

	if(file) {
		file.exceptions(ifstream::badbit);
		try {
			file.seekg(0, file.end);
			int length = file.tellg();
			file.seekg(0, file.beg);

			response.insert("Content-Type", mime);
			response.insert("Content-Length", to_string(length));
			char buf[8192 * 4];
			while(!file.eof()) {
				file.read(buf, sizeof(buf));
				response.write(buf, file.gcount());
			}
		} catch (ifstream::failure &e) {
			cout << e.what() << endl;
			throw SocketClosedException();
		}
	} else {
		string data = "404 Not Found";

		response.insert("Content-Length", to_string(data.size()));
		response.insert("Content-Type", "text/html");
		response.write(data);
	}

	file.close();
}