#include "addComponents.h"
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

int insertComponents(sqlite3 *db, json product);
double getRandomNumber(double min, double max, int decimalPlaces);
json generateComponents();


void addComponents()
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
	char* errMsg = nullptr;

	int result_code = sqlite3_open(dbPath.c_str(), &db);
	if (result_code != SQLITE_OK)
	{
		fprintf(stderr, "Error opening database: %s\n", sqlite3_errmsg(db));
		sqlite3_close(db);
		return;
	}

	const char* deleteSQL = "DELETE FROM components;";
	if (sqlite3_exec(db, deleteSQL, nullptr, nullptr, &errMsg) != SQLITE_OK) {
		std::cerr << "Error deleting entries: " << errMsg << "\n";
		sqlite3_free(errMsg);
	}

	json components = generateComponents();

	result_code = insertComponents(db, components);
	if (result_code != SQLITE_OK)
	{
		sqlite3_close(db);
		return;
	}

	cout << "Updated components:" << endl;

	for (const auto& component : components) {
		cout << "  " << component["name"] << ": " << component["weight"] << endl;
	}

	cout << endl;

	result_code = sqlite3_close(db);
	if (result_code != SQLITE_OK)
	{
		fprintf(stderr, "Error closing database: %s\n", sqlite3_errmsg(db));
	}
}

int insertComponents(sqlite3 *db, json components)
{
	sqlite3_stmt *stmt;

	string sql_insert_component = R"(
		INSERT INTO components (name, weight) VALUES (?, ?);
	)";


	int rc = sqlite3_prepare_v2(db, sql_insert_component.c_str(), -1, &stmt, NULL);
	if (rc != SQLITE_OK) {
		fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
		return rc;
	}

	for (const auto& component : components) {
		sqlite3_bind_text(stmt, 1, component["name"].get<string>().c_str(), -1, SQLITE_TRANSIENT);
		sqlite3_bind_double(stmt, 2, component["weight"]);


		rc = sqlite3_step(stmt);
		if (rc != SQLITE_DONE) {
			fprintf(stderr, "Error inserting component into database: %s\n", sqlite3_errmsg(db));
			return rc;
		}

		sqlite3_reset(stmt);
		sqlite3_clear_bindings(stmt);
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

json generateComponents()
{
	vector<string> componentNames = {"Відсів", "Пісок", "Щебінь", "Щебінь 2", "Цемент", "Хім. добавки" };

	json components = json::array();

	for (const auto& name : componentNames) {
		components.push_back({ {"name", name}, {"weight", getRandomNumber(0, 100000, 0)} });
	}

	return components;
}
