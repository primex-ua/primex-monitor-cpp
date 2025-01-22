#define CURL_STATICLIB

#include <iostream>
#include <windows.h>
#include "sqlite-db.h"
#include "sqlite3.h"
#include "Cursor.h"
#include "Env.h"
#include "sendData.h"
#include "json.hpp"

using namespace std;
using json = nlohmann::json;

int main() {
	SetConsoleOutputCP(CP_UTF8);

	cout << "\n******************** Configuration ********************\n" << endl;
	cout << "Configuration" << endl;

	Env& env = Env::getEnv();
	SQLiteDB db(env.getDbPath());

	db.createTableIfNotExists();

	string sqlQueryProductsByCursor = R"(
		SELECT json_object(
			'mixedAt', products.mixed_at,
			'name', products.name,
			'lineName', products.line_name,
			'press', products.press,
			'totalWeight', products.total_weight,
			'moistureContent', products.moisture_content,
			'produced', json_object(
				'amount', products.produced_amount,
				'units', products.produced_units
			),
			'components', json_group_array(
				json_object(
					'name', components.name,
					'value', components.value
				)
			)
		) AS product
		FROM products
		JOIN (
			SELECT 
				products.id AS product_id,
				products.component_1_name AS name, 
				products.component_1_weight AS value
			FROM products
			WHERE products.component_1_name IS NOT NULL AND products.component_1_weight IS NOT NULL

			UNION ALL

			SELECT 
				products.id AS product_id,
				products.component_2_name AS name, 
				products.component_2_weight AS value
			FROM products
			WHERE products.component_2_name IS NOT NULL AND products.component_2_weight IS NOT NULL

			UNION ALL

			SELECT 
				products.id AS product_id,
				products.component_3_name AS name, 
				products.component_3_weight AS value
			FROM products
			WHERE products.component_3_name IS NOT NULL AND products.component_3_weight IS NOT NULL

			UNION ALL

			SELECT 
				products.id AS product_id,
				products.component_4_name AS name, 
				products.component_4_weight AS value
			FROM products
			WHERE products.component_4_name IS NOT NULL AND products.component_4_weight IS NOT NULL

			UNION ALL

			SELECT 
				products.id AS product_id,
				products.component_5_name AS name, 
				products.component_5_weight AS value
			FROM products
			WHERE products.component_5_name IS NOT NULL AND products.component_5_weight IS NOT NULL

			UNION ALL

			SELECT 
				products.id AS product_id,
				products.component_6_name AS name, 
				products.component_6_weight AS value
			FROM products
			WHERE products.component_6_name IS NOT NULL AND products.component_6_weight IS NOT NULL

			UNION ALL

			SELECT 
				products.id AS product_id,
				'Вода' AS name, 
				products.water_weight AS value
			FROM products
			WHERE products.water_weight IS NOT NULL
		) AS components
		ON products.id = components.product_id
		WHERE products.mixed_at > ? 
		   OR (products.mixed_at = ? AND COALESCE(products.line_name, '') > COALESCE(?, ''))
		GROUP BY products.id
		ORDER BY products.mixed_at ASC, products.line_name ASC;
	)";
	sqlite3_stmt* stmtByCursor = db.prepareStmt(sqlQueryProductsByCursor.c_str());
	
	string sqlQueryProductsByPeriod = R"(
		SELECT json_object(
			'mixedAt', products.mixed_at,
			'name', products.name,
			'lineName', products.line_name,
			'press', products.press,
			'totalWeight', products.total_weight,
			'moistureContent', products.moisture_content,
			'produced', json_object(
				'amount', products.produced_amount,
				'units', products.produced_units
			),
			'components', json_group_array(
				json_object(
					'name', components.name,
					'value', components.value
				)
			)
		) AS product
		FROM products
		JOIN (
			SELECT 
				products.id AS product_id,
				products.component_1_name AS name, 
				products.component_1_weight AS value
			FROM products
			WHERE products.component_1_name IS NOT NULL AND products.component_1_weight IS NOT NULL

			UNION ALL

			SELECT 
				products.id AS product_id,
				products.component_2_name AS name, 
				products.component_2_weight AS value
			FROM products
			WHERE products.component_2_name IS NOT NULL AND products.component_2_weight IS NOT NULL

			UNION ALL

			SELECT 
				products.id AS product_id,
				products.component_3_name AS name, 
				products.component_3_weight AS value
			FROM products
			WHERE products.component_3_name IS NOT NULL AND products.component_3_weight IS NOT NULL

			UNION ALL

			SELECT 
				products.id AS product_id,
				products.component_4_name AS name, 
				products.component_4_weight AS value
			FROM products
			WHERE products.component_4_name IS NOT NULL AND products.component_4_weight IS NOT NULL

			UNION ALL

			SELECT 
				products.id AS product_id,
				products.component_5_name AS name, 
				products.component_5_weight AS value
			FROM products
			WHERE products.component_5_name IS NOT NULL AND products.component_5_weight IS NOT NULL

			UNION ALL

			SELECT 
				products.id AS product_id,
				products.component_6_name AS name, 
				products.component_6_weight AS value
			FROM products
			WHERE products.component_6_name IS NOT NULL AND products.component_6_weight IS NOT NULL

			UNION ALL

			SELECT 
				products.id AS product_id,
				'Вода' AS name, 
				products.water_weight AS value
			FROM products
			WHERE products.water_weight IS NOT NULL
		) AS components
		ON products.id = components.product_id
		WHERE products.mixed_at > DATETIME('now', ?) 
		   OR (products.mixed_at = DATETIME('now', ?) AND COALESCE(products.line_name, '') > COALESCE(?, ''))
		GROUP BY products.id
		ORDER BY products.mixed_at ASC, products.line_name ASC;
	)";
	sqlite3_stmt* stmtByPeriod = db.prepareStmt(sqlQueryProductsByPeriod.c_str());

	while (true) {
		cout << "\n=======================================================\n" << endl;
		cout << "Start reading from the database" << endl;

		json cursor = Cursor::getCursor();
		Table result;
		json products;

		if (cursor["hasCursor"].get<bool>()) {
			sqlite3_bind_text(stmtByCursor, 1, cursor["mixed_at"].get<string>().c_str(), -1, SQLITE_TRANSIENT);
			sqlite3_bind_text(stmtByCursor, 2, cursor["mixed_at"].get<string>().c_str(), -1, SQLITE_TRANSIENT);
			sqlite3_bind_text(stmtByCursor, 3, cursor["line_name"].get<string>().c_str(), -1, SQLITE_TRANSIENT);

			result = db.queryDatabase(stmtByCursor);
		}
		else {
			string period = "-" + to_string(env.getSyncPeriodDays()) + " days";
			sqlite3_bind_text(stmtByPeriod, 1, period.c_str(), -1, SQLITE_TRANSIENT);
			sqlite3_bind_text(stmtByPeriod, 2, period.c_str(), -1, SQLITE_TRANSIENT);
			sqlite3_bind_null(stmtByPeriod, 3);

			result = db.queryDatabase(stmtByPeriod);
		}

		sqlite3_reset(stmtByCursor);
		sqlite3_reset(stmtByPeriod);

		cout << "Found " << result.size() << " new products" << endl;

		if (result.size() > 0) {
			for (const Row& row: result) {
				if (row.find("product") != row.end()) {
					products.push_back(json::parse(row.at("product")));
				}
			}

			nlohmann::json postRequestData;
			postRequestData["data"] = products;

			bool isRequestSuccess = sendData(env.getApiUrl(), env.getApiKey(), postRequestData.dump());

			if (isRequestSuccess) {
				cout << endl;
				json lastProduct = products.back();

				json cursor;
				cursor["mixed_at"] = lastProduct["mixedAt"];
				if (lastProduct.contains("lineName") && lastProduct["lineName"].is_string()) {
					cursor["line_name"] = lastProduct["lineName"];
				}

				Cursor::setCursor(cursor);
			}
		}

		Sleep(10000);
	}

	sqlite3_finalize(stmtByCursor);
	sqlite3_finalize(stmtByPeriod);

	return 0;
}
