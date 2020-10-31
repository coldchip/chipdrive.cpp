#include <string>
#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "chiphttp.h"

Request::Request(int fd) {
	this->fd = fd;
	this->parse();
}

void Request::parse() {

	string data = "";

	char bit;

	while(data.find("\r\n\r\n") == string::npos) {
		if(data.length() > MAX_HEADER_LENGTH) {
			throw HeaderTooLargeException();
		}
		this->read(&bit, sizeof(bit));
		data += bit;
	}

	vector<string> pairs = ChipHttp::split(data, "\r\n");

	for(auto t = pairs.begin(); t != pairs.end(); ++t) {
		if(t == pairs.begin()) {
			vector<string> kv = ChipHttp::split(*t, " ");
			if(kv.size() == 3) {
				this->method = kv.at(0);
				this->path = kv.at(1);
			} else {
				throw MalformedHeaderException();
			}
		} else {
			vector<string> kv = ChipHttp::split(*t, ":");
			if(kv.size() == 2) {
				this->header.insert(pair<string, string>(kv.at(0), kv.at(1)));
			}
		}
	}
}

int Request::read(char *buf, int length) {
	int read = 0;
	read = recv(this->fd, buf, length, 0);
	if(read < 1) {
		throw SocketClosedException();
	}
	return read;
}

Request::~Request() {
	
}