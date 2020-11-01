#include <string>
#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "chiphttp.h"

Request::Request(int fd) {
	this->fd = fd;
	this->Parse();
}

string Request::GetHeader(string key) {
	auto kv = this->header.find(key);
	if(kv != this->header.end()) {
		return kv->second;
	}
	return "";
}

string Request::GetQuery(string key) {
	auto kv = this->query.find(key);
	if(kv != this->query.end()) {
		return kv->second;
	}
	return "";
}

void Request::Parse() {

	string data = "";

	char bit;

	while(data.find("\r\n\r\n") == string::npos) {
		if(data.length() > MAX_HEADER_LENGTH) {
			throw HeaderTooLargeException();
		}
		this->read(&bit, sizeof(bit));
		data += bit;
	}

	this->ParseHeader(data);
}

void Request::ParseHeader(string header) {
	vector<string> pairs = ChipHttp::SplitToken(header, "\r\n");

	for(auto t = pairs.begin(); t != pairs.end(); ++t) {
		if(t == pairs.begin()) {
			// at header top
			vector<string> kv = ChipHttp::SplitToken(*t, " ");
			if(kv.size() == 3) {
				this->method = kv.at(0);
				pair<string, string> pathquery = ChipHttp::SplitPair(kv.at(1), "?");
				if(pathquery.first.length() > 0) {
					this->path = pathquery.first;
				} else {
					throw MalformedHeaderException();
				}

				if(pathquery.second.length() > 0) {
					this->ParseQuery(pathquery.second);
				}
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
			if((*t).length() > 0) {
				pair<string, string> kv = ChipHttp::SplitPair(*t, ":");
				string key = ChipHttp::Trim(kv.first, " ");
				string val = ChipHttp::Trim(kv.second, " ");
				if(key.length() > 0 && val.length() > 0) {
					this->header.insert(pair<string, string>(key, val));
				} else {
					throw MalformedHeaderException();
				}
			} else {
				throw MalformedHeaderException();
			}
		}
	}
}

void Request::ParseQuery(string query) {
	vector<string> pairs = ChipHttp::SplitToken(query, "&");

	for(auto t = pairs.begin(); t != pairs.end(); ++t) {
		if((*t).length() > 0) {
			pair<string, string> kv = ChipHttp::SplitPair(*t, "=");
			this->query.insert(kv);
		}
	}
}

int Request::read(char *buf, int length) {
	int read = 0;
	read = recv(this->fd, buf, length, 0);
	if(read < 1) {
		throw IOException();
	}
	return read;
}

Request::~Request() {
	
}