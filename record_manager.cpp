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

   
    for (size_t i = 0; i < fields.size(); ++i) {
        const auto& field = fields[i];
        if (std::find(uniqueKeys.begin(), uniqueKeys.end(), field.name) != uniqueKeys.end()) {
            indexManager.insertIntoIndex(field.name, data[i], offset);
        }
    }

    indexManager.saveIndexes();
    std::cout << "Record added successfully.\n";
}
void RecordManager::findRecord(const std::string& tableName) {
    // 1) load schema & unique keys
    std::ifstream meta("Tables/" + tableName + "/meta.txt");
    std::string schemaStr, keysStr;
    std::getline(meta, schemaStr);
    std::getline(meta, keysStr);
    Schema schema(schemaStr, keysStr);

    // 2) get user query
    std::cout << "Enter query (field=value): ";
    std::string input;
    std::getline(std::cin, input);
    auto eq = input.find('=');
    if (eq == std::string::npos) {
        std::cout << "Invalid format\n"; return;
    }
    std::string field = input.substr(0, eq);
    std::string value = input.substr(eq + 1);
    // trim spaces
    auto trim = [](std::string& s) {
        s.erase(std::remove_if(s.begin(), s.end(), ::isspace), s.end());
        };
    trim(field); trim(value);

    // 3) find field index & unique flag
    const auto& fields = schema.getFields();
    const auto& uniqueKeys = schema.getUniqueKeys();
    int idx = -1;
    bool isUnique = false;
    for (int i = 0; i < (int)fields.size(); ++i) {
        if (fields[i].name == field) {
            idx = i;
            isUnique = std::find(uniqueKeys.begin(), uniqueKeys.end(), field)
                != uniqueKeys.end();
            break;
        }
    }
    if (idx < 0) {
        std::cout << "Field not in schema\n"; return;
    }
    

    // 4) if unique: use B+ tree
    IndexManager im(tableName, "Tables/" + tableName);
    im.loadIndexes(uniqueKeys);
    if (isUnique) {
        long off = im.searchIndex(field, value);
        if (off < 0) {
            std::cout << "Not found\n"; return;
        }
        // read exactly one record at off
        std::ifstream data("Tables/" + tableName + "/data.tbl", std::ios::binary);
        data.seekg(off);
        for (auto& f : fields) {
            char buf[40] = {};
            data.read(buf, 40);
            std::cout << f.name << ": " << buf << "  ";
        }
        std::cout << "\n";
    }
    // 5) else: linear scan
    else {
        std::cout << "Scanning all records...\n";
        std::ifstream data("Tables/" + tableName + "/data.tbl", std::ios::binary);
        while (data.peek() != EOF) {
            long recPos = data.tellg();
            std::vector<std::string> row(fields.size());
            for (int i = 0; i < (int)fields.size(); ++i) {
                char buf[40] = {};
                data.read(buf, 40);
                row[i] = buf;
            }
            if (row[idx] == value) {
                for (int i = 0; i < (int)fields.size(); ++i)
                    std::cout << fields[i].name << ": " << row[i] << "  ";
                std::cout << "\n";
            }
        }
    }
}
