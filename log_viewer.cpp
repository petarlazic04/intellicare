#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include "core/LogAnalyzer.hpp"

void printMenu() {
    std::cout << "\n========================================" << std::endl;
    std::cout << "      IntelliCare Log Viewer" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "\n1. Show all logs" << std::endl;
    std::cout << "2. Filter by log level" << std::endl;
    std::cout << "3. Filter by device" << std::endl;
    std::cout << "4. Filter by location" << std::endl;
    std::cout << "5. Filter by device type" << std::endl;
    std::cout << "6. Show statistics" << std::endl;
    std::cout << "7. Search and export logs" << std::endl;
    std::cout << "8. Clear all logs" << std::endl;
    std::cout << "9. Exit" << std::endl;
    std::cout << "\nSelect (1-9): ";
}

int main() {
    LogAnalyzer analyzer("logs.json");
    int choice = 0;

    while (choice != 9) {
        printMenu();
        std::cin >> choice;
        std::cin.ignore();

        switch (choice) {
            case 1: {
                int limit = -1;
                std::cout << "How many logs to show? (-1 for all): ";
                std::cin >> limit;
                analyzer.showAll(limit);
                break;
            }
            case 2: {
                std::cout << "Enter log level (INFO, WARNING, ERROR, DEBUG, SENSOR_DATA, ACTUATOR_ACTION, HAZARD_DETECTED, EMERGENCY): ";
                std::string level;
                std::getline(std::cin, level);
                analyzer.filterByLevel(level);
                break;
            }
            case 3: {
                std::cout << "Enter device ID: ";
                std::string device;
                std::getline(std::cin, device);
                analyzer.filterByDevice(device);
                break;
            }
            case 4: {
                std::cout << "Enter location: ";
                std::string location;
                std::getline(std::cin, location);
                analyzer.filterByLocation(location);
                break;
            }
            case 5: {
                std::cout << "Enter device type: ";
                std::string deviceType;
                std::getline(std::cin, deviceType);
                analyzer.filterByDeviceType(deviceType);
                break;
            }
            case 6: {
                analyzer.statistics();
                break;
            }
            case 7: {
                std::cout << "\nExport with filters:" << std::endl;
                std::cout << "Output filename (e.g. export.json): ";
                std::string filename;
                std::getline(std::cin, filename);

                std::cout << "Log level (leave empty for all): ";
                std::string level;
                std::getline(std::cin, level);

                std::cout << "Device ID (leave empty for all): ";
                std::string device;
                std::getline(std::cin, device);

                std::cout << "Location (leave empty for all): ";
                std::string location;
                std::getline(std::cin, location);

                std::cout << "Device type (leave empty for all): ";
                std::string deviceType;
                std::getline(std::cin, deviceType);

                analyzer.exportFiltered(filename, level, device, location, deviceType);
                break;
            }
            case 8: {
                analyzer.clearLogs();
                break;
            }
            case 9: {
                std::cout << "Goodbye!" << std::endl;
                break;
            }
            default: {
                std::cout << "Invalid choice." << std::endl;
            }
        }
    }

    return 0;
}
