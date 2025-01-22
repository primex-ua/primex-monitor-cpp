#include <string>
#include <fstream>
#include <iostream>
#include "Cursor.h"
#include "json.hpp"

using namespace std;
using json = nlohmann::json;

int Cursor::setCursor(json cursor) {
	ofstream file("cursor.json");

	if (!file) {
		cerr << "Error opening file: cursor.json" << endl;
		return 1;
	}

	file << setw(4) << cursor << endl;

	file.close();

	cout << "Successfully updated cursor: new cursor is " << cursor << endl;

	return 0;
}

json Cursor::getCursor() {
	json cursor;
	ifstream file("cursor.json");

	if (!file) {
		cout << "Error opening file: cursor.json" << endl;
		return { {"hasCursor", false} };
	}

	cursor = json::parse(file);
	cout << "Cursor read from cursor.json: " << cursor << endl;

	if (!cursor.contains("mixed_at") || !cursor.contains("line_name")) {
		cout << "Error: cursor doesn't have mixed_at or line_name" << endl;
		return { {"hasCursor", false} };
	}

	cursor["hasCursor"] = true;
	
	file.close();

	return cursor;
}