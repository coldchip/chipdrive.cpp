#include <string>
#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "chiphttp.h"

using namespace std;

Request::Request(int fd) {
	this->fd = fd;
}

vector<string> Request::split(string s, string delimiter) {
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

void Request::parse() {

	string data = "";

	char bit;
	while(data.find("\r\n\r\n") == string::npos) {
		this->read(&bit, sizeof(bit));
		data += bit;
	}

	vector<string> pairs = this->split(data, "\r\n");

	for(auto t = pairs.begin(); t != pairs.end(); ++t) {
		if(t != pairs.begin()) {
			vector<string> kv = this->split(*t, ":");
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