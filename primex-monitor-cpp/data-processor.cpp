#include "data-processor.h"

std::vector<Product> DataProcessor::transformDataToApiFormat(const Table &products, const Table &componentNames) {
	std::vector<Product> apiProducts;

	for (const Row &row : products) {
		Product apiProduct;
		std::vector<Material> materials;
		
		if (row.at("component_1") != "NULL" && componentNames[0].at("component_1_name") != "NULL") {
			Material material;
			material.name = componentNames[0].at("component_1_name");
			material.value = std::stod(row.at("component_1"));
			materials.push_back(std::move(material));
		}
		if (row.at("component_2") != "NULL" && componentNames[0].at("component_2_name") != "NULL") {
			Material material;
			material.name = componentNames[0].at("component_2_name");
			material.value = std::stod(row.at("component_2"));
			materials.push_back(std::move(material));
		}
		if (row.at("component_3") != "NULL" && componentNames[0].at("component_3_name") != "NULL") {
			Material material;
			material.name = componentNames[0].at("component_3_name");
			material.value = std::stod(row.at("component_3"));
			materials.push_back(std::move(material));
		}
		if (row.at("component_4") != "NULL" && componentNames[0].at("component_4_name") != "NULL") {
			Material material;
			material.name = componentNames[0].at("component_4_name");
			material.value = std::stod(row.at("component_4"));
			materials.push_back(std::move(material));
		}
		if (row.at("component_5") != "NULL" && componentNames[0].at("component_5_name") != "NULL") {
			Material material;
			material.name = componentNames[0].at("component_5_name");
			material.value = std::stod(row.at("component_5"));
			materials.push_back(std::move(material));
		}
		if (row.at("component_6") != "NULL" && componentNames[0].at("component_6_name") != "NULL") {
			Material material;
			material.name = componentNames[0].at("component_6_name");
			material.value = std::stod(row.at("component_6"));
			materials.push_back(std::move(material));
		}

		Material water;
		water.name = "Вода";
		water.value = std::stod(row.at("water"));
		water.units = std::make_unique<std::string>("л");
		materials.push_back(std::move(water));

		apiProduct.name = row.at("name");
		apiProduct.mixedAt = row.at("mixed_at");
		apiProduct.press = row.at("press") != "NULL" ? std::make_unique<int>(std::stoi(row.at("press"))) : nullptr;
		apiProduct.mode = row.at("mode") != "NULL" ? std::make_unique<std::string>(row.at("mode")) : nullptr;
		apiProduct.totalWeight = std::stod(row.at("total_weight"));
		apiProduct.moistureContent = row.at("moisture_content") != "NULL" ? std::make_unique<double>(std::stod(row.at("moisture_content"))) : nullptr;
		apiProduct.producedRunningMeters = row.at("specific_weight") != "NULL" ? std::make_unique<double>(apiProduct.totalWeight / std::stod(row.at("specific_weight"))) : nullptr;
		apiProduct.materialsConsumed = std::move(materials);

		apiProducts.push_back(std::move(apiProduct));
	}

	return apiProducts;
}
