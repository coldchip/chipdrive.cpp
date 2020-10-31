#include <string>
#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "chiphttp.h"

Request::Request(int fd) {
	this->fd = fd;
}

void Request::parse() {

	string data = "";

	char bit;
	while(data.find("\r\n\r\n") == string::npos) {
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
		throw SocketClosed();
	}
	return read;
}

Request::~Request() {
	
}