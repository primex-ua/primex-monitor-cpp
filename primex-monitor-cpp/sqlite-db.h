#ifndef SQLITE_DB_H
#define SQLITE_DB_H

#include <iostream>
#include <map>
#include <sqlite3.h>
#include <stdexcept>
#include <string>
#include <vector>

// Define Table and Row for database query results
using Row = std::map<std::string, std::string>;
using Table = std::vector<Row>;

class SQLiteDB {
public:
	explicit SQLiteDB(const std::string &dbPath);
	~SQLiteDB();

	void createTableIfNotExists();
	sqlite3_stmt* prepareStmt(const char* sql);
	Table queryDatabase(sqlite3_stmt* stmt);

private:
	sqlite3 *db_;
};

#endif // SQLITE_DB_H