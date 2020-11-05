#ifndef FILESTREAM
#define FILESTREAM

#include <string>

using namespace std;

class FileStream {
	public:
		FileStream();
		bool open(string file, string mode);
		long int tell();
		int seek(long int offset, int whence);
		int eof();
		long int size();
		int read(char *buf, int length);
		int write(char *buf, int length);
		void close();
		~FileStream();
	private:
		FILE *file;
};

#endif