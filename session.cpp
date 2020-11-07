#include "session.h"

Session::Session() {

}

bool Session::ValidateToken(string token) {
	for(auto &t : this->tokens) {
		if(t.compare(token) == 0 && token.length() > 0) {
			return true;
		}
	}
	return false;
}

string Session::GenerateToken() {
	string random = this->Random(64);
	this->tokens.push_back(random);
	return random;
}

string Session::Random(int len) {
	string tmp_s;
	static const char alphanum[] = "0123456789abcdef";

	for (int i = 0; i < len; ++i) {
		tmp_s += alphanum[rand() % (sizeof(alphanum) - 1)];
	}
	return tmp_s;
}

Session::~Session() {
	
}