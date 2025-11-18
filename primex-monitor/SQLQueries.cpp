#include "SQLQueries.h"
#include <string>

using namespace std;

const string SQL_CREATE_TABLES = R"(
	CREATE TABLE IF NOT EXISTS "db_metadata" (
		"key"		TEXT PRIMARY KEY,
		"value"		TEXT NOT NULL
	);

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
		"components"					TEXT DEFAULT '[]' CHECK (json_valid(components)),
		"additional_params"				TEXT CHECK ("additional_params" IS NULL OR json_valid("additional_params")),
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

const string SQL_INIT_DB_INSTANCE_ID = R"(
	INSERT OR IGNORE INTO db_metadata (key, value)
	VALUES ('instance_id', hex(randomblob(16)));
)";

const string SQL_SELECT_DB_INSTANCE_ID = R"(
	SELECT value FROM db_metadata WHERE key = 'instance_id';
)";

const string SQL_SELECT_COMPONENTS = R"(
	SELECT json_object(
		'id', id,
		'name', name,
		'weight', weight,
		'updated_at', updated_at
	) AS component
	FROM components
	WHERE
		EXISTS (
			SELECT 1
			FROM components c
			WHERE c.id > ?
		)
	ORDER BY components.id ASC;
)";

const string SQL_SELECT_PRODUCTS = R"(
	SELECT json_object(
		'id', products.id,
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
		'components', products.components,
		'metadata', products.additional_params
	) AS product
	FROM products
	WHERE products.id > ?
	ORDER BY products.id ASC;
)";

const string SQL_SELECT_TRANSACTIONS = R"(
	SELECT json_object(
		'id', id,
		'componentName', component_name,
		'timestamp', timestamp,
		'amount', amount
	) AS component_transaction
	FROM component_transactions
	WHERE component_transactions.id > ?
	ORDER BY component_transactions.id ASC;
)";