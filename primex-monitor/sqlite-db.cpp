#include "sqlite-db.h"
#include "Logger.h"

SQLiteDB::SQLiteDB(const std::string &dbPath) {
	int resultCode = sqlite3_open(dbPath.c_str(), &db_);
	if (resultCode != SQLITE_OK) {
		std::string errorMsg = "Error opening database: " + std::string(sqlite3_errmsg(db_));
		Logger::log(errorMsg);
		throw std::runtime_error(errorMsg);
	}
	Logger::log("Opened database connection: " + dbPath);
}

SQLiteDB::~SQLiteDB() {
	if (db_) {
		sqlite3_close(db_);
		Logger::log("Closed database connection.");
	}
}

void SQLiteDB::createTableIfNotExists() {
	const char *sqlCreateTables = R"(
        CREATE TABLE IF NOT EXISTS "products" (
			"id"					INTEGER NOT NULL,
			"mixed_at"				TIMESTAMP NOT NULL DEFAULT (DATETIME('now', 'localtime')),
			"name"					TEXT NOT NULL,
			"line_name"				TEXT,
			"press"					INTEGER,
			"total_weight"			REAL NOT NULL,
			"moisture_content"		REAL,
			"temperature"			REAL,
			"output"				REAL,
			"water_weight"			REAL NOT NULL,
			"component_1_weight"	REAL,
			"component_1_name"		TEXT,
			"component_2_weight"	REAL,
			"component_2_name"		TEXT,
			"component_3_weight"	REAL,
			"component_3_name"		TEXT,
			"component_4_weight"	REAL,
			"component_4_name"		TEXT,
			"component_5_weight"	REAL,
			"component_5_name"		TEXT,
			"component_6_weight"	REAL,
			"component_6_name"		TEXT,
			PRIMARY KEY("id" AUTOINCREMENT)
		);

		CREATE TABLE IF NOT EXISTS "components" (
			"id"			INTEGER NOT NULL,
			"updated_at"	TIMESTAMP NOT NULL DEFAULT (DATETIME('now', 'localtime')),
			"name"			TEXT NOT NULL UNIQUE,
			"weight"		REAL NOT NULL,
			PRIMARY KEY("id" AUTOINCREMENT)
		);

		CREATE TABLE IF NOT EXISTS "component_transactions" (
			"id"				INTEGER NOT NULL,
			"timestamp"			TIMESTAMP NOT NULL DEFAULT (DATETIME('now', 'localtime')),
			"component_name"	TEXT NOT NULL,
			"amount"			REAL NOT NULL,
			PRIMARY KEY("id" AUTOINCREMENT)
		);
    )";

	char *errMsg = nullptr;
	int rc = sqlite3_exec(db_, sqlCreateTables, nullptr, nullptr, &errMsg);
	if (rc != SQLITE_OK) {
		std::string errorMsg = "SQL error: " + std::string(errMsg);
		sqlite3_free(errMsg);
		Logger::log(errorMsg);
		throw std::runtime_error(errorMsg);
	}

	Logger::log("Tables ensured (created or already exist).");
}

sqlite3_stmt* SQLiteDB::prepareStmt(const char* sql) {
	sqlite3_stmt* stmt;
	int rc = sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);

	if (rc != SQLITE_OK) {
		std::string errorMsg = "Failed to prepare SQL statement: " + std::string(sqlite3_errmsg(db_));
		Logger::log(errorMsg);
		throw std::runtime_error(errorMsg);
	}

	return stmt;
}

Table SQLiteDB::queryDatabase(sqlite3_stmt* stmt) {
	int rc;
	Table results;

	while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
		Row row;
		int columnCount = sqlite3_column_count(stmt);

		for (int i = 0; i < columnCount; ++i) {
			const char *columnName = sqlite3_column_name(stmt, i);

			if (sqlite3_column_type(stmt, i) != SQLITE_NULL) {
				row[columnName] = reinterpret_cast<const char *>(sqlite3_column_text(stmt, i));
			}
			else {
				row[columnName] = "NULL";
			}
		}
		results.push_back(row);
	}

	if (rc != SQLITE_DONE) {
		std::string errorMsg = "Error during query execution: " + std::string(sqlite3_errmsg(db_));
		Logger::log(errorMsg);
		throw std::runtime_error(errorMsg);
	}

	return results;
}
