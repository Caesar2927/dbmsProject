#include "table_manager.hpp"
#include "schema.hpp"
#include "record_manager.hpp"
#include "index_manager.hpp"

#include <iostream>
#include <filesystem>
#include <fstream>
#include <limits>

namespace fs = std::filesystem;

void TableManager::createTable() {
    std::string tableName;
    std::cout << "Enter table name: ";
    std::cin >> tableName;

    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::string schemaInput;
    std::cout << "Enter schema (e.g., int id, string name, int age):\n> ";
    std::getline(std::cin, schemaInput);

    std::cout << "Enter unique keys (comma separated):\n> ";
    std::string keys;
    std::getline(std::cin, keys);

    std::string tablePath = "Tables/" + tableName;
    if (fs::exists(tablePath)) {
        std::cout << "Table already exists.\n";
        return;
    }

    fs::create_directories(tablePath);
    Schema schema(schemaInput, keys);
    schema.saveToFile(tablePath + "/meta.txt");

    // Create an empty data file
    std::ofstream data(tablePath + "/data.tbl", std::ios::binary);
    data.close();

    // Initialize empty indexes
    IndexManager idx(tableName, tablePath);
    idx.loadIndexes(schema.getUniqueKeys());
    idx.saveIndexes();

    std::cout << "Table '" << tableName << "' created successfully.\n";
}

void TableManager::useTable() {
    std::string tableName;
    std::cout << "Enter table name to use: ";
    std::cin >> tableName;

    std::string tablePath = "Tables/" + tableName;
    if (!fs::exists(tablePath)) {
        std::cout << "Table not found.\n";
        return;
    }

    while (true) {
        std::cout << "\n--- Table: " << tableName << " ---\n"
            << "1. Add Record\n"
            << "2. Find Record\n"
            << "3. Exit\n"
            << "Enter choice: ";
        int choice;
        std::cin >> choice;
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        if (choice == 1) {
            RecordManager::addRecord(tableName);
        }
        else if (choice == 2) {
            RecordManager::findRecord(tableName);
        }
        else if (choice == 3) {
            break;
        }
        else {
            std::cout << "Invalid choice. Please try again.\n";
        }
    }
}

void TableManager::deleteTable() {
    std::string tableName;
    std::cout << "Enter table name to delete: ";
    std::cin >> tableName;

    fs::remove_all("Tables/" + tableName);
    std::cout << "Table '" << tableName << "' deleted.\n";
}
