#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <cstring>
#include "chiphttp.h"

using namespace std;

ChipHttpResponse::ChipHttpResponse(int fd) {
	this->fd          = fd;
	this->header_sent = false;
}

void ChipHttpResponse::insert(string key, string val) {
	this->header.insert(pair<string, string>(key, val));
}

string ChipHttpResponse::build() {
	string result = "";
	result += "HTTP/1.1 200 OK\r\n";
	for (map<string, string>::iterator it = this->header.begin(); it != this->header.end(); it++ ) {
	    result += it->first + ": " + it->second + "\r\n";
	}
	result += "\r\n";
	return result;
}

int ChipHttpResponse::write(string data) {
	if(this->header_sent == false) {
		string header = this->build();
		send(this->fd, header.c_str(), header.size(), 0);
		this->header_sent = true;
	}
	return send(this->fd, data.c_str(), data.size(), 0);
}

ChipHttpResponse::~ChipHttpResponse() {
	
}