#include "productsToJson.h"

using json = nlohmann::json;

json productsToJson(const std::vector<Product> &products) {
	json productsJson = json::array();

	for (const Product &product : products) {
		json productJson = json::object();
		json materialsJson = json::array();

		for (const Material &material : product.materialsConsumed) {
			json materialJson = json::object();

			materialJson["name"] = material.name;
			materialJson["value"] = material.value;
			if (material.units) {
				materialJson["units"] = *material.units;
			}
			if (material.decimalPart) {
				materialJson["decimalPart"] = *material.decimalPart;
			}

			materialsJson.push_back(materialJson);
		}

		productJson["mixedAt"] = product.mixedAt;

		if (product.mode) {
			productJson["mode"] = *product.mode;
		}

		if (product.moistureContent) {
			productJson["moistureContent"] = *product.moistureContent;
		}

		productJson["name"] = product.name;

		if (product.press) {
			productJson["press"] = *product.press;
		}

		if (product.producedRunningMeters) {
			productJson["producedRunningMeters"] = *product.producedRunningMeters;
		}

		productJson["totalWeight"] = product.totalWeight;

		productJson["materialsConsumed"] = materialsJson;
		
		productsJson.push_back(productJson);
	}

	return productsJson;
}