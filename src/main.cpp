#include "data_processor.h"
#include "sqlite_db.h"
#include <iostream>

void printTable(const Table &table) {
    for (const auto &row : table) {
        for (const auto &cell : row) {
            std::cout << cell.first << ": " << cell.second << std::endl;
        }
        std::cout << std::endl;
    }
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

        std::vector<Product> apiProducts = DataProcessor::transformDataToApiFormat(products, componentNames);

        for (const Product &product : apiProducts) {
            std::cout << product.name << std::endl;
        }
    } catch (const std::exception &e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
