#include <string>
#include <fstream>
#include <iostream>
#include <cstdio>
#include "Cursor.h"
#include "json.hpp"
#include "Logger.h"

using namespace std;
using json = nlohmann::json;

int Cursor::setCursor(json cursor) {
	ofstream tmp("primex-monitor-cursor.json.tmp");
	if (!tmp) {
		Logger::log("Error creating file: primex-monitor-cursor.json.tmp");
		return 1;
	}

	tmp << cursor;
	tmp.flush();
	tmp.close();

	std::remove("primex-monitor-cursor.json");

	if (std::rename("primex-monitor-cursor.json.tmp", "primex-monitor-cursor.json") != 0) {
		Logger::log("Error renaming temp file to cursor file");
		return 2;
	}

	Logger::log("Successfully updated cursor: new cursor is " + cursor.dump());

	return 0;
}

json Cursor::getCursor() {
	json cursor;
	ifstream file("primex-monitor-cursor.json");
	if (!file) {
		Logger::log("Error opening file: primex-monitor-cursor.json");
		return { {"hasCursor", false} };
	}

	if (file.peek() == std::ifstream::traits_type::eof()) {
		Logger::log("Cursor file is empty.");
		return { {"hasCursor", false} };
	}

	try {
		cursor = json::parse(file);
	}
	catch (const json::parse_error& e) {
		Logger::log("JSON parse error: " + std::string(e.what()));
		return { {"hasCursor", false} };
	}


	if (!cursor.contains("timestamp")) {
		Logger::log("Error: cursor doesn't have timestamp");
		return { {"hasCursor", false} };
	}

	Logger::log("Cursor read from primex-monitor-cursor.json: " + cursor.dump());

	cursor["hasCursor"] = true;

	return cursor;
}
