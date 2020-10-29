#include <iostream>
#include "chiphttp.h"

int main(int argc, char const *argv[]) {
	/* code */

	ChipHttp chttp = ChipHttp(8080);

	chttp.start();

	return 0;
}