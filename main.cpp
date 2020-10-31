#include <iostream>
#include <fstream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "chiphttp.h"

using namespace std;

int main(int argc, char const *argv[]) {

	ChipHttp chttp = ChipHttp(8080);

	chttp.route([](Request &request, Response &response) {
		vector<string> path_pieces = ChipHttp::split(request.path, "/");
		string clean_path = "htdocs/bin/";
		for(auto t = path_pieces.begin(); t != path_pieces.end(); ++t) {
			if((*t).find("..") == string::npos && (*t).length() > 0) {
				clean_path.append(*t);
				clean_path.append("/");
			}
		}

		clean_path.pop_back();

		struct stat path_stat;
    	stat(clean_path.c_str(), &path_stat);

    	if(!S_ISREG(path_stat.st_mode)) {
			clean_path.append("/index.html");
		}

		string mime = "application/octet-stream";
		if(clean_path.find(".html") != string::npos) {
			mime = "text/html";
		} else if(clean_path.find(".css") != string::npos) {
			mime = "text/css";
		} else if(clean_path.find(".js") != string::npos) {
			mime = "application/javascript";
		} else if(clean_path.find(".png") != string::npos) {
			mime = "image/png";
		} else if(clean_path.find(".jpg") != string::npos) {
			mime = "image/jpeg";
		} else if(clean_path.find(".jpeg") != string::npos) {
			mime = "image/jpeg";
		} else if(clean_path.find(".svg") != string::npos) {
			mime = "image/svg+xml";
		} else if(clean_path.find(".mp3") != string::npos) {
			mime = "audio/mp3";
		} else if(clean_path.find(".mp4") != string::npos) {
			mime = "video/mp4";
		} else if(clean_path.find(".mov") != string::npos) {
			mime = "video/quicktime";
		}

		ifstream file(clean_path, ifstream::binary);

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
				throw SocketClosed();
			}
		} else {
			string data = "404 Not Found";

			response.insert("Content-Length", to_string(data.size()));
			response.insert("Content-Type", "text/html");
			response.write(data);
		}

		file.close();

	});

	chttp.start();

	return 0;
}