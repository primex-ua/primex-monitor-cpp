#ifndef DATA_PROCESSOR_H
#define DATA_PROCESSOR_H

#include "sqlite-db.h"
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <vector>

struct Material {
	std::string name;
	double value;
	std::unique_ptr<std::string> units;
	std::unique_ptr<int> decimalPart;
};

struct Product {
	std::string name;
	std::string mixedAt;
	std::unique_ptr<int> press;
	std::unique_ptr<std::string> mode;
	double totalWeight;
	std::unique_ptr<double> moistureContent;
	std::unique_ptr<double> producedRunningMeters;
	std::vector<Material> materialsConsumed;
};

class DataProcessor {
public:
	static std::vector<Product> transformDataToApiFormat(const Table &products, const Table &componentNames);
};

#endif // DATA_PROCESSOR_H