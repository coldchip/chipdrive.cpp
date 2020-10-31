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

			response.insert("Connection", "Keep-Alive");
			response.insert("Content-Type", "text/plain");
			response.insert("Keep-Alive", "timeout=5, max=1000");
			response.insert("Server", "ColdChip API Server v2");

			this->cb(request, response);
		}
	} catch(const SocketClosedException &e) {
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

void ChipHttp::error(string data) {
	printf("ChipHttp::error [%s]\n", data.c_str());
	exit(1);
}

void ChipHttp::log(string data) {
	printf("ChipHttp::log [%s]\n", data.c_str());
}

ChipHttp::~ChipHttp() {
	
}