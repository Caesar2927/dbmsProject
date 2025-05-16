#pragma once
#include <string>

class TableManager {
public:
    /// Create a new table (schema + empty data + empty indexes)
    static void createTable();

    /// Main per-table loop: Add Record, Find Record, or Exit
    static void useTable();

    /// Delete an existing table (folder + files)
    static void deleteTable();
};
