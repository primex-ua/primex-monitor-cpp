#define CURL_STATICLIB

#include <iostream>
#include <windows.h>

#include "sqlite-db.h"
#include "sqlite3.h"
#include "Cursor.h"
#include "Env.h"
#include "data-processor.h"
#include "productsToJson.h"
#include "sendData.h"

using namespace std;

void printTable(const Table &table) {
	for (const auto &row : table) {
		for (const auto &cell : row) {
			cout << cell.first << ": " << cell.second << endl;
		}
		cout << endl;
	}
}

void printProducts(const vector<Product>& products) {
	cout << "\nProducts:\n";
	for (const Product &product : products) {
		cout << "product.name = " << product.name << endl;
		cout << "product.press = " << (product.press ? to_string(*product.press) : "NULL") << endl;
		cout << "product.mixedAt = " << product.mixedAt << endl;
		cout << "product.mode = " << (product.mode ? *product.mode : "NULL") << endl;
		cout << "product.moistureContent = " << (product.moistureContent ? to_string(*product.moistureContent) : "NULL") << endl;
		cout << "product.producedRunningMeters = " << (product.producedRunningMeters ? to_string(*product.producedRunningMeters) : "NULL") << endl;
		cout << "product.totalWeight = " << product.totalWeight << endl;
		cout << "product.materialsConsumed = [" << endl;

		for (const Material &material : product.materialsConsumed) {
			cout << "    { " << "name: " << material.name << ", value: " << material.value << " }\n";
		}

		cout << "]" << endl;

	}
	cout << endl;
}


int main() {
	SetConsoleOutputCP(CP_UTF8);

	cout << "\n******************** Configuration ********************\n" << endl;
	cout << "Configuration" << endl;

	Env& env = Env::getEnv();
	SQLiteDB db(env.getDbPath());

	db.createTableIfNotExists();

	string sqlQueryProductsById = R"(
		SELECT * FROM products
		WHERE id > ?
		ORDER BY mixed_at ASC;
	)";
	sqlite3_stmt* idStmt = db.prepareStmt(sqlQueryProductsById.c_str());

	string sqlQueryProductsByMixedAt = R"(
		SELECT * FROM products
		WHERE mixed_at > ?
		ORDER BY mixed_at ASC;
	)";
	sqlite3_stmt* mixedAtStmt = db.prepareStmt(sqlQueryProductsByMixedAt.c_str());

	string sqlQuerySettings = R"(
		SELECT * FROM component_names;
	)";
	sqlite3_stmt* settingsStmt = db.prepareStmt(sqlQuerySettings.c_str());

	Table dbProducts;
	Table dbSettings;

	while (true) {
		cout << "\n=======================================================\n" << endl;
		cout << "Start reading from the database" << endl;

		string cursor = Cursor::getCursor();

		if (cursor != "UNDEFINED") {
			sqlite3_bind_text(idStmt, 1, cursor.c_str(), -1, SQLITE_STATIC);

			dbProducts = db.queryDatabase(idStmt);
		}
		else {
			string startingTimestamp = Cursor::getStartingTimestamp();

			sqlite3_bind_text(mixedAtStmt, 1, startingTimestamp.c_str(), -1, SQLITE_STATIC);

			dbProducts = db.queryDatabase(mixedAtStmt);
		}


		sqlite3_reset(idStmt);
		sqlite3_reset(mixedAtStmt);

		dbSettings = db.queryDatabase(settingsStmt);
		if (dbSettings.size() == 0) {
			throw runtime_error("Components names not specified.");
			continue;
		}

		cout << "Found " << dbProducts.size() << " new products.\n" << endl;

		if (dbProducts.size() > 0) {
			vector<Product> apiProducts = DataProcessor::transformDataToApiFormat(dbProducts, dbSettings);
			// printProducts(apiProducts);

			nlohmann::json productsJson = productsToJson(apiProducts);
			nlohmann::json postRequestData;
			postRequestData["data"] = productsJson;

			bool isRequestSuccess = sendData(env.getApiUrl(), env.getApiKey(), postRequestData.dump());

			if (isRequestSuccess) {
				cout << endl;
				Cursor::setCursor(dbProducts[dbProducts.size() - 1].at("id"));
			}
		}

		Sleep(10000);
	}

	sqlite3_finalize(idStmt);
	sqlite3_finalize(mixedAtStmt);

	return 0;
}
