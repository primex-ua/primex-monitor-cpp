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
	string configPath = exeDir + "\\config.ini";

	INIReader reader(configPath);
	
	if (reader.ParseError() < 0) {
		cout << "Can't load " << configPath << endl;
		throw runtime_error("Failed to load config file.");
	}

	if (!reader.HasValue("database", "path")) {
		throw runtime_error("Database path not specified in config.ini");
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

	cout << "Added new record: " << product["name"] << ", лінія: " << product["line_name"] << endl;

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
			moisture_content,
			output_value,
			output_units,
			water_weight,
			component_1_weight,
			component_1_name,
			component_2_weight,
			component_2_name,
			component_3_weight,
			component_3_name,
			component_4_weight,
			component_4_name,
			component_5_weight,
			component_5_name,
			component_6_weight,
			component_6_name
		)
		VALUES (
			?,	-- name
			?,	-- line_name
			?,	-- press
			?,	-- total_weight
			?,	-- moisture_content
			?,	-- output_value
			?,	-- output_units
			?,	-- water_weight
			?,	-- component_1_weight
			?,	-- component_1_name
			?,	-- component_2_weight
			?,	-- component_2_name
			?,	-- component_3_weight
			?,	-- component_3_name
			?,	-- component_4_weight
			?,	-- component_4_name
			?,	-- component_5_weight
			?,	-- component_5_name
			?,	-- component_6_weight
			?	-- component_6_name
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
	if (product.contains("moisture_content")) {
		sqlite3_bind_double(stmt, 5, product["moisture_content"]);
	}
	else {
		sqlite3_bind_null(stmt, 5);
	}
	if (product.contains("output_value") && product.contains("output_units")) {
		sqlite3_bind_double(stmt, 6, product["output_value"]);
		sqlite3_bind_text(stmt, 7, product["output_units"].get<string>().c_str(), -1, SQLITE_TRANSIENT);
	}
	else {
		sqlite3_bind_null(stmt, 6);
		sqlite3_bind_null(stmt, 7);
	}
	sqlite3_bind_double(stmt, 8, product["water_weight"]);
	if (product.contains("component_1_weight") && product.contains("component_1_name")) {
		sqlite3_bind_double(stmt, 9, product["component_1_weight"]);
		sqlite3_bind_text(stmt, 10, product["component_3_name"].get<string>().c_str(), -1, SQLITE_TRANSIENT);
	}
	else {
		sqlite3_bind_null(stmt, 9);
		sqlite3_bind_null(stmt, 10);
	}
	if (product.contains("component_2_weight") && product.contains("component_2_name")) {
		sqlite3_bind_double(stmt, 11, product["component_2_weight"]);
		sqlite3_bind_text(stmt, 12, product["component_2_name"].get<string>().c_str(), -1, SQLITE_TRANSIENT);
	}
	else {
		sqlite3_bind_null(stmt, 11);
		sqlite3_bind_null(stmt, 12);
	}
	if (product.contains("component_3_weight") && product.contains("component_3_name")) {
		sqlite3_bind_double(stmt, 13, product["component_3_weight"]);
		sqlite3_bind_text(stmt, 14, product["component_3_name"].get<string>().c_str(), -1, SQLITE_TRANSIENT);
	}
	else {
		sqlite3_bind_null(stmt, 13);
		sqlite3_bind_null(stmt, 14);
	}
	if (product.contains("component_4_weight") && product.contains("component_4_name")) {
		sqlite3_bind_double(stmt, 15, product["component_4_weight"]);
		sqlite3_bind_text(stmt, 16, product["component_4_name"].get<string>().c_str(), -1, SQLITE_TRANSIENT);
	}
	else {
		sqlite3_bind_null(stmt, 15);
		sqlite3_bind_null(stmt, 16);
	}
	if (product.contains("component_5_weight") && product.contains("component_5_name")) {
		sqlite3_bind_double(stmt, 17, product["component_5_weight"]);
		sqlite3_bind_text(stmt, 18, product["component_5_name"].get<string>().c_str(), -1, SQLITE_TRANSIENT);
	}
	else {
		sqlite3_bind_null(stmt, 17);
		sqlite3_bind_null(stmt, 18);
	}
	if (product.contains("component_6_weight") && product.contains("component_6_name")) {
		sqlite3_bind_double(stmt, 19, product["component_6_weight"]);
		sqlite3_bind_text(stmt, 20, product["component_6_name"].get<string>().c_str(), -1, SQLITE_TRANSIENT);
	}
	else {
		sqlite3_bind_null(stmt, 19);
		sqlite3_bind_null(stmt, 20);
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
	json product;

	int index = static_cast<int>(getRandomNumber(0, names.size() - 1, 0));
	bool isBackupRecord = index == 0;

	double mixerVolume = 5.0;
	double specificWeight = 2500; // kg/m^3

	double components[7];
	components[0] = mixerVolume * getRandomNumber(60.0, 100.0, 1);  // вода
	components[1] = mixerVolume * getRandomNumber(150.0, 200.0, 1); // відсів
	components[2] = mixerVolume * getRandomNumber(350.0, 450.0, 1); // пісок
	components[3] = mixerVolume * getRandomNumber(150.0, 200.0, 1); // щебень
	components[4] = mixerVolume * getRandomNumber(150.0, 200.0, 1); // щебень 2
	components[5] = mixerVolume * getRandomNumber(150.0, 200.0, 1); // цемент
	components[6] = mixerVolume * getRandomNumber(2.0, 15.0, 2);    // хім. добавки

	double totalWeight = 0;
	for (int i = 0; i < 7; i++)
	{
		totalWeight += components[i];
	}
	totalWeight = round(totalWeight * 10.0) / 10.0;

	product["mixed_at"] = "CURRENT_TIMESTAMP";
	product["name"] = names[index];
	product["line_name"] = lineNames[(int)getRandomNumber(0, lineNames.size() -1, 0)];
	product["total_weight"] = totalWeight;
	product["water_weight"] = components[0];
	product["component_1_weight"] = components[1];
	product["component_1_name"] = "Відсів";
	product["component_2_weight"] = components[2];
	product["component_2_name"] = "Пісок";
	product["component_3_weight"] = components[3];
	product["component_3_name"] = "Щебень";
	product["component_4_weight"] = components[4];
	product["component_4_name"] = "Щебень 2";
	product["component_5_weight"] = components[5];
	product["component_5_name"] = "Цемент";
	product["component_6_weight"] = components[6];
	product["component_6_name"] = "Хім. добавки";

	if (!isBackupRecord) {
		product["press"] = (int)getRandomNumber(1, 5, 0);
		product["moisture_content"] = getRandomNumber(1, 20, 2);
		product["output_value"] = round((totalWeight / specificWeight) * 100.0) / 100.0;
		product["output_units"] = "пог.м";
	}

	return product;
}
