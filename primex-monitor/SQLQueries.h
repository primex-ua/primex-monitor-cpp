#ifndef SQL_QUERIES_H
#define SQL_QUERIES_H

#include <string>

using namespace std;

string SQL_SELECT_COMPONENTS = R"(
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

string SQL_SELECT_PRODUCTS = R"(
	SELECT json_object(
		'mixedAt', products.mixed_at,
		'name', products.name,
		'lineName', products.line_name,
		'press', products.press,
		'totalWeight', products.total_weight,
		'moistureContent', products.moisture_content,
		'temperature', products.temperature,
		'output', products.output,
		'components', json_group_array(
			json_object(
				'name', components.name,
				'value', components.value,
				'source', components.source
			)
		)
	) AS product
	FROM products
	JOIN (
		SELECT 
			products.id AS product_id,
			1 AS source,
			products.component_1_name AS name, 
			products.component_1_weight AS value
		FROM products
		WHERE products.component_1_name IS NOT NULL AND products.component_1_weight IS NOT NULL

		UNION ALL

		SELECT 
			products.id AS product_id,
			2 AS source,
			products.component_2_name AS name, 
			products.component_2_weight AS value
		FROM products
		WHERE products.component_2_name IS NOT NULL AND products.component_2_weight IS NOT NULL

		UNION ALL

		SELECT 
			products.id AS product_id,
			3 AS source,
			products.component_3_name AS name, 
			products.component_3_weight AS value
		FROM products
		WHERE products.component_3_name IS NOT NULL AND products.component_3_weight IS NOT NULL

		UNION ALL

		SELECT 
			products.id AS product_id,
			4 AS source,
			products.component_4_name AS name, 
			products.component_4_weight AS value
		FROM products
		WHERE products.component_4_name IS NOT NULL AND products.component_4_weight IS NOT NULL

		UNION ALL

		SELECT 
			products.id AS product_id,
			5 AS source,
			products.component_5_name AS name, 
			products.component_5_weight AS value
		FROM products
		WHERE products.component_5_name IS NOT NULL AND products.component_5_weight IS NOT NULL

		UNION ALL

		SELECT 
			products.id AS product_id,
			6 AS source,
			products.component_6_name AS name, 
			products.component_6_weight AS value
		FROM products
		WHERE products.component_6_name IS NOT NULL AND products.component_6_weight IS NOT NULL

		UNION ALL

		SELECT 
			products.id AS product_id,
			7 AS source,
			'Вода' AS name, 
			products.water_weight AS value
		FROM products
		WHERE products.water_weight IS NOT NULL
	) AS components
	ON products.id = components.product_id
	WHERE products.mixed_at > COALESCE(?, DATETIME('now', ?))
	GROUP BY products.id
	ORDER BY products.mixed_at ASC, products.line_name ASC;
)";

string SQL_SELECT_TRANSACTIONS = R"(
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

#endif // !SQL_QUERIES_H
