#define CURL_STATICLIB

#include <iostream>
#include <windows.h>
#include "sqlite-db.h"
#include "sqlite3.h"
#include "Cursor.h"
#include "Env.h"
#include "sendData.h"
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

	try {
		std::string exeDir = getExecutableDir();
		std::string mutexName = Hasher::simpleHash(exeDir);

		HANDLE hMutex = CreateMutexA(NULL, FALSE, mutexName.c_str());
		if (!hMutex) {
			std::cerr << "CreateMutex failed with error " << GetLastError() << std::endl;
			return 1;
		}

		if (GetLastError() == ERROR_ALREADY_EXISTS) {
			std::cout << "Another instance is running in this directory." << std::endl;
			CloseHandle(hMutex);
			return 1;
		}

		curl_global_init(CURL_GLOBAL_DEFAULT);

		Logger::init();

		Logger::log("App started with following parameters:");

		Env& env = Env::getEnv();
		SQLiteDB db(env.getDbPath());

		db.createTableIfNotExists();

		sqlite3_stmt* stmtComponents = db.prepareStmt(SQL_SELECT_COMPONENTS.c_str());

		sqlite3_stmt* stmtQueryProducts = db.prepareStmt(SQL_SELECT_PRODUCTS.c_str());

		sqlite3_stmt* stmtQueryTransactions = db.prepareStmt(SQL_SELECT_TRANSACTIONS.c_str());

		while (true) {
			Logger::log("");
			Logger::log("Start reading from the database");

			json cursor = Cursor::getCursor();
			Table productsResult;
			Table componentsResult;
			Table transactionsResult;
			json products;
			json components;
			json transactions;

			if (cursor["hasCursor"].get<bool>()) {
				sqlite3_bind_text(stmtComponents, 1, cursor["timestamp"].get<string>().c_str(), -1, SQLITE_TRANSIENT);
				sqlite3_bind_text(stmtComponents, 2, cursor["timestamp"].get<string>().c_str(), -1, SQLITE_TRANSIENT);

				sqlite3_bind_text(stmtQueryProducts, 1, cursor["timestamp"].get<string>().c_str(), -1, SQLITE_TRANSIENT);
				sqlite3_bind_null(stmtQueryProducts, 2);

				sqlite3_bind_text(stmtQueryTransactions, 1, cursor["timestamp"].get<string>().c_str(), -1, SQLITE_TRANSIENT);
				sqlite3_bind_null(stmtQueryTransactions, 2);

				componentsResult = db.queryDatabase(stmtComponents);
				productsResult = db.queryDatabase(stmtQueryProducts);
				transactionsResult = db.queryDatabase(stmtQueryTransactions);
			}
			else {
				string period = "-" + to_string(env.getSyncPeriodDays()) + " days";

				sqlite3_bind_null(stmtComponents, 1);
				sqlite3_bind_null(stmtComponents, 2);

				sqlite3_bind_null(stmtQueryProducts, 1);
				sqlite3_bind_text(stmtQueryProducts, 2, period.c_str(), -1, SQLITE_TRANSIENT);

				sqlite3_bind_null(stmtQueryTransactions, 1);
				sqlite3_bind_text(stmtQueryTransactions, 2, period.c_str(), -1, SQLITE_TRANSIENT);

				componentsResult = db.queryDatabase(stmtComponents);
				productsResult = db.queryDatabase(stmtQueryProducts);
				transactionsResult = db.queryDatabase(stmtQueryTransactions);
			}

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

				bool isRequestSuccess = sendData(env.getApiUrl(), env.getApiKey(), systemUUID, postRequestData.dump());

				if (isRequestSuccess) {
					cout << endl;

					string latestTimestamp = cursor["hasCursor"].get<bool>() ? cursor["timestamp"].get<string>() : "0000-00-00T00:00:00Z";

					if (!products.empty() && !products.back()["mixedAt"].is_null()) {
						string productTimestamp = products.back()["mixedAt"].get<string>();
						if (productTimestamp > latestTimestamp) {
							latestTimestamp = productTimestamp;
						}
					}

					if (!transactions.empty() && !transactions.back()["timestamp"].is_null()) {
						string transactionTimestamp = transactions.back()["timestamp"].get<string>();
						if (transactionTimestamp > latestTimestamp) {
							latestTimestamp = transactionTimestamp;
						}
					}

					for (const auto& comp : components) {
						if (!comp["updated_at"].is_null()) {
							string componentTimestamp = comp["updated_at"].get<string>();
							if (componentTimestamp > latestTimestamp) {
								latestTimestamp = componentTimestamp;
							}
						}
					}

					if (!latestTimestamp.empty()) {
						json newCursor;
						newCursor["timestamp"] = latestTimestamp;
						Cursor::setCursor(newCursor);
					}

				}
			}

			components.clear();
			products.clear();
			transactions.clear();

			Sleep(10000);
		}

		sqlite3_finalize(stmtComponents);
		sqlite3_finalize(stmtQueryProducts);
		sqlite3_finalize(stmtQueryTransactions);

		curl_global_cleanup();

		Logger::log("App finished");
		Logger::close();

		CloseHandle(hMutex);

		return 0;
	}
	catch (const std::exception& ex) {
		std::cerr << "Unhandled exception: " << ex.what() << std::endl;
		Logger::log(std::string("Unhandled exception: ") + ex.what());
		Logger::close();
		return 1;
	}
	catch (...) {
		std::cerr << "Unhandled unknown exception" << std::endl;
		Logger::log("Unhandled unknown exception");
		Logger::close();
		return 2;
	}
}
