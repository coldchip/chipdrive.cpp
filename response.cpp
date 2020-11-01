#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <cstring>
#include "chiphttp.h"

Response::Response(int fd) {
	this->fd          = fd;
	this->header_sent = false;
}

void Response::PutHeader(string key, string val) {
	auto const result = this->header.insert(pair<string, string>(key, val));
	if (not result.second) { 
		result.first->second = val; 
	}
}

void Response::SetStatus(int status) {
	this->status = status;
}

string Response::Build() {
	string result;
	string status_string = "HTTP/1.1 200 OK";
	switch(this->status) {
		case 200:
			result.append("HTTP/1.1 200 OK");
		break;
		case 206:
			result.append("HTTP/1.1 206 Partial Content");
		break;
		case 404:
			result.append("HTTP/1.1 404 Not Found");
		break;
		case 416:
			result.append("HTTP/1.1 416 Requested Range Not Satisfiable");
		break;
	}
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
		string header = this->Build();
		if(send(this->fd, header.c_str(), header.size(), 0) < 1) {
			throw IOException();
		}
		this->header_sent = true;
	}

	int written = 0;
	written = send(this->fd, data, size, 0);

	if(written < 1) {
		throw IOException();
	}

	return written;
}

Response::~Response() {
	
}