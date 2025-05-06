#pragma once
#include <string>
#include <unordered_map>
#include <map>
#include <vector>

struct IndexEntry {
    std::string key;
    long offset; // Byte offset in the .tbl file
};

class IndexManager {
private:
    std::unordered_map<std::string, std::map<std::string, long>> indexes;
    std::string tableName;
    std::string tablePath;

public:
    IndexManager(const std::string& name, const std::string& path);
    bool existsInIndex(const std::string& fieldName, const std::string& key);
    void loadIndexes(const std::vector<std::string>& uniqueFields);
    void insertIntoIndex(const std::string& fieldName, const std::string& key, long offset);
    void saveIndexes();
};
