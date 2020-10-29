#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>
#include <iostream>
#include <string>
#include <cstring>
#include <unistd.h>
#include <stdlib.h>
#include "chiphttp.h"

using namespace std;

ChipHttp::ChipHttp(int port) {
	this->port = port;
}

void ChipHttp::start() {
	signal(SIGPIPE, SIG_IGN);

	this->sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	struct timeval timeout;      
    timeout.tv_sec = 10;
    timeout.tv_usec = 0;

    int one = 1;

    if(setsockopt(this->sockfd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(int)) < 0) {
		error("setsockopt(SO_REUSEADDR) failed");
	}
	if(setsockopt(this->sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char *)&timeout, sizeof(timeout)) < 0) {
		error("setsockopt(SO_RCVTIMEO) failed");
	}

	if(setsockopt(this->sockfd, SOL_SOCKET, SO_SNDTIMEO, (const char *)&timeout, sizeof(timeout)) < 0) {
		error("setsockopt(SO_SNDTIMEO) failed");
	}

	SockAddrIn server_addr;
	server_addr.sin_family      = AF_INET; 
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY); 
    server_addr.sin_port        = htons(this->port); 

    if((bind(this->sockfd, (SockAddr*)&server_addr, sizeof(server_addr))) != 0) { 
        error("socket bind failed..."); 
    }

    if((listen(this->sockfd, 5)) != 0) {
    	error("listen failed..."); 
    }

	while(1) {
		SockAddrIn client_addr;
		socklen_t client_length = sizeof(client_addr);

		int clientfd = accept(this->sockfd, (SockAddr*)&client_addr, &client_length);

		this->process(clientfd);
	}
}

void ChipHttp::process(int fd) {
	try {
		while(1) {
			ChipHttpRequest  request  = ChipHttpRequest(fd);
			ChipHttpResponse response = ChipHttpResponse(fd);

			string header_data = "";

			char bit;
			while(header_data.find("\r\n\r\n") == string::npos) {
				if(recv(fd, &bit, sizeof(bit), 0) < 1) {
					close(fd);
					throw string("Socket Closed");
				}
				header_data += bit;
			}

			request.parse(header_data);

			response.insert("Server", "ColdChip Web Server");
			response.insert("Content-Type", "text/plain");
			response.insert("Content-Length", "0");

			this->handle(request, response);
		}
	} catch(string &err) {
		close(fd);
	}
}

void ChipHttp::handle(ChipHttpRequest &request, ChipHttpResponse &response) {
	string data = "lol wtf";
	response.insert("Content-Length", to_string(data.size()));
	response.write(data);
}

void ChipHttp::error(string data) {
	printf("%s\n", data.c_str());
	exit(1);
}

ChipHttp::~ChipHttp() {

}