#pragma once

#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <algorithm>
#include "core/Logger.hpp"

/**
 * LogAnalyzer - C++ utility for log analysis and filtering
 */
class LogAnalyzer {
private:
    std::vector<json> logs;
    std::string logFilePath;

public:
    LogAnalyzer(const std::string& filePath = "logs.json") : logFilePath(filePath) {
        loadLogs();
    }

    void loadLogs() {
        std::ifstream file(logFilePath);
        if (!file.is_open()) {
            std::cerr << "Error: Cannot open file: " << logFilePath << std::endl;
            return;
        }

        try {
            json allLogs;
            file >> allLogs;
            
            if (allLogs.is_array()) {
                logs = allLogs.get<std::vector<json>>();
            } else {
                std::cerr << "Error: JSON is not an array!" << std::endl;
                return;
            }
            
            std::cout << "Loaded logs: " << logs.size() << " entries" << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Error reading JSON: " << e.what() << std::endl;
        }
    }

    void showAll(int limit = -1) {
        if (logs.empty()) {
            std::cout << "No logs available." << std::endl;
            return;
        }

        int toShow = (limit > 0) ? std::min(limit, (int)logs.size()) : logs.size();
        std::cout << "\nShowing " << toShow << "/" << logs.size() << " logs:\n" << std::endl;

        for (int i = 0; i < toShow; i++) {
            printLog(logs[i], i + 1);
        }
    }

    void filterByLevel(const std::string& level) {
        auto filtered = filterLogsInternal([&](const json& log) {
            return log.value("logLevel", "") == level;
        });

        std::cout << "\nLogs with level '" << level << "': " << filtered.size() << "\n" << std::endl;
        displayFiltered(filtered);
    }

    void filterByDevice(const std::string& deviceId) {
        auto filtered = filterLogsInternal([&](const json& log) {
            return log.value("deviceId", "") == deviceId;
        });

        std::cout << "\nLogs for device '" << deviceId << "': " << filtered.size() << "\n" << std::endl;
        displayFiltered(filtered);
    }

    void filterByLocation(const std::string& location) {
        auto filtered = filterLogsInternal([&](const json& log) {
            return log.value("location", "") == location;
        });

        std::cout << "\nLogs from '" << location << "': " << filtered.size() << "\n" << std::endl;
        displayFiltered(filtered);
    }

    void filterByDeviceType(const std::string& deviceType) {
        auto filtered = filterLogsInternal([&](const json& log) {
            return log.value("deviceType", "") == deviceType;
        });

        std::cout << "\nLogs for device type '" << deviceType << "': " << filtered.size() << "\n" << std::endl;
        displayFiltered(filtered);
    }

    void statistics() {
        if (logs.empty()) {
            std::cout << "No logs available." << std::endl;
            return;
        }

        std::cout << "\nSTATISTICS:\n" << std::endl;
        std::cout << "  Total logs: " << logs.size() << std::endl;

        // By log level
        std::map<std::string, int> levels;
        for (const auto& log : logs) {
            std::string level = log.value("logLevel", "UNKNOWN");
            levels[level]++;
        }

        std::cout << "\n  By log level:" << std::endl;
        for (const auto& [level, count] : levels) {
            std::cout << "    " << level << ": " << count << std::endl;
        }

        // By devices
        std::map<std::string, int> devices;
        for (const auto& log : logs) {
            std::string device = log.value("deviceId", "UNKNOWN");
            devices[device]++;
        }

        std::cout << "\n  By devices (Top 10):" << std::endl;
        std::vector<std::pair<int, std::string>> deviceList;
        for (const auto& [device, count] : devices) {
            deviceList.push_back({count, device});
        }
        std::sort(deviceList.rbegin(), deviceList.rend());

        int shown = 0;
        for (const auto& [count, device] : deviceList) {
            if (shown >= 10) break;
            std::cout << "    " << device << ": " << count << std::endl;
            shown++;
        }

        // By locations
        std::map<std::string, int> locations;
        for (const auto& log : logs) {
            std::string loc = log.value("location", "UNKNOWN");
            locations[loc]++;
        }

        std::cout << "\n  By locations:" << std::endl;
        for (const auto& [loc, count] : locations) {
            std::cout << "    " << loc << ": " << count << std::endl;
        }

        std::cout << "\n" << std::endl;
    }

    void clearLogs() {
        std::cout << "Are you sure you want to delete all logs? (yes/no): ";
        std::string response;
        std::cin >> response;

        if (response == "yes") {
            std::ofstream file(logFilePath);
            file << json::array();
            file.close();
            logs.clear();
            std::cout << "All logs deleted." << std::endl;
        } else {
            std::cout << "Cancelled." << std::endl;
        }
    }

    void exportFiltered(const std::string& outputFile, 
                       const std::string& level = "",
                       const std::string& device = "",
                       const std::string& location = "",
                       const std::string& deviceType = "") {
        
        auto filtered = filterLogsInternal([&](const json& log) {
            bool matches = true;
            
            if (!level.empty() && log.value("logLevel", "") != level) matches = false;
            if (!device.empty() && log.value("deviceId", "") != device) matches = false;
            if (!location.empty() && log.value("location", "") != location) matches = false;
            if (!deviceType.empty() && log.value("deviceType", "") != deviceType) matches = false;
            
            return matches;
        });

        std::ofstream file(outputFile);
        file << json(filtered).dump(2);
        file.close();

        std::cout << "Logs saved to '" << outputFile << "' (" << filtered.size() << " entries)" << std::endl;
    }

private:
    void printLog(const json& log, int index) {
        std::string timestamp = log.value("timestamp", "N/A");
        std::string level = log.value("logLevel", "N/A");
        std::string deviceId = log.value("deviceId", "N/A");
        std::string location = log.value("location", "N/A");
        std::string message = log.value("message", "N/A");
        json metadata = log.value("metadata", json::object());

        std::cout << "[" << index << "] [" << timestamp << "] [" << level << "]" << std::endl;
        std::cout << "     Device: " << deviceId << " @ " << location << std::endl;
        std::cout << "     Message: " << message << std::endl;

        if (!metadata.empty()) {
            std::cout << "     Metadata: ";
            for (auto& [key, value] : metadata.items()) {
                std::cout << key << "=" << value << " ";
            }
            std::cout << std::endl;
        }
        std::cout << std::endl;
    }

    void displayFiltered(const std::vector<json>& filtered) {
        if (filtered.empty()) {
            std::cout << "No logs match this filter." << std::endl;
            return;
        }

        for (size_t i = 0; i < filtered.size(); i++) {
            printLog(filtered[i], i + 1);
        }
    }

    template<typename Predicate>
    std::vector<json> filterLogsInternal(Predicate pred) {
        std::vector<json> result;
        for (const auto& log : logs) {
            if (pred(log)) {
                result.push_back(log);
            }
        }
        return result;
    }
};
