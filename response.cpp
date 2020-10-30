#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <cstring>
#include "chiphttp.h"

using namespace std;

Response::Response(int fd) {
	this->fd          = fd;
	this->header_sent = false;
}

void Response::insert(string key, string val) {
	auto const result = this->header.insert(pair<string, string>(key, val));
	if (not result.second) { 
		result.first->second = val; 
	}
}

string Response::build() {
	string result;
	result.append("HTTP/1.1 200 OK");
	result.append("\r\n");
	for (map<string, string>::iterator it = this->header.begin(); it != this->header.end(); it++ ) {
	    result.append(it->first);
	    result.append(": ");
	    result.append(it->second);
	    result.append("\r\n");
	}
	result.append("\r\n");
	return result;
}

int Response::write(string data) {
	return this->write((char*)data.c_str(), data.length());
}

int Response::write(char *data, int size) {
	if(this->header_sent == false) {
		string header = this->build();
		if(send(this->fd, header.c_str(), header.size(), 0) < 1) {
			throw SocketClosed();
		}
		this->header_sent = true;
	}

	int written = 0;
	written = send(this->fd, data, size, 0);

	if(written < 1) {
		throw SocketClosed();
	}

	return written;
}

Response::~Response() {
	
}