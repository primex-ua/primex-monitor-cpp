#include "seed.h"
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

using namespace std;

typedef struct
{
	int press;
	string name;
	double component_1;
	double component_2;
	double component_3;
	double component_4;
	double component_5;
	double component_6;
	double water;
	double total_weight;
	double specific_weight;
	double moisture_content;
	string mode;
} Product;

int insert_product(sqlite3 *db, Product product);
double get_random_number(double min, double max, int decimal_places);
Product generate_product();


void seed()
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

	Product product = generate_product();

	result_code = insert_product(db, product);
	if (result_code != SQLITE_OK)
	{
		sqlite3_close(db);
		return;
	}

	cout << "Added new record: " << product.name << ", прес №" << product.press << ", режим " << product.mode << endl;

	result_code = sqlite3_close(db);
	if (result_code != SQLITE_OK)
	{
		fprintf(stderr, "Error closing database: %s\n", sqlite3_errmsg(db));
	}
}

int insert_product(sqlite3 *db, Product product)
{
	sqlite3_stmt *stmt;

	string sql_insert_product = R"(
		INSERT INTO products (
			press, name, component_1, component_2, component_3, component_4, component_5, component_6, water, total_weight, specific_weight, moisture_content, mode)
		VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?);
	)";


	int rc = sqlite3_prepare_v2(db, sql_insert_product.c_str(), -1, &stmt, NULL);
	if (rc != SQLITE_OK)
	{
		fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
		return rc;
	}

	if (product.press != -1) {
		sqlite3_bind_int(stmt, 1, product.press);
	}
	sqlite3_bind_text(stmt, 2, product.name.c_str(), -1, SQLITE_TRANSIENT);
	sqlite3_bind_double(stmt, 3, product.component_1);
	sqlite3_bind_double(stmt, 4, product.component_2);
	sqlite3_bind_double(stmt, 5, product.component_3);
	sqlite3_bind_double(stmt, 6, product.component_4);
	sqlite3_bind_double(stmt, 7, product.component_5);
	sqlite3_bind_double(stmt, 8, product.component_6);
	sqlite3_bind_double(stmt, 9, product.water);
	sqlite3_bind_double(stmt, 10, product.total_weight);
	if (product.specific_weight != -1) {
		sqlite3_bind_double(stmt, 11, product.specific_weight);
	}
	if (product.moisture_content != -1) {
		sqlite3_bind_double(stmt, 12, product.moisture_content);
	}
	if (product.mode != "UNDEFINED") {
		sqlite3_bind_text(stmt, 13, product.mode.c_str(), -1, SQLITE_TRANSIENT);
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

double get_random_number(double min, double max, int decimal_places)
{
	if (min > max || decimal_places < 0)
	{
		fprintf(stderr, "Invalid input values.\n");
		return -1;
	}

	double random_value = (double)rand() / RAND_MAX;

	double scaled_value = min + random_value * (max - min);

	double factor = pow(10, decimal_places);
	double rounded_value = round(scaled_value * factor) / factor;

	return rounded_value;
}

Product generate_product()
{
	vector<string> names = { "Без обліку", "Рецепт 1", "Рецепт 2", "Рецепт 3", "Рецепт 4", "Рецепт 5" };
	Product product;

	int index = static_cast<int>(get_random_number(0, names.size() - 1, 0));
	bool is_backup_record = index == 0;

	double mixer_volume = 5.0;

	double components[7];
	components[0] = mixer_volume * get_random_number(60.0, 100.0, 1);  // вода
	components[1] = mixer_volume * get_random_number(150.0, 200.0, 1); // відсів
	components[2] = mixer_volume * get_random_number(350.0, 450.0, 1); // пісок
	components[3] = mixer_volume * get_random_number(150.0, 200.0, 1); // щебень
	components[4] = mixer_volume * get_random_number(150.0, 200.0, 1); // щебень 2
	components[5] = mixer_volume * get_random_number(150.0, 200.0, 1); // цемент
	components[6] = mixer_volume * get_random_number(2.0, 15.0, 2);    // хім. добавки

	double total_weight = 0;
	for (int i = 0; i < 7; i++)
	{
		total_weight += components[i];
	}
	total_weight = round(total_weight * 10.0) / 10.0;

	product.name = names[index];																	// name
	product.press = is_backup_record ? -1 : (int)get_random_number(1, 5, 0);						// press
	product.mode = is_backup_record ? "UNDEFINED" : (get_random_number(1, 2, 0) == 1) ? "auto" : "manual";	// mode
	product.specific_weight = is_backup_record ? -1 : get_random_number(500, 2500, 0);				// specific_weight
	product.moisture_content = is_backup_record ? -1 : get_random_number(1, 20, 2);					// moisture_content
	product.component_1 = components[1];															// component_1
	product.component_2 = components[2];															// component_2
	product.component_3 = components[3];															// component_3
	product.component_4 = components[4];															// component_4
	product.component_5 = components[5];															// component_5
	product.component_6 = components[6];															// component_6
	product.water = components[0];																	// water
	product.total_weight = total_weight;															// total_weight

	return product;
}
