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
	// HTTP header names are case-insensitive, according to RFC 2616
	for(auto &t : this->header) {
		if(ChipHttp::stricmp(t.first, key) == 0) {
			return t.second;
		}
	}
	return "";
}

string Request::GetQuery(string key) {
	for(auto &t : this->query) {
		if(t.first.compare(key) == 0) {
			return t.second;
		}
	}
	return "";
}

string Request::GetCookie(string key) {
	for(auto &t : this->cookie) {
		if(t.first.compare(key) == 0) {
			return t.second;
		}
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
				string decoded_key = ChipHttp::URLDecode(key);
				string decoded_val = ChipHttp::URLDecode(val);
				if(decoded_key.length() > 0 && decoded_val.length() > 0) {
					this->header.insert(pair<string, string>(decoded_key, decoded_val));
				} else {
					throw MalformedHeaderException();
				}
			} else {
				throw MalformedHeaderException();
			}
		}
	}

	string cookie = this->GetHeader("Cookie");
	if(cookie.length() > 0) {
		this->ParseCookie(cookie);
	}
}

void Request::ParseQuery(string query) {
	vector<string> pairs = ChipHttp::SplitToken(query, "&");

	for(auto t = pairs.begin(); t != pairs.end(); ++t) {
		if((*t).length() > 0) {
			pair<string, string> kv = ChipHttp::SplitPair(*t, "=");
			string key = ChipHttp::URLDecode(kv.first);
			string val = ChipHttp::URLDecode(kv.second);
			if(key.length() > 0) {
				this->query.insert(pair<string, string>(key, val));
			}
		}
	}
}

void Request::ParseCookie(string cookie) {
	vector<string> pairs = ChipHttp::SplitToken(cookie, ";");

	for(auto t = pairs.begin(); t != pairs.end(); ++t) {
		if((*t).length() > 0) {
			pair<string, string> kv = ChipHttp::SplitPair(*t, "=");
			string key = ChipHttp::Trim(kv.first, " ");
			string val = ChipHttp::Trim(kv.second, " ");
			string decoded_key = ChipHttp::URLDecode(key);
			string decoded_val = ChipHttp::URLDecode(val);
			if(decoded_key.length() > 0 && decoded_val.length() > 0) {
				this->cookie.insert(pair<string, string>(decoded_key, decoded_val));
			}
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