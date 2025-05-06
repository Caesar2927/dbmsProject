#include "table_manager.hpp"
#include "schema.hpp"
#include "record_manager.hpp"
#include "index_manager.hpp"

#include <iostream>
#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

void TableManager::createTable() {
    std::string tableName;
    std::cout << "Enter table name: ";
    std::cin >> tableName;

    std::string schemaInput;
    std::cin.ignore();
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

    std::ofstream data(tablePath + "/data.tbl", std::ios::binary); // create file

    IndexManager indexManager(tableName, tablePath);
    indexManager.loadIndexes(schema.getUniqueKeys());  // initialize empty trees
    indexManager.saveIndexes();  // save empty index files

    std::cout << "Table created successfully.\n";
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
        std::cout << "\nUsing table: " << tableName << "\n";
        std::cout << "1. Add Record\n2. Exit\nEnter choice: ";
        int choice;
        std::cin >> choice;
        if (choice == 1) {
            RecordManager::addRecord(tableName);
        }
        else {
            break;
        }
    }
}

void TableManager::deleteTable() {
    std::string tableName;
    std::cout << "Enter table name to delete: ";
    std::cin >> tableName;
    fs::remove_all("Tables/" + tableName);
    std::cout << "Table deleted.\n";
}
