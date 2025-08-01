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