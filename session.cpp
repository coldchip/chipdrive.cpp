#include "session.h"

Session::Session() {

}

bool Session::ValidateToken(string token, mutex *lock_) {
	std::lock_guard<std::mutex> lock(*lock_);

	auto t = this->tokens.begin();

	while(t != this->tokens.end()) {
		if(time(NULL) > t->expiry) {
			// Fix loop
			t = this->tokens.erase(t);
		} else {
			if(t->id.compare(token) == 0 && token.length() > 0) {
				return true;
			}
			t++;
		}
	}
	
	return false;
}

string Session::GenerateToken(mutex *lock_) {
	std::lock_guard<std::mutex> lock(*lock_);

	string random = this->Random(64);

	Token token;
	token.id     = random;
	token.expiry = time(NULL) + 60 * 60;

	this->tokens.push_back(token);
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