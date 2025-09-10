#include "SQLQueries.h"
#include <string>

using namespace std;

const string SQL_CREATE_TABLES = R"(
	CREATE TABLE IF NOT EXISTS "products" (
		"id"							INTEGER NOT NULL,
		"mixed_at"						TIMESTAMP NOT NULL DEFAULT (DATETIME('now', 'localtime')),
		"name"							TEXT NOT NULL,
		"line_name"						TEXT,
		"press"							INTEGER,
		"total_weight"					REAL NOT NULL,
		"moisture_content_target"		REAL,
		"moisture_content_measured"		REAL,
		"temperature"					REAL,
		"output"						REAL,
		"water_correction_factor"		REAL,
		"components"					TEXT CHECK (json_valid(components)), 
		PRIMARY KEY("id" AUTOINCREMENT)
	);

	CREATE TABLE IF NOT EXISTS "components" (
		"id"			INTEGER NOT NULL,
		"updated_at"	TIMESTAMP NOT NULL DEFAULT (DATETIME('now', 'localtime')),
		"name"			TEXT NOT NULL UNIQUE,
		"weight"		REAL NOT NULL,
		PRIMARY KEY("id" AUTOINCREMENT)
	);

	CREATE TABLE IF NOT EXISTS "component_transactions" (
		"id"				INTEGER NOT NULL,
		"timestamp"			TIMESTAMP NOT NULL DEFAULT (DATETIME('now', 'localtime')),
		"component_name"	TEXT NOT NULL,
		"amount"			REAL NOT NULL,
		PRIMARY KEY("id" AUTOINCREMENT)
	);
)";

const string SQL_SELECT_COMPONENTS = R"(
	SELECT json_object(
		'id', id,
		'name', name,
		'weight', weight,
		'updated_at', updated_at
	) AS component
	FROM components
	WHERE ? IS NULL
		OR EXISTS (
			SELECT 1
			FROM components c
			WHERE c.updated_at > ?
		);
)";

const string SQL_SELECT_PRODUCTS = R"(
	SELECT json_object(
		'mixedAt', products.mixed_at,
		'name', products.name,
		'lineName', products.line_name,
		'press', products.press,
		'totalWeight', products.total_weight,
		'moistureContentTarget', products.moisture_content_target,
		'moistureContentMeasured', products.moisture_content_measured,
		'temperature', products.temperature,
		'output', products.output,
		'waterCorrectionFactor', products.water_correction_factor,
		'components', products.components
	) AS product
	FROM products
	WHERE products.mixed_at > COALESCE(?, DATETIME('now', ?))
	ORDER BY products.mixed_at ASC, products.line_name ASC;
)";

const string SQL_SELECT_TRANSACTIONS = R"(
	SELECT json_object(
		'id', id,
		'componentName', component_name,
		'timestamp', timestamp,
		'amount', amount
	) AS component_transaction
	FROM (
		SELECT *
		FROM component_transactions
		WHERE component_transactions.timestamp > COALESCE(?, DATETIME('now', ?))
		ORDER BY timestamp ASC
	);
)";