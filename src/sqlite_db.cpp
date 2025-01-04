#include "sqlite_db.h"

SQLiteDB::SQLiteDB(const std::string &dbPath) {
    int resultCode = sqlite3_open(dbPath.c_str(), &db_);
    if (resultCode != SQLITE_OK) {
        throw std::runtime_error("Error opening database: " + std::string(sqlite3_errmsg(db_)));
    }
    std::cout << "Opened database connection.\n";
}

SQLiteDB::~SQLiteDB() {
    if (db_) {
        sqlite3_close(db_);
    }
}

void SQLiteDB::createTableIfNotExists() {
    const char *sqlCreateTables = R"(
        CREATE TABLE IF NOT EXISTS products (
            id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
            mixed_at INTEGER DEFAULT (unixepoch()) NOT NULL,
            press INTEGER,
            name TEXT NOT NULL,
            component_1 REAL,
            component_2 REAL,
            component_3 REAL,
            component_4 REAL,
            component_5 REAL,
            component_6 REAL,
            water REAL NOT NULL,
            total_weight REAL NOT NULL,
            specific_weight REAL,
            moisture_content REAL,
            mode TEXT CHECK (mode IN ('auto', 'manual'))
        );

        CREATE TABLE IF NOT EXISTS settings ( 
            component_1_name TEXT,
            component_2_name TEXT,
            component_3_name TEXT,
            component_4_name TEXT,
            component_5_name TEXT,
            component_6_name TEXT
        );
    )";

    char *errMsg = nullptr;
    int rc = sqlite3_exec(db_, sqlCreateTables, nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        std::string errorMsg = "SQL error: " + std::string(errMsg);
        sqlite3_free(errMsg);
        throw std::runtime_error(errorMsg);
    }
    std::cout << "Table created or already exists.\n";
}

Table SQLiteDB::queryDatabase(const std::string &sqlQuery) {
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db_, sqlQuery.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        throw std::runtime_error("Failed to prepare statement: " + std::string(sqlite3_errmsg(db_)));
    }

    Table results;
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        Row row;
        int columnCount = sqlite3_column_count(stmt);

        for (int i = 0; i < columnCount; ++i) {
            const char *columnName = sqlite3_column_name(stmt, i);

            if (sqlite3_column_type(stmt, i) != SQLITE_NULL) {
                row[columnName] = reinterpret_cast<const char *>(sqlite3_column_text(stmt, i));
            } else {
                row[columnName] = "NULL";
            }
        }
        results.push_back(row);
    }

    if (rc != SQLITE_DONE) {
        std::string errorMsg = "Error during execution: " + std::string(sqlite3_errmsg(db_));
        sqlite3_finalize(stmt);
        throw std::runtime_error(errorMsg);
    }

    sqlite3_finalize(stmt);
    return results;
}
