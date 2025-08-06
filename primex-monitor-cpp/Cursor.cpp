#include <string>
#include <fstream>
#include <iostream>
#include <cstdio>
#include "Cursor.h"
#include "json.hpp"

using namespace std;
using json = nlohmann::json;

int Cursor::setCursor(json cursor) {
	ofstream tmp("primex-monitor-cursor.json.tmp");
	if (!tmp) {
		cerr << "Error creating file: primex-monitor-cursor.json.tmp" << endl;
		return 1;
	}

	tmp << cursor;
	tmp.flush();
	tmp.close();

	std::remove("primex-monitor-cursor.json");

	if (std::rename("primex-monitor-cursor.json.tmp", "primex-monitor-cursor.json") != 0) {
		std::cerr << "Error renaming temp file to cursor file" << std::endl;
		return 2;
	}

	cout << "Successfully updated cursor: new cursor is " << cursor << endl;
	return 0;
}

json Cursor::getCursor() {
	json cursor;
	ifstream file("primex-monitor-cursor.json");
	if (!file) {
		cout << "Error opening file: primex-monitor-cursor.json" << endl;
		return { {"hasCursor", false} };
	}

	if (file.peek() == std::ifstream::traits_type::eof()) {
		std::cerr << "Cursor file is empty." << std::endl;
		return { {"hasCursor", false} };
	}

	try {
		cursor = json::parse(file);
	}
	catch (const json::parse_error& e) {
		std::cerr << "JSON parse error: " << e.what() << std::endl;
		return { {"hasCursor", false} };
	}


	if (!cursor.contains("timestamp")) {
		cout << "Error: cursor doesn't have timestamp" << endl;
		return { {"hasCursor", false} };
	}

	cout << "Cursor read from primex-monitor-cursor.json: " << cursor << endl;

	cursor["hasCursor"] = true;

	return cursor;
}
