#ifndef CHIPHTTP
#define CHIPHTTP

#include <string>
#include <map>
#include <vector>
#include <functional>
#include "ThreadPool.h"

using namespace std;

typedef struct sockaddr_in SockAddrIn;
typedef struct sockaddr SockAddr;

struct SocketClosed : public exception {
	const char *what () const throw () {
		return "C++ Exception";
	}
};

class Request {
	public:
		Request(int fd);
		void parse();
		int read(char *buf, int length);
		~Request();
	private:
		int fd;
		vector<string> split (string s, string delimiter);
		map <string, string> header;
		
};

class Response {
	public:
		Response(int fd);
		void insert(string key, string val);
		int write(string data);
		~Response();
	private:
		int fd;
		bool header_sent;
		map <string, string> header;
		string build();
};

class ChipHttp {
	using Handler = function<void(Request &request, Response &response)>;
	public:
		ChipHttp();
		ChipHttp(int port);
		void start();
		void route(Handler handler);
		~ChipHttp();
	private:
		void error(string data);
		void process(int clientfd);
		ThreadPool *pool;
		Handler cb;
		int sockfd;
		int port;

};

#endif