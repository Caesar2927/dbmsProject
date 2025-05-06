#include "index_manager.hpp"
#include <fstream>
#include <filesystem>

namespace fs = std::filesystem;

IndexManager::IndexManager(const std::string& name, const std::string& path)
    : tableName(name), tablePath(path) {
}
bool IndexManager::existsInIndex(const std::string& fieldName, const std::string& key) {
    if (indexes.find(fieldName) == indexes.end()) return false;
    return indexes[fieldName].find(key) != indexes[fieldName].end();
}


void IndexManager::loadIndexes(const std::vector<std::string>& uniqueFields) {
    for (const auto& field : uniqueFields) {
        std::string idxPath = tablePath + "/" + field + ".idx";
        indexes[field] = std::map<std::string, long>();

        std::ifstream idxFile(idxPath);
        if (idxFile.is_open()) {
            std::string key;
            long offset;
            while (idxFile >> key >> offset) {
                indexes[field][key] = offset;
            }
            idxFile.close();
        }
    }
}

void IndexManager::insertIntoIndex(const std::string& fieldName, const std::string& key, long offset) {
    indexes[fieldName][key] = offset;
}

void IndexManager::saveIndexes() {
    for (const auto& [field, btree] : indexes) {
        std::ofstream idxFile(tablePath + "/" + field + ".idx", std::ios::trunc);
        for (const auto& [key, offset] : btree) {
            idxFile << key << " " << offset << "\n";
        }
    }
}
