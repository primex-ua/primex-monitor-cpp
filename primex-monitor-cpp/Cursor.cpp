#include "Cursor.h"
#include <string>
#include <fstream>
#include <iostream>
#include "Env.h"
#include <iomanip>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <chrono>
#include <ctime>
#include <string>

int Cursor::setCursor(std::string cursor) {
	std::ofstream file("cursor.txt");

	if (!file) {
		std::cerr << "Error opening file: cursor.txt" << std::endl;
		return 1;
	}

	file << cursor << std::endl;

	file.close();

	std::cout << "Successfully updated cursor: new cursor is " << cursor << std::endl;

	return 0;
}

std::string Cursor::getCursor() {
	std::string cursor;

	std::ifstream file("cursor.txt");

	if (!file) {
		std::cout << "Error opening file: cursor.txt" << std::endl;
		return "UNDEFINED";
	}

	std::getline(file, cursor);
	std::cout << "Cursor read from cursor.txt: " << cursor << std::endl;

	file.close();

	return cursor;
}

std::string Cursor::getStartingTimestamp() {
	Env& env = Env::getEnv();
	int syncPeriodDays = env.getSyncPeriodDays();

	std::time_t now = std::time(nullptr);
	std::time_t shiftSeconds = syncPeriodDays * 24 * 60 * 60;
	std::time_t startingTimestamp = now - shiftSeconds;
	std::tm utcTime;

	if (gmtime_s(&utcTime, &startingTimestamp) != 0) {
		throw std::runtime_error("Failed to convert time to UTC.");
	}

	char utcString[100];
	std::strftime(utcString, sizeof(utcString), "%Y-%m-%d %H:%M:%S", &utcTime);


	std::cout << "Generated starting timestamp: " << utcString << std::endl;
	std::cout << "Start recording from " << syncPeriodDays << " days ago" << std::endl;

	return std::string(utcString);
}