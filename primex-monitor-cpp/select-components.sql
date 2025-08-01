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