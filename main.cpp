#include <iostream>
#include "chiphttp.h"

int main(int argc, char const *argv[]) {
	/* code */

	ChipHttp chttp = ChipHttp(8080);

	chttp.route([](Request &request, Response &response) {
		time_t rawtime;
		struct tm * timeinfo;
		char buffer[80];
		time (&rawtime);
		timeinfo = localtime(&rawtime);
		strftime(buffer, sizeof(buffer),"%d-%m-%Y %H:%M:%S", timeinfo);

		string data = "";
		data.append("<!DOCTYPE html>");
		data.append("<html>");
		data.append("<head>");
		data.append("<style>");
		data.append("body, html { padding: 0; margin: 0; }");
		data.append(".header { text-align: center; font-family: monospace; color: #4D4D4D; font-size: 50px; }");
		data.append("</style>");
		data.append("</head>");
		data.append("<body>");
		data.append("<h1 class=\"header\">" + string(buffer) + "</h1>");
		data.append("</body>");
		data.append("</html>");

		response.insert("Content-Length", to_string(data.size()));
		response.insert("Content-Type", "text/html");
		response.write(data);
	});

	chttp.start();

	return 0;
}