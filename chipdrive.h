#ifndef CHIPDRIVE
#define CHIPDRIVE

#include "json.hpp"
#include "chiphttp.h"
#include "filesystem.h"
#include "filestream.h"
#include <mutex>
#include <sys/types.h>
#include <sys/stat.h>
#include <iostream>
#include <fstream>
#include <ctime>

using json = nlohmann::json;

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
		long int hits = 0;
		// Internal Methods
		string MakeJSON(bool success, string reason, json data);

		void Router(Request &request, Response &response);
		void ServeRoot(Request &request, Response &response);

		void ServeConfig(Request &request, Response &response);
		void ServeList(Request &request, Response &response);
		void ServeCreateFolder(Request &request, Response &response);
		void ServeUpload(Request &request, Response &response);
		void ServeStream(Request &request, Response &response);
};

#endif