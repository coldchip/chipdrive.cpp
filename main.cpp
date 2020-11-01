#include <iostream>
#include "chipdrive.h"

int main(int argc, char const *argv[]) {
	ChipDrive cd = ChipDrive(8080);
	cd.start();
	return 0;
}