#define WIN32_LEAN_AND_MEAN
#define STRICT
#include <windows.h>
#include <iostream>
#include <string>
#include <unordered_map>
#include "addProduct.h"
#include <cstdlib>
#include <ctime>

using namespace std;

int main(int argc, char *argv[]) {
	SetConsoleOutputCP(CP_UTF8);

	srand(static_cast<unsigned int>(time(nullptr)));

	unordered_map<string, string> args;
	int intervalSeconds = 1;
	int count = 1;
	bool isInfiniteCount = false;

	for (int i = 1; i < argc; i += 2) {
		if (i + 1 < argc) {
			args[argv[i]] = argv[i + 1];
		}
		else {
			cerr << "Error: Value missing for argument " << argv[i] << endl;
			return 1;
		}
	}

	bool hasInterval = args.find("-i") != args.end();
	bool hasCount = args.find("-c") != args.end();

	if (hasInterval) {
		try {
			intervalSeconds = stoi(args["-i"]);
		}
		catch (...) {
			cerr << "-i (interval) must be a number" << endl;
		}
	}
	cout << "Interval: " << intervalSeconds << endl;

	if (hasCount) {
		try {
			count = stoi(args["-c"]);
		}
		catch (...) {
			cerr << "-c (count) must be a number" << endl;
		}
	}

	isInfiniteCount = hasInterval && !hasCount;

	cout << "Count: " << (isInfiniteCount ? "infinite" : to_string(count)) << endl;

	int i = 1;
	while (isInfiniteCount || i <= count) {
		if (!isInfiniteCount) {
			cout << i << ". ";
		}

		addProduct();

		Sleep(intervalSeconds * 1000);

		if (!isInfiniteCount) {
			i++;
		}
	}

	return 0;
}
