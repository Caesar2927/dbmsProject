#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include "bplustree.hpp"

class IndexManager {
public:
    IndexManager(const std::string& tableName, const std::string& tablePath);
    ~IndexManager();

    void loadIndexes(const std::vector<std::string>& uniqueFields);
    void insertIntoIndex(const std::string& fieldName, const std::string& key, long offset);
    bool existsInIndex(const std::string& fieldName, const std::string& key);
    void saveIndexes();  // no-op, BPlusTree persists on insert
    long getOffset(const std::string& fieldName, const std::string& key);
    long searchIndex(const std::string& fieldName,
        const std::string& key);



private:
    std::string tableName;
    std::string tablePath;
    std::unordered_map<std::string, BPlusTree*> trees;
};
