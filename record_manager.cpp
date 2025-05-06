#include "record_manager.hpp"
#include "schema.hpp"
#include "index_manager.hpp"

#include <fstream>
#include <iostream>
#include <filesystem>
#include <vector>
#include <cstring>

void RecordManager::addRecord(const std::string& tableName) {
    // Load schema
    std::ifstream meta("Tables/" + tableName + "/meta.txt");
    if (!meta) {
        std::cerr << "Failed to open metadata file for table: " << tableName << "\n";
        return;
    }

    std::string schemaStr, keysStr;
    std::getline(meta, schemaStr);
    std::getline(meta, keysStr);
    Schema schema(schemaStr, keysStr);

    // Load index
    IndexManager indexManager(tableName, "Tables/" + tableName);
    indexManager.loadIndexes(schema.getUniqueKeys());

    // Read data input
    std::vector<std::string> data;
    const auto& fields = schema.getFields();
    const auto& uniqueKeys = schema.getUniqueKeys();

    for (const auto& f : fields) {
        std::cout << "Enter " << f.name << " (" << f.type << "): ";
        std::string val;
        std::cin >> val;

        if (f.type == "int") {
            try {
                std::stoi(val);
            }
            catch (...) {
                std::cout << "Invalid input for int field: " << f.name << "\n";
                return;
            }
        }
        data.push_back(val);
    }

    // Check for duplicate unique keys
    for (size_t i = 0; i < fields.size(); ++i) {
        const auto& field = fields[i];
        if (std::find(uniqueKeys.begin(), uniqueKeys.end(), field.name) != uniqueKeys.end()) {
            if (indexManager.existsInIndex(field.name, data[i])) {
                std::cout << "Duplicate key '" << data[i] << "' for unique field '" << field.name << "'. Record not added.\n";
                return;
            }
        }
    }

    // Write to data file
    std::ofstream file("Tables/" + tableName + "/data.tbl", std::ios::app | std::ios::binary);
    if (!file) {
        std::cerr << "Failed to open data file for writing.\n";
        return;
    }

    long offset = file.tellp();
    for (const std::string& val : data) {
        char buffer[40] = {};
#ifdef _MSC_VER
        strncpy_s(buffer, val.c_str(), 40);
#else
        std::strncpy(buffer, val.c_str(), 40);
#endif
        file.write(buffer, 40);
    }

    // Update indexes
    for (size_t i = 0; i < fields.size(); ++i) {
        const auto& field = fields[i];
        if (std::find(uniqueKeys.begin(), uniqueKeys.end(), field.name) != uniqueKeys.end()) {
            indexManager.insertIntoIndex(field.name, data[i], offset);
        }
    }

    indexManager.saveIndexes();
    std::cout << "Record added successfully.\n";
}
