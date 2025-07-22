#include "addTransaction.h"
#include "path.h"
#include <windows.h>
#include "sqlite3.h"
#include <string>
#include <vector>
#include <cstdlib>
#include <ctime>
#include "INIReader.h"
#include <iostream>
#include <locale>
#include "json.hpp"

using namespace std;
using json = nlohmann::json;

int insertTransaction(sqlite3 *db, json transaction);
double getRandomNumber(double min, double max, int decimalPlaces);
json generateTransaction();


void addTransaction()
{
	string exeDir = getExecutableDir();
	string configPath = exeDir + "\\primex-monitor-config.ini";

	INIReader reader(configPath);
	
	if (reader.ParseError() < 0) {
		cout << "Can't load " << configPath << endl;
		throw runtime_error("Failed to load config file.");
	}

	if (!reader.HasValue("database", "path")) {
		throw runtime_error("Database path not specified in primex-monitor-config.ini");
	}

	string dbPath = reader.Get("database", "path", "UNDEFINED").c_str();

	sqlite3 *db;

	int result_code = sqlite3_open(dbPath.c_str(), &db);
	if (result_code != SQLITE_OK)
	{
		fprintf(stderr, "Error opening database: %s\n", sqlite3_errmsg(db));
		sqlite3_close(db);
		return;
	}

	json transaction = generateTransaction();

	result_code = insertTransaction(db, transaction);
	if (result_code != SQLITE_OK)
	{
		sqlite3_close(db);
		return;
	}

	cout << "Added new record:" << endl;
	cout << "  " << transaction["component_name"] << ": " << transaction["amount"] << endl;
	cout << endl;

	result_code = sqlite3_close(db);
	if (result_code != SQLITE_OK)
	{
		fprintf(stderr, "Error closing database: %s\n", sqlite3_errmsg(db));
	}
}

int insertTransaction(sqlite3 *db, json transaction)
{
	sqlite3_stmt *stmt;

	string sql_insert_transaction = R"(
		INSERT INTO component_transactions (component_name, amount) VALUES (?, ?);
	)";


	int rc = sqlite3_prepare_v2(db, sql_insert_transaction.c_str(), -1, &stmt, NULL);
	if (rc != SQLITE_OK)
	{
		fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
		return rc;
	}

	sqlite3_bind_text(stmt, 1, transaction["component_name"].get<string>().c_str(), -1, SQLITE_TRANSIENT);
	
	sqlite3_bind_double(stmt, 2, transaction["amount"]);

	rc = sqlite3_step(stmt);

	if (rc != SQLITE_DONE)
	{
		fprintf(stderr, "Error inserting transaction into database: %s\n", sqlite3_errmsg(db));
		return rc;
	}

	rc = sqlite3_finalize(stmt);

	return rc;
}

double getRandomNumber(double min, double max, int decimalPlaces)
{
	if (min > max || decimalPlaces < 0)
	{
		fprintf(stderr, "Invalid input values.\n");
		return -1;
	}

	double random_value = (double)rand() / RAND_MAX;

	double scaled_value = min + random_value * (max - min);

	double factor = pow(10, decimalPlaces);
	double rounded_value = round(scaled_value * factor) / factor;

	return rounded_value;
}

json generateTransaction()
{
	vector<string> componentNames = { "Відсів", "Пісок", "Щебінь", "Щебінь 2", "Цемент", "Хім. добавки" };
	
	int index = static_cast<int>(getRandomNumber(0, componentNames.size() - 1, 0));

	json transaction;
	transaction["component_name"] = componentNames[index];
	transaction["amount"] = getRandomNumber(-5000, 5000, 1);

	return transaction;
}
