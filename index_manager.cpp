#include "index_manager.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>

namespace fs = std::filesystem;

IndexManager::IndexManager(const std::string& tableName_,
    const std::string& tablePath_)
    : tableName(tableName_), tablePath(tablePath_) {
}

IndexManager::~IndexManager() {
    for (auto& [_, tree] : trees) {
        delete tree;
    }
    trees.clear();
}

void IndexManager::loadIndexes(const std::vector<std::string>& uniqueFields) {
    for (const auto& field : uniqueFields) {
        std::string idxFile = tablePath + "/" + field + ".idx";
        // ensure directory
        if (!fs::exists(tablePath)) {
            std::cerr << "Index load error: missing table path " << tablePath << "\n";
            continue;
        }
        // create file if absent
        if (!fs::exists(idxFile)) {
            std::ofstream(idxFile, std::ios::binary).close();
        }
        trees[field] = new BPlusTree(idxFile);
    }
}

void IndexManager::insertIntoIndex(const std::string& fieldName,
    const std::string& key,
    long offset) {
    auto it = trees.find(fieldName);
    if (it == trees.end()) {
        std::cerr << "Insert error: no index for field " << fieldName << "\n";
        return;
    }
    it->second->insert(key, offset);
}

bool IndexManager::existsInIndex(const std::string& fieldName,
    const std::string& key) {
    auto it = trees.find(fieldName);
    if (it == trees.end()) return false;
    long dummy;
    return it->second->search(key, dummy);
}

void IndexManager::saveIndexes() {
    // all persistence is handled by BPlusTree::insert()
}

long IndexManager::getOffset(const std::string& fieldName, const std::string& key)
{
    return 0;
}

long IndexManager::searchIndex(const std::string& fieldName,
    const std::string& key) {
    auto it = trees.find(fieldName);

    if (it == trees.end()) {
    //    std::cout << "problem finding the filed name in tree" << std::endl;
        return -1;
    }
    //std::cout << "no probelm in this section " << std::endl;
    long offset;
    // this will invoke BPlusTree::search, which loads one 4KB page at a time
    if (it->second->search(key, offset)) {
      //  std::cout << "return ing offset " << std::endl;
        return offset;
    }
    //std::cout << "returnin -1 " << std::endl;
    return -1;
}
