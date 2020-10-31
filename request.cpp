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

	vector<string> pairs = ChipHttp::SplitToken(data, "\r\n");

	for(auto t = pairs.begin(); t != pairs.end(); ++t) {
		if(t == pairs.begin()) {
			// at header top
			vector<string> kv = ChipHttp::SplitToken(*t, " ");
			if(kv.size() == 3) {
				this->method = kv.at(0);
				this->path = kv.at(1);
			} else {
				throw MalformedHeaderException();
			}
		} else if(t == pairs.end() - 1 || t == pairs.end() - 2) {
			// at \r\n\r\n
			if((*t).length() > 0) {
				throw MalformedHeaderException();
			}
		} else {
			// at any header pair
			pair<string, string> kv = ChipHttp::SplitPair(*t, ":");
			if(kv.first.length() > 0 && kv.second.length() > 0) {
				this->header.insert(kv);
			} else {
				throw MalformedHeaderException();
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