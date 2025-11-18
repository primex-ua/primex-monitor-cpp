#define CURL_STATICLIB

#include <iostream>
#include <windows.h>
#include "sqlite-db.h"
#include "sqlite3.h"
#include "Cursor.h"
#include "Env.h"
#include "api.h"
#include "json.hpp"
#include "getSystemUUID.h"
#include "SQLQueries.h"
#include <curl/curl.h>
#include "Logger.h"
#include "Hasher.h"

using namespace std;
using json = nlohmann::json;

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow) {
	SetConsoleOutputCP(CP_UTF8);

	HANDLE hMutex = NULL;
	HANDLE hShutdownEvent = NULL;

	try {
		std::string exeDir = getExecutableDir();
		std::string mutexName = Hasher::simpleHash(exeDir);
		std::string shutdownEventName = mutexName + "_shutdown";

		hMutex = CreateMutexA(NULL, FALSE, mutexName.c_str());
		if (!hMutex) {
			std::cerr << "CreateMutex failed with error " << GetLastError() << std::endl;
			return 1;
		}

		if (GetLastError() == ERROR_ALREADY_EXISTS) {
			std::cout << "Another instance is running in this directory." << std::endl;
			CloseHandle(hMutex);
			return 1;
		}

		if (curl_global_init(CURL_GLOBAL_DEFAULT) != 0) {
			std::cerr << "curl_global_init failed." << std::endl;
			CloseHandle(hMutex);
			return 1;
		}

		Logger::init();
		Logger::log("");
		Logger::log("");
		Logger::log("App started with following parameters:");

		hShutdownEvent = CreateEventA(NULL, TRUE, FALSE, shutdownEventName.c_str());
		if (!hShutdownEvent) {
			Logger::log("Failed to create shutdown event. Error: " + std::to_string(GetLastError()));
			Logger::close();
			curl_global_cleanup();
			CloseHandle(hMutex);
			return 1;
		}

		Env& env = Env::getEnv();
		SQLiteDB db(env.getDbPath());

		db.createTableIfNotExists();

		sqlite3_stmt* stmtInitInstanceId = db.prepareStmt(SQL_INIT_DB_INSTANCE_ID.c_str());
		db.queryDatabase(stmtInitInstanceId);
		sqlite3_finalize(stmtInitInstanceId);

		sqlite3_stmt* stmtGetInstanceId = db.prepareStmt(SQL_SELECT_DB_INSTANCE_ID.c_str());
		Table instanceIdResult = db.queryDatabase(stmtGetInstanceId);
		sqlite3_finalize(stmtGetInstanceId);

		std::string dbInstanceId;
		if (instanceIdResult.empty() || instanceIdResult[0].find("value") == instanceIdResult[0].end()) {
			Logger::log("ERROR: Failed to get database instance ID");
			Logger::close();
			curl_global_cleanup();
			CloseHandle(hMutex);
			CloseHandle(hShutdownEvent);
			return 1;
		}

		dbInstanceId = instanceIdResult[0].at("value");
		Logger::log("Database instance ID: " + dbInstanceId);

		sqlite3_stmt* stmtComponents = db.prepareStmt(SQL_SELECT_COMPONENTS.c_str());
		sqlite3_stmt* stmtQueryProducts = db.prepareStmt(SQL_SELECT_PRODUCTS.c_str());
		sqlite3_stmt* stmtQueryTransactions = db.prepareStmt(SQL_SELECT_TRANSACTIONS.c_str());

		bool isAppStart = true;
		int count = 0;
		int syncCount = 0;

		while (true) {
			DWORD waitResult = WaitForSingleObject(hShutdownEvent, 10000);
			if (waitResult == WAIT_OBJECT_0) {
				Logger::log("Shutdown signal received. Exiting main loop.");
				break;
			}
			else if (waitResult == WAIT_TIMEOUT) {
			}
			else {
				DWORD err = GetLastError();
				Logger::log(std::string("WaitForSingleObject failed. Error: ") + std::to_string(err));
				break;
			}

			Logger::log("");

			json cursor = Cursor::getCursor();
			bool hasCursorChanged = false;

			if (!cursor["hasCursor"].get<bool>()) {
				cursor = Cursor::DEFAULT_CURSOR;
				cursor["dbInstanceId"] = dbInstanceId;
				hasCursorChanged = true;

				Logger::log("");
				Logger::log("Cursor is absent, using default cursor instead");
			}
			else if (cursor["dbInstanceId"].get<string>() != dbInstanceId) {
				cursor = Cursor::DEFAULT_CURSOR;
				cursor["dbInstanceId"] = dbInstanceId;
				hasCursorChanged = true;

				Logger::log("");
				Logger::log("Database was recreated, resetting cursor");
			}

			Logger::log("Start reading from the database");

			Table productsResult;
			Table componentsResult;
			Table transactionsResult;
			json products;
			json components;
			json transactions;

			sqlite3_bind_int(stmtComponents, 1, cursor["componentId"].get<int>());
			sqlite3_bind_int(stmtQueryProducts, 1, cursor["productId"].get<int>());
			sqlite3_bind_int(stmtQueryTransactions, 1, cursor["transactionId"].get<int>());

			componentsResult = db.queryDatabase(stmtComponents);
			productsResult = db.queryDatabase(stmtQueryProducts);
			transactionsResult = db.queryDatabase(stmtQueryTransactions);

			sqlite3_reset(stmtComponents);
			sqlite3_reset(stmtQueryProducts);
			sqlite3_reset(stmtQueryTransactions);

			Logger::log("Found " + to_string(componentsResult.size()) + " components, " + to_string(productsResult.size()) + " new products, " + to_string(transactionsResult.size()) + " new transactions");

			if (componentsResult.size() > 0 || productsResult.size() > 0 || transactionsResult.size() > 0) {
				for (const Row& row : componentsResult) {
					if (row.find("component") != row.end()) {
						components.push_back(json::parse(row.at("component")));
					}
				}

				for (const Row& row : productsResult) {
					if (row.find("product") != row.end()) {
						products.push_back(json::parse(row.at("product")));
					}
				}

				for (const Row& row : transactionsResult) {
					if (row.find("component_transaction") != row.end()) {
						transactions.push_back(json::parse(row.at("component_transaction")));
					}
				}

				nlohmann::json postRequestData;
				postRequestData["components"] = components;
				postRequestData["products"] = products;
				postRequestData["componentTransactions"] = transactions;

				string systemUUID = GetSystemUUID();

				bool isRequestSuccess = sendData(env.getApiUrl() + "/sync", env.getApiKey(), systemUUID, postRequestData.dump());

				if (isRequestSuccess) {
					json newCursor;

					newCursor["dbInstanceId"] = cursor["dbInstanceId"];
					newCursor["productId"] = products.empty() || products.back()["id"].is_null() ? cursor["productId"] : products.back()["id"];
					newCursor["componentId"] = components.empty() || components.back()["id"].is_null() ? cursor["componentId"] : components.back()["id"];
					newCursor["transactionId"] = transactions.empty() || transactions.back()["id"].is_null() ? cursor["transactionId"] : transactions.back()["id"];

					Cursor::setCursor(newCursor);

					isAppStart = false;
					count = 0;
					syncCount++;

					if (syncCount >= 50) {
						Logger::log("Performing periodic WAL checkpoint (after 50 syncs)...");
						db.checkpoint();
						syncCount = 0;
					}
				}
			}
			else if (isAppStart || count >= 5) {
				string systemUUID = GetSystemUUID();

				bool isRequestSuccess = sendHeartbeat(env.getApiUrl() + "/heartbeat", env.getApiKey(), systemUUID);

				if (isRequestSuccess) {
					isAppStart = false;
					count = 0;
				}
			}
			else {
				count++;
			}

			if (hasCursorChanged) {
				cursor.erase("hasCursor");
				Cursor::setCursor(cursor);
			}

			components.clear();
			products.clear();
			transactions.clear();
		}

		sqlite3_finalize(stmtComponents);
		sqlite3_finalize(stmtQueryProducts);
		sqlite3_finalize(stmtQueryTransactions);

		db.checkpoint(true);

		curl_global_cleanup();

		Logger::log("");
		Logger::log("App finished");
		Logger::close();

		CloseHandle(hMutex);
		CloseHandle(hShutdownEvent);

		return 0;
	}
	catch (const std::exception& ex) {
		std::cerr << "Unhandled exception: " << ex.what() << std::endl;
		Logger::log(std::string("Unhandled exception: ") + ex.what());
		Logger::close();
		if (hShutdownEvent) CloseHandle(hShutdownEvent);
		if (hMutex) CloseHandle(hMutex);
		curl_global_cleanup();
		return 1;
	}
	catch (...) {
		std::cerr << "Unhandled unknown exception" << std::endl;
		Logger::log("Unhandled unknown exception");
		Logger::close();
		if (hShutdownEvent) CloseHandle(hShutdownEvent);
		if (hMutex) CloseHandle(hMutex);
		curl_global_cleanup();
		return 2;
	}
}
