

#include <iostream>
#include "table_manager.hpp"

int main() {
    while (true) {
        std::cout << "\n--- Simple DBMS CLI ---\n";
        std::cout << "1. Create Table\n";
        std::cout << "2. Use Table\n";
        std::cout << "3. Delete Table\n";
        std::cout << "4. Exit\n";
        std::cout << "Enter choice: ";

        int choice;
        std::cin >> choice;

        switch (choice) {
        case 1:
            TableManager::createTable();
            break;
        case 2:
            TableManager::useTable();
            break;
        case 3:
            TableManager::deleteTable();
            break;
        case 4:
            return 0;
        default:
            std::cout << "Invalid choice!\n";
        }
    }
}



