#include <iostream>
#include <map>
#include <memory>
#include <optional>
#include <sqlite3.h>
#include <stdexcept>
#include <string>
#include <vector>

using Row = std::map<std::string, std::string>;
using Table = std::vector<Row>;

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

class SQLiteDB {
public:
    explicit SQLiteDB(const std::string &dbPath) {
        int resultCode = sqlite3_open(dbPath.c_str(), &db_);
        if (resultCode != SQLITE_OK) {
            throw std::runtime_error("Error opening database: " + std::string(sqlite3_errmsg(db_)));
        }
        std::cout << "Opened database connection.\n";
    }

    ~SQLiteDB() {
        if (db_) {
            sqlite3_close(db_);
        }
    }

    void createTableIfNotExists() {
        const char *sqlCreateTables = R"(
                CREATE TABLE IF NOT EXISTS products (
                    id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
                    mixed_at INTEGER DEFAULT (unixepoch()) NOT NULL,
                    press INTEGER,
                    name TEXT NOT NULL,
                    component_1 REAL,
                    component_2 REAL,
                    component_3 REAL,
                    component_4 REAL,
                    component_5 REAL,
                    component_6 REAL,
                    water REAL NOT NULL,
                    total_weight REAL NOT NULL,
                    specific_weight REAL,
                    moisture_content REAL,
                    mode TEXT CHECK (mode IN ('auto', 'manual'))
                );

                CREATE TABLE IF NOT EXISTS settings ( 
                    component_1_name TEXT,
                    component_2_name TEXT,
                    component_3_name TEXT,
                    component_4_name TEXT,
                    component_5_name TEXT,
                    component_6_name TEXT
                );
            )";

        char *errMsg = nullptr;
        int rc = sqlite3_exec(db_, sqlCreateTables, nullptr, nullptr, &errMsg);
        if (rc != SQLITE_OK) {
            std::string errorMsg = "SQL error: " + std::string(errMsg);
            sqlite3_free(errMsg);
            throw std::runtime_error(errorMsg);
        }
        std::cout << "Table created or already exists.\n";
    }

    Table queryDatabase(const std::string &sqlQuery) {
        sqlite3_stmt *stmt;
        int rc = sqlite3_prepare_v2(db_, sqlQuery.c_str(), -1, &stmt, nullptr);
        if (rc != SQLITE_OK) {
            throw std::runtime_error("Failed to prepare statement: " + std::string(sqlite3_errmsg(db_)));
        }

        Table results;
        while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
            Row row;
            int columnCount = sqlite3_column_count(stmt);

            for (int i = 0; i < columnCount; ++i) {
                const char *columnName = sqlite3_column_name(stmt, i);

                if (sqlite3_column_type(stmt, i) != SQLITE_NULL) {
                    row[columnName] = reinterpret_cast<const char *>(sqlite3_column_text(stmt, i));
                } else {
                    row[columnName] = "NULL";
                }
            }
            results.push_back(row);
        }

        if (rc != SQLITE_DONE) {
            std::string errorMsg = "Error during execution: " + std::string(sqlite3_errmsg(db_));
            sqlite3_finalize(stmt);
            throw std::runtime_error(errorMsg);
        }

        sqlite3_finalize(stmt);
        return results;
    }

private:
    sqlite3 *db_;
};

template <typename T, typename... Args>
std::unique_ptr<T> makeUnique(Args &&...args) {
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

void printTable(const Table &table) {
    for (const auto &row : table) {
        for (const auto &cell : row) {
            std::cout << cell.first << ": " << cell.second << std::endl;
        }
        std::cout << std::endl;
    }
}

std::vector<Product> transformDataToApiFormat(const Table products, const Table componentNames) {
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
        water.name = "Water";
        water.value = std::stod(row.at("water"));
        water.units = makeUnique<std::string>("l");
        materials.push_back(std::move(water));

        apiProduct.name = row.at("name");
        apiProduct.mixedAt = row.at("mixed_at");
        apiProduct.press = row.at("press") != "NULL" ? makeUnique<int>(std::stoi(row.at("press"))) : nullptr;
        apiProduct.mode = row.at("mode") != "NULL" ? makeUnique<std::string>(row.at("mode")) : nullptr;
        apiProduct.totalWeight = std::stod(row.at("total_weight"));
        apiProduct.moistureContent = row.at("moisture_content") != "NULL" ? makeUnique<double>(std::stod(row.at("moisture_content"))) : nullptr;
        apiProduct.producedRunningMeters = row.at("specific_weight") != "NULL" ? makeUnique<double>(apiProduct.totalWeight / std::stod(row.at("specific_weight"))) : nullptr;
        apiProduct.materialsConsumed = std::move(materials);

        apiProducts.push_back(std::move(apiProduct));
    }

    return apiProducts;
}

int main() {
    try {
        const std::string dbPath = "../Base/primex_monitor.db";
        const std::string sqlQueryProducts = "SELECT * FROM products LIMIT 10;";
        const std::string sqlQueryComponents = "SELECT * FROM settings;";

        // Open the database and initialize SQLiteDB instance
        SQLiteDB db(dbPath);

        // Create tables if they don't exist
        db.createTableIfNotExists();

        // Query and print results for products table
        Table products = db.queryDatabase(sqlQueryProducts);
        std::cout << "Products:\n";
        printTable(products);

        // Query and print results for settings table
        Table componentNames = db.queryDatabase(sqlQueryComponents);
        std::cout << "Settings:\n";
        printTable(componentNames);

        std::vector<Product> apiProducts = transformDataToApiFormat(products, componentNames);

        for (const Product &product : apiProducts) {
            std::cout << product.name << std::endl;
        }
    } catch (const std::exception &e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
