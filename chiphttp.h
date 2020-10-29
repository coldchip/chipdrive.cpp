#ifndef CHIPHTTP
#define CHIPHTTP

#include <string>
#include <map>
#include <vector>

using namespace std;

typedef struct sockaddr_in SockAddrIn;
typedef struct sockaddr SockAddr;

class ChipHttpRequest {
	public:
		ChipHttpRequest(int fd);
		void parse(string data);
		~ChipHttpRequest();
	private:
		int fd;
		vector<string> split (string s, string delimiter);
		map <string, string> header;
		
};

class ChipHttpResponse {
	public:
		ChipHttpResponse(int fd);
		void insert(string key, string val);
		int write(string data);
		~ChipHttpResponse();
	private:
		int fd;
		bool header_sent;
		map <string, string> header;
		string build();
};

class ChipHttp {
	public:
		ChipHttp(int port);
		void start();
		void error(string data);
		void process(int clientfd);
		void handle(ChipHttpRequest &request, ChipHttpResponse &response);
		~ChipHttp();
	private:
		int sockfd;
		int port;
		
};

#endif