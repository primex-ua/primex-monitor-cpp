#ifndef PRODUCTS_TO_JSON_H
#define PRODUCTS_TO_JSON_H

#include "data-processor.h"
#include "json.hpp"
#include <vector>

nlohmann::json productsToJson(const std::vector<Product> &products);

#endif // !PRODUCTS_TO_JSON_H