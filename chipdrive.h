#ifndef CHIPDRIVE
#define CHIPDRIVE

#include <mutex>
#include <sys/types.h>
#include <sys/stat.h>
#include <iostream>
#include <fstream>
#include <ctime>

#include "json.hpp"
#include "chiphttp.h"
#include "filesystem.h"
#include "filestream.h"
#include "session.h"

using namespace std;
using json = nlohmann::json;

typedef struct _TarHeader {       /* byte offset */
    char name[100];               /*   0 */
    char mode[8];                 /* 100 */
    char uid[8];                  /* 108 */
    char gid[8];                  /* 116 */
    char size[12];                /* 124 */
    char mtime[12];               /* 136 */
    char chksum[8];               /* 148 */
    char typeflag;                /* 156 */
    char linkname[100];           /* 157 */
    char magic[6];                /* 257 */
    char version[2];              /* 263 */
    char uname[32];               /* 265 */
    char gname[32];               /* 297 */
    char devmajor[8];             /* 329 */
    char devminor[8];             /* 337 */
    char prefix[155];             /* 345 */
    char extra_padding[12];       /* 500 */
} TarHeader;

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

		char *GetTarFile(string path, long long *size);
		void ServeRoot(Request &request, Response &response);
		void ServeLogin(Request &request, Response &response);
		void ServeLogout(Request &request, Response &response);
		void ServeConfig(Request &request, Response &response);
		void ServeList(Request &request, Response &response);
		void ServeCreateFolder(Request &request, Response &response);
		void ServeRename(Request &request, Response &response);
		void ServeDelete(Request &request, Response &response);
		void ServeUpload(Request &request, Response &response);
		void ServeStream(Request &request, Response &response);
};

#endif