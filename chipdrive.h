#ifndef CHIPDRIVE
#define CHIPDRIVE

#include "json.hpp"
#include "chiphttp.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <iostream>
#include <fstream>
#include <ctime>

using json = nlohmann::json;

class ChipDrive {
	public:
		ChipDrive(int port);
		void start();
		~ChipDrive();
	private:
		// HTTP API Services
		ChipHttp *chiphttp;
		// Internal Variables

		// Internal Methods
		static string Random(int len);
		string MakeJSON(bool success, string reason, json data);

		void Router(Request &request, Response &response);
		void ServeRoot(Request &request, Response &response);

		void ServeConfig(Request &request, Response &response);
		void ServeList(Request &request, Response &response);
		void ServeStream(Request &request, Response &response);
};

class ChipDriveAPI : ChipDrive {

};

#endif