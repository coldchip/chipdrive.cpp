#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>
#include <iostream>
#include <string>
#include <cstring>
#include <unistd.h>
#include <stdlib.h>
#include <thread>
#include <functional>
#include "ThreadPool.h"
#include "chiphttp.h"

using namespace std;

ChipHttp::ChipHttp() {
	this->port = 80;
}

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

    this->pool = new ThreadPool(200);

	while(1) {
		SockAddrIn client_addr;
		socklen_t client_length = sizeof(client_addr);

		int clientfd = accept(this->sockfd, (SockAddr*)&client_addr, &client_length);

		this->pool->enqueue([=] {
			this->process(clientfd);
		});

	}
}

void ChipHttp::process(int fd) {
	try {
		while(1) {
			Request  request  = Request(fd);
			Response response = Response(fd);

			request.parse();

			response.insert("Connection", "Keep-Alive");
			response.insert("Content-Type", "text/plain");
			response.insert("Keep-Alive", "timeout=5, max=1000");
			response.insert("Server", "ColdChip API Server v2");

			this->cb(request, response);
		}
	} catch(SocketClosed &e) {
		close(fd);
	}
}

void ChipHttp::route(Handler handler) {
	this->cb = handler;
}

void ChipHttp::error(string data) {
	printf("%s\n", data.c_str());
	exit(1);
}

ChipHttp::~ChipHttp() {
	delete this->pool;
}