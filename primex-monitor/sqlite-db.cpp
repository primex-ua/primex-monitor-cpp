#include "sqlite-db.h"
#include "Logger.h"
#include "SQLQueries.h"

SQLiteDB::SQLiteDB(const std::string &dbPath) {
	int resultCode = sqlite3_open(dbPath.c_str(), &db_);
	if (resultCode != SQLITE_OK) {
		std::string errorMsg = "Error opening database: " + std::string(sqlite3_errmsg(db_));
		Logger::log(errorMsg);
		throw std::runtime_error(errorMsg);
	}

	resultCode = sqlite3_busy_timeout(db_, 5000);
	if (resultCode != SQLITE_OK) {
		std::string errorMsg = "Error setting busy timeout: " + std::string(sqlite3_errmsg(db_));
		Logger::log(errorMsg);
		throw std::runtime_error(errorMsg);
	}

	char* errMsg = nullptr;
	resultCode = sqlite3_exec(db_, "PRAGMA journal_mode=WAL;", nullptr, nullptr, &errMsg);
	if (resultCode != SQLITE_OK) {
		std::string errorMsg = "Error enabling WAL journal mode: " + std::string(errMsg ? errMsg : sqlite3_errmsg(db_));
		if (errMsg) {
			sqlite3_free(errMsg);
		}
		Logger::log(errorMsg);
		throw std::runtime_error(errorMsg);
	}
	if (errMsg) {
		sqlite3_free(errMsg);
	}

	Logger::log("Enabled WAL journal mode.");

	Logger::log("Opened database connection: " + dbPath);
}

SQLiteDB::~SQLiteDB() {
	if (db_) {
		checkpoint(true);
		sqlite3_close(db_);
		Logger::log("Closed database connection.");
	}
}

void SQLiteDB::createTableIfNotExists() {
	char *errMsg = nullptr;
	int rc = sqlite3_exec(db_, SQL_CREATE_TABLES.c_str(), nullptr, nullptr, &errMsg);
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
	const int maxAttempts = 5;
	int attemptsRemaining = maxAttempts;

	while (true) {
		rc = sqlite3_step(stmt);

		if (rc == SQLITE_ROW) {
			Row row;
			int columnCount = sqlite3_column_count(stmt);

			for (int i = 0; i < columnCount; ++i) {
				const char* columnName = sqlite3_column_name(stmt, i);

				if (sqlite3_column_type(stmt, i) != SQLITE_NULL) {
					row[columnName] = reinterpret_cast<const char*>(sqlite3_column_text(stmt, i));
				}
				else {
					row[columnName] = "NULL";
				}
			}
			results.push_back(row);
			continue;
		}

		if (rc == SQLITE_DONE) {
			break;
		}

		if (rc == SQLITE_BUSY || rc == SQLITE_LOCKED) {
			Logger::log("Database is busy, retrying...");
			
			if (--attemptsRemaining <= 0) {
				std::string errorMsg = "Query aborted after " + std::to_string(maxAttempts) + " busy retries: " + std::string(sqlite3_errmsg(db_));
				Logger::log(errorMsg);
				throw std::runtime_error(errorMsg);
			}

			sqlite3_sleep(100);
			continue;
		}

		std::string errorMsg = "Error during query execution: " + std::string(sqlite3_errmsg(db_));
		Logger::log(errorMsg);
		throw std::runtime_error(errorMsg);
	}

	return results;
}

void SQLiteDB::checkpoint(bool truncate) {
	if (db_) {
		int mode = truncate ? SQLITE_CHECKPOINT_TRUNCATE : SQLITE_CHECKPOINT_PASSIVE;
		int nLog = 0, nCkpt = 0;
		int rc = sqlite3_wal_checkpoint_v2(db_, nullptr, mode, &nLog, &nCkpt);
		if (rc != SQLITE_OK) {
			Logger::log("Warning: WAL checkpoint failed: " + std::string(sqlite3_errmsg(db_)));
		} else {
			std::string modeStr = truncate ? " (truncated)" : " (passive)";
			Logger::log("WAL checkpoint completed" + modeStr + ": " + std::to_string(nLog) + " frames in WAL, " + std::to_string(nCkpt) + " checkpointed.");
		}
	}
}
