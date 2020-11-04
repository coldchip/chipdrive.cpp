#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>
#include <iostream>
#include <string>
#include <cstring>
#include <unistd.h>
#include <stdlib.h>
#include <thread>
#include <functional>
#include "chiphttp.h"

ChipHttp::ChipHttp() {
	this->port = 80;
}

ChipHttp::ChipHttp(int port) {
	this->port = port;
}

void ChipHttp::start() {
	signal(SIGPIPE, SIG_IGN);

	this->sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	struct timeval timeout;      
	timeout.tv_sec = 10;
	timeout.tv_usec = 0;

	int one = 1;

    if(setsockopt(this->sockfd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(int)) < 0) {
		error("setsockopt(SO_REUSEADDR) failed");
	}
	if(setsockopt(this->sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char *)&timeout, sizeof(timeout)) < 0) {
		error("setsockopt(SO_RCVTIMEO) failed");
	}

	if(setsockopt(this->sockfd, SOL_SOCKET, SO_SNDTIMEO, (const char *)&timeout, sizeof(timeout)) < 0) {
		error("setsockopt(SO_SNDTIMEO) failed");
	}

	SockAddrIn server_addr;
	server_addr.sin_family      = AF_INET; 
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY); 
    server_addr.sin_port        = htons(this->port); 

    if((bind(this->sockfd, (SockAddr*)&server_addr, sizeof(server_addr))) != 0) { 
        error("socket bind failed..."); 
    }

    if((listen(this->sockfd, 5)) != 0) {
    	error("listen failed..."); 
    }

	while(1) {
		SockAddrIn client_addr;
		socklen_t client_length = sizeof(client_addr);

		int clientfd = accept(this->sockfd, (SockAddr*)&client_addr, &client_length);

		try {
			thread t1(&ChipHttp::process, this, clientfd);
			t1.detach();
		} catch(const exception& e) {
			close(clientfd);
		}
	}
}

void ChipHttp::process(int fd) {
	try {
		while(1) {
			Request  request  = Request(fd);
			Response response = Response(fd);

			response.PutHeader("Connection", "Keep-Alive");
			response.PutHeader("Content-Type", "text/plain");
			response.PutHeader("Keep-Alive", "timeout=5, max=1000");
			response.PutHeader("Server", "ColdChip API Server v2");

			this->cb(request, response);
		}
	} catch(const IOException &e) {
		// this->log(string(e.what()));
	} catch(const HeaderTooLargeException &e) {
		// this->log(string(e.what()));
	} catch(const MalformedHeaderException &e) {
		// this->log(string(e.what()));
	}
	close(fd);
}

void ChipHttp::route(Handler handler) {
	this->cb = handler;
}

vector<string> ChipHttp::SplitToken(string s, string delimiter) {
	size_t pos_start = 0, pos_end, delim_len = delimiter.length();
	string token;
	vector<string> res;

	while ((pos_end = s.find (delimiter, pos_start)) != string::npos) {
		token = s.substr (pos_start, pos_end - pos_start);
		pos_start = pos_end + delim_len;
		res.push_back (token);
	}

	res.push_back (s.substr (pos_start));
	return res;
}

pair<string, string> ChipHttp::SplitPair(string s, string delimiter) {
	pair<string, string> res;

	size_t delim_pos = s.find(delimiter);

	if(delim_pos != string::npos) {
		res.first = s.substr(0, delim_pos);
		res.second = s.substr(delim_pos + delimiter.length());
	} else {
		res.first = s;
		res.second = "";
	}
	return res;
}

string ChipHttp::CleanPath(string pair) {
	vector<string> pieces = ChipHttp::SplitToken(pair, "/");
	string glue = "";
	for(auto t = pieces.begin(); t != pieces.end(); ++t) {
		if((*t).find("..") == string::npos && (*t).length() > 0) {
			glue.append(*t);
			glue.append("/");
		}
	}

	if(glue.back() == '/') {
		glue.pop_back();
	}
	return glue;
}

string ChipHttp::Trim(string str, string whitespace) {
	auto begin = str.find_first_not_of(whitespace);
	if(begin != string::npos) {
		return str.substr(begin);
	}
	return "";
}

string ChipHttp::GetMIME(string &ext) {
	string mime = "application/octet-stream";
	if(ext.find(".html") != string::npos) {
		mime = "text/html";
	} else if(ext.find(".css") != string::npos) {
		mime = "text/css";
	} else if(ext.find(".js") != string::npos) {
		mime = "application/javascript";
	} else if(ext.find(".png") != string::npos) {
		mime = "image/png";
	} else if(ext.find(".jpg") != string::npos) {
		mime = "image/jpeg";
	} else if(ext.find(".jpeg") != string::npos) {
		mime = "image/jpeg";
	} else if(ext.find(".svg") != string::npos) {
		mime = "image/svg+xml";
	} else if(ext.find(".mp3") != string::npos) {
		mime = "audio/mp3";
	} else if(ext.find(".mp4") != string::npos) {
		mime = "video/mp4";
	} else if(ext.find(".mov") != string::npos) {
		mime = "video/quicktime";
	}
	return mime;
}

string ChipHttp::URLDecode(string &src) {
    string ret;
    char ch;
    string::size_type i; 
    int ii;
    for (i = 0; i < src.length(); i++) {
        if(int(src[i]) == 37) {
            if(sscanf(src.substr(i + 1, 2).c_str(), "%x", &ii) == 1) {
            	ch = static_cast<char>(ii);
            	ret += ch;
            	i = i + 2;
        	}
        } else {
            ret += src[i];
        }
    }
    return(ret);
}

void ChipHttp::error(string data) {
	printf("ChipHttp::error [%s]\n", data.c_str());
	exit(1);
}

void ChipHttp::log(string data) {
	printf("ChipHttp::log [%s]\n", data.c_str());
}

ChipHttp::~ChipHttp() {
	
}