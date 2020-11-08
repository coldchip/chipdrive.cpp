#ifndef CHIPDRIVE
#define CHIPDRIVE

#include <mutex>
#include <sys/types.h>
#include <sys/stat.h>
#include <iostream>
#include <fstream>
#include <ctime>

#include <unistd.h>

#include "json.hpp"
#include "chiphttp.h"
#include "filesystem.h"
#include "filestream.h"
#include "session.h"

using json = nlohmann::json;

class ChipDriveAuthException : public exception {
	public:
		ChipDriveAuthException(string msg) {
			this->msg = msg;
		}
		const char *what() const throw() {
			return this->msg.c_str();
		}
	private:
		string msg;
};

class ChipDriveException : public exception {
	public:
		ChipDriveException(string msg) {
			this->msg = msg;
		}
		const char *what() const throw() {
			return this->msg.c_str();
		}
	private:
		string msg;
};

class ChipDrive {
	public:
		ChipDrive(int port);
		void start();
		~ChipDrive();
	private:
		// HTTP API Services
		ChipHttp *chiphttp;
		// Internal Variables
		mutex *lock;
		Session *session;
		int auth_fd;
		long int hits = 1;
		// Internal Methods
		bool auth(string username, string password);

		string MakeJSON(bool success, string reason, json data);

		void Router(Request &request, Response &response);

		void ServeLogin(Request &request, Response &response);
		void ServeRoot(Request &request, Response &response);
		void ServeConfig(Request &request, Response &response);
		void ServeList(Request &request, Response &response);
		void ServeCreateFolder(Request &request, Response &response);
		void ServeRename(Request &request, Response &response);
		void ServeDelete(Request &request, Response &response);
		void ServeUpload(Request &request, Response &response);
		void ServeStream(Request &request, Response &response);
};

#endif