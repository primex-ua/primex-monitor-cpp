#include "addProduct.h"
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

int insertProduct(sqlite3 *db, json product);
double getRandomNumber(double min, double max, int decimalPlaces);
json generateProduct();


void addProduct()
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

	json product = generateProduct();

	result_code = insertProduct(db, product);
	if (result_code != SQLITE_OK)
	{
		sqlite3_close(db);
		return;
	}

	cout << "Added new record:" << endl;
	cout << "  " << product["name"] << ", лінія: " << product["line_name"] << endl;
	cout << endl;

	result_code = sqlite3_close(db);
	if (result_code != SQLITE_OK)
	{
		fprintf(stderr, "Error closing database: %s\n", sqlite3_errmsg(db));
	}
}

int insertProduct(sqlite3 *db, json product)
{
	sqlite3_stmt *stmt;

	string sql_insertProduct = R"(
		INSERT INTO products (
			name,
			line_name,
			press,
			total_weight,
			moisture_content_target,
			moisture_content_measured,
			temperature,
			output,
			water_correction_factor,
			components,
			additional_params
		)
		VALUES (
			?,	-- name
			?,	-- line_name
			?,	-- press
			?,	-- total_weight
			?,	-- moisture_content_target
			?,	-- moisture_content_measured
			?,	-- temperature
			?,	-- output
			?,  -- water_correction_factor
			?,  -- components
			?	-- additional_params
		);
	)";


	int rc = sqlite3_prepare_v2(db, sql_insertProduct.c_str(), -1, &stmt, NULL);
	if (rc != SQLITE_OK)
	{
		fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
		return rc;
	}

	sqlite3_bind_text(stmt, 1, product["name"].get<string>().c_str(), -1, SQLITE_TRANSIENT);

	sqlite3_bind_text(stmt, 2, product["line_name"].get<string>().c_str(), -1, SQLITE_TRANSIENT);

	if (product.contains("press")) {
		sqlite3_bind_int(stmt, 3, product["press"]);
	}
	else {
		sqlite3_bind_null(stmt, 3);
	}

	sqlite3_bind_double(stmt, 4, product["total_weight"]);

	if (product.contains("moisture_content_target")) {
		sqlite3_bind_double(stmt, 5, product["moisture_content_target"]);
	}
	else {
		sqlite3_bind_null(stmt, 5);
	}

	if (product.contains("moisture_content_measured")) {
		sqlite3_bind_double(stmt, 6, product["moisture_content_measured"]);
	}
	else {
		sqlite3_bind_null(stmt, 6);
	}

	if (product.contains("temperature")) {
		sqlite3_bind_double(stmt, 7, product["temperature"]);
	}
	else {
		sqlite3_bind_null(stmt, 7);
	}

	if (product.contains("output")) {
		sqlite3_bind_double(stmt, 8, product["output"]);
	}
	else {
		sqlite3_bind_null(stmt, 8);
	}

	if (product.contains("water_correction_factor")) {
		sqlite3_bind_double(stmt, 9, product["water_correction_factor"]);
	}
	else {
		sqlite3_bind_null(stmt, 9);
	}

	if (product.contains("components")) {
		sqlite3_bind_text(stmt, 10, product["components"].dump().c_str(), -1, SQLITE_TRANSIENT);
	}
	else {
		sqlite3_bind_null(stmt, 10);
	}

	if (product.contains("additional_params")) {
		sqlite3_bind_text(stmt, 11, product["additional_params"].dump().c_str(), -1, SQLITE_TRANSIENT);
	}
	else {
		sqlite3_bind_null(stmt, 11);
	}

	rc = sqlite3_step(stmt);

	if (rc != SQLITE_DONE)
	{
		fprintf(stderr, "Error inserting product into database: %s\n", sqlite3_errmsg(db));
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

json generateProduct()
{
	vector<string> names = { "Облік не проводився", "Рецепт 1", "Рецепт 2", "Рецепт 3", "Рецепт 4", "Рецепт 5" };
	vector<string> lineNames = { "О", "Ф", "О1", "О2", "Ф1", "Ф2" };
	vector<string> componentNames = { "Відсів", "Пісок", "Щебінь", "Щебінь 2", "Цемент", "Хім. добавки", "Вода" };
	json product;

	int index = static_cast<int>(getRandomNumber(0, names.size() - 1, 0));
	bool isBackupRecord = index == 0;

	double mixerVolume = 5.0;
	double specificWeight = 2500; // kg/m^3

	double componentsWeights[7];
	componentsWeights[0] = mixerVolume * getRandomNumber(150.0, 200.0, 1); // відсів
	componentsWeights[1] = mixerVolume * getRandomNumber(350.0, 450.0, 1); // пісок
	componentsWeights[2] = mixerVolume * getRandomNumber(150.0, 200.0, 1); // щебінь
	componentsWeights[3] = mixerVolume * getRandomNumber(150.0, 200.0, 1); // щебінь 2
	componentsWeights[4] = mixerVolume * getRandomNumber(150.0, 200.0, 1); // цемент
	componentsWeights[5] = mixerVolume * getRandomNumber(2.0, 15.0, 2);    // хім. добавки
	componentsWeights[6] = mixerVolume * getRandomNumber(60.0, 100.0, 1);  // вода

	double totalWeight = 0;
	for (int i = 0; i < 7; i++)
	{
		totalWeight += componentsWeights[i];
	}
	totalWeight = round(totalWeight * 10.0) / 10.0;

	product["name"] = names[index];
	product["line_name"] = lineNames[(int)getRandomNumber(0, lineNames.size() - 1, 0)];
	product["total_weight"] = totalWeight;

	json components = json::array();

	int count = 0;

	for (const std::string& componentName : componentNames) {
		if (count < 4) {
			components.push_back({
				{"name", componentName},
				{"weight", componentsWeights[count]},
				{"moistureContent", getRandomNumber(1, 25, 1)}
			});
		}
		else {
			components.push_back({
				{"name", componentName},
				{"weight", componentsWeights[count]},
			});
		}

		count++;
	}

	product["components"] = components;

	if (!isBackupRecord) {
		product["press"] = (int)getRandomNumber(1, 5, 0);
		product["moisture_content_target"] = getRandomNumber(1, 20, 2);
		product["moisture_content_measured"] = getRandomNumber(1, 20, 2);
		product["temperature"] = getRandomNumber(0, 40, 1);
		product["output"] = round((totalWeight / specificWeight) * 100.0) / 100.0;
		product["water_correction_factor"] = getRandomNumber(0, 1, 3);
		
		json params;
		params["mode"] = (int)getRandomNumber(1, 7, 0);
		product["additional_params"] = params;
	}

	return product;
}
