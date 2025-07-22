#include <string>
#include <fstream>
#include <iostream>
#include "Cursor.h"
#include "json.hpp"

using namespace std;
using json = nlohmann::json;

int Cursor::setCursor(json cursor) {
	ofstream file("primex-monitor-cursor.json");

	if (!file) {
		cerr << "Error opening file: primex-monitor-cursor.json" << endl;
		return 1;
	}

	file << setw(4) << cursor << endl;

	file.close();

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

	cursor = json::parse(file);
	cout << "Cursor read from primex-monitor-cursor.json: " << cursor << endl;

	if (!cursor.contains("timestamp")) {
		cout << "Error: cursor doesn't have timestamp" << endl;
		return { {"hasCursor", false} };
	}

	cursor["hasCursor"] = true;
	
	file.close();

	return cursor;
}