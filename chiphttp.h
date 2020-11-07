#ifndef CHIPHTTP
#define CHIPHTTP

#include <string>
#include <map>
#include <vector>
#include <functional>

using namespace std;

#define MAX_HEADER_LENGTH 8192 * 2
#define MAX_POST_LENGTH 8192

typedef struct sockaddr_in SockAddrIn;
typedef struct sockaddr SockAddr;

struct IOException : public exception {
	const char *what() const throw() {
		return "ChipHttp IOException";
	}
};

struct HeaderTooLargeException : public exception {
	const char *what() const throw() {
		return "ChipHttp HeaderTooLargeException";
	}
};

struct MalformedHeaderException : public exception {
	const char *what() const throw() {
		return "ChipHttp MalformedHeaderException";
	}
};

class Request {
	public:
		string method;
		string path;

		Request(int fd);
		string GetHeader(string key);
		string GetQuery(string key);
		string GetCookie(string key);
		int read(char *buf, int length);
		~Request();
	private:
		int fd;
		map <string, string> header;
		map <string, string> query;
		map <string, string> cookie;

		void Parse();
		void ParseHeader(string header);
		void ParseQuery(string query);
		void ParseCookie(string cookie);
};

class Response {
	public:
		Response(int fd);
		void SetHeader(string key, string val);
		void SetCookie(string key, string val);
		void SetStatus(int status);
		int write(string data);
		int write(char *data, int size);
		~Response();
	private:
		int fd;
		bool header_sent;
		int status = 200;
		map <string, string> header;
		map <string, string> cookie;

		string Build();
};

class ChipHttp {
	using Handler = function<void(Request &request, Response &response)>;
	public:
		map<string, string> mime;

		static vector<string> SplitToken(string s, string delimiter);
		static pair<string, string> SplitPair(string s, string delimiter);
		static string CleanPath(string path);
		static string Trim(string str, string whitespace);
		static string GetMIME(string &ext);
		static string URLDecode(string &src);
		static int stricmp(const string &a, const string &b);

		ChipHttp();
		ChipHttp(int port);
		void start();
		void route(Handler handler);
		~ChipHttp();
	private:
		Handler cb;
		int sockfd;
		int port;

		void error(string data);
		void log(string data);
		void process(int clientfd);
};

#endif