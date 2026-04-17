#pragma once

#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <csignal>
#include <mutex>
#include "../third_party/nlohmann/json.hpp"
#include "DataModel.hpp"

using json = nlohmann::json;

enum class LogLevel {
    INFO,
    WARNING,
    ERROR,
    DEBUG,
    SENSOR_DATA,
    ACTUATOR_ACTION,
    HAZARD_DETECTED,
    EMERGENCY
};

struct LogEntry {
    std::time_t timestamp;
    LogLevel logLevel;
    std::string deviceId;
    DeviceType deviceType;
    Room location;
    std::string message;
    json metadata;
};

class Logger {
private:
    static Logger* instance;
    std::vector<LogEntry> logBuffer;
    std::string logFilePath;
    std::mutex logMutex;
    
    Logger(const std::string& filePath = "logs.json") : logFilePath(filePath) {
        ensureLogFile();
    }
    
    void ensureLogFile() {
        std::ifstream inFile(logFilePath);
        if (!inFile.good()) {
            json emptyLog = json::array();
            std::ofstream outFile(logFilePath);
            outFile << emptyLog.dump(2) << std::endl;
            outFile.close();
        }
    }
    
public:
    static Logger& getInstance(const std::string& filePath = "logs.json") {
        if (instance == nullptr) {
            instance = new Logger(filePath);
            std::signal(SIGINT, Logger::handleSignal);
            std::signal(SIGTERM, Logger::handleSignal);
        }
        return *instance;
    }
    
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
    
    void log(LogLevel level, 
             const std::string& deviceId, 
             DeviceType deviceType, 
             Room location,
             const std::string& message,
             const json& metadata = json::object()) {
        
        std::lock_guard<std::mutex> lock(logMutex);
        
        LogEntry entry;
        entry.timestamp = std::time(nullptr);
        entry.logLevel = level;
        entry.deviceId = deviceId;
        entry.deviceType = deviceType;
        entry.location = location;
        entry.message = message;
        entry.metadata = metadata;
        
        logBuffer.push_back(entry);
        
        printToConsole(entry);
    }
    
    void logInfo(const std::string& deviceId, DeviceType type, Room loc, 
                 const std::string& msg) {
        log(LogLevel::INFO, deviceId, type, loc, msg);
    }
    
    void logError(const std::string& deviceId, DeviceType type, Room loc,
                  const std::string& msg) {
        log(LogLevel::ERROR, deviceId, type, loc, msg);
    }
    
    void logSensorData(const std::string& deviceId, DeviceType type, Room loc,
                       const std::string& msg, const json& data) {
        log(LogLevel::SENSOR_DATA, deviceId, type, loc, msg, data);
    }
    
    void logActuatorAction(const std::string& deviceId, DeviceType type, Room loc,
                           const std::string& action, const json& details = json::object()) {
        log(LogLevel::ACTUATOR_ACTION, deviceId, type, loc, action, details);
    }
    
    void logHazard(const std::string& hazardType, const std::string& location,
                   const std::string& details, const json& metadata = json::object()) {
        log(LogLevel::HAZARD_DETECTED, hazardType, DeviceType::FIRE_SENSOR, Room::HALLWAY, 
            details, metadata);
    }
    
    void logEmergency(const std::string& emergencyType, const std::string& description,
                      const json& metadata = json::object()) {
        log(LogLevel::EMERGENCY, emergencyType, DeviceType::FIRE_SENSOR, Room::HALLWAY,
            description, metadata);
    }
    
    void writeLogsToFile() {
        std::lock_guard<std::mutex> lock(logMutex);
        
        json logsArray = json::array();
        for (const auto& entry : logBuffer) {
            json logJson;
            logJson["timestamp"] = formatTimestamp(entry.timestamp);
            logJson["unixTimestamp"] = entry.timestamp;
            logJson["logLevel"] = logLevelToString(entry.logLevel);
            logJson["deviceId"] = entry.deviceId;
            logJson["deviceType"] = deviceTypeToString(entry.deviceType);
            logJson["location"] = roomToString(entry.location);
            logJson["message"] = entry.message;
            logJson["metadata"] = entry.metadata;
            
            logsArray.push_back(logJson);
        }
        
        std::ofstream outFile(logFilePath);
        outFile << logsArray.dump(2) << std::endl;
        outFile.close();
    }
    
    json filterLogs(const std::string& deviceId = "",
                    LogLevel* level = nullptr,
                    const std::string& location = "") {
        std::lock_guard<std::mutex> lock(logMutex);
        
        json filtered = json::array();
        
        for (const auto& entry : logBuffer) {
            bool matches = true;
            
            if (!deviceId.empty() && entry.deviceId != deviceId) {
                matches = false;
            }
            
            if (level != nullptr && entry.logLevel != *level) {
                matches = false;
            }
            
            if (!location.empty() && roomToString(entry.location) != location) {
                matches = false;
            }
            
            if (matches) {
                json logJson;
                logJson["timestamp"] = formatTimestamp(entry.timestamp);
                logJson["logLevel"] = logLevelToString(entry.logLevel);
                logJson["deviceId"] = entry.deviceId;
                logJson["deviceType"] = deviceTypeToString(entry.deviceType);
                logJson["location"] = roomToString(entry.location);
                logJson["message"] = entry.message;
                logJson["metadata"] = entry.metadata;
                
                filtered.push_back(logJson);
            }
        }
        
        return filtered;
    }
    
    void clearLogs() {
        std::lock_guard<std::mutex> lock(logMutex);
        logBuffer.clear();
    }
    
    size_t getLogCount() const {
        return logBuffer.size();
    }
    
    json filterByLevel(LogLevel level) {
        LogLevel levelCopy = level;
        return filterLogs("", &levelCopy, "");
    }
    
    json filterByDevice(const std::string& deviceId) {
        return filterLogs(deviceId, nullptr, "");
    }
    
    json filterByLocation(Room location) {
        return filterLogs("", nullptr, roomToString(location));
    }
    
    bool exportToFile(const std::string& filename) {
        std::lock_guard<std::mutex> lock(logMutex);
        
        try {
            json logsArray = json::array();
            for (const auto& entry : logBuffer) {
                json logJson;
                logJson["timestamp"] = formatTimestamp(entry.timestamp);
                logJson["unixTimestamp"] = entry.timestamp;
                logJson["logLevel"] = logLevelToString(entry.logLevel);
                logJson["deviceId"] = entry.deviceId;
                logJson["deviceType"] = deviceTypeToString(entry.deviceType);
                logJson["location"] = roomToString(entry.location);
                logJson["message"] = entry.message;
                logJson["metadata"] = entry.metadata;
                
                logsArray.push_back(logJson);
            }
            
            std::ofstream outFile(filename);
            if (!outFile.is_open()) return false;
            
            outFile << logsArray.dump(2) << std::endl;
            outFile.close();
            return true;
        } catch (const std::exception& e) {
            std::cerr << "Error exporting logs to file: " << e.what() << std::endl;
            return false;
        }
    }
    
    json getAllLogsAsJSON() {
        std::lock_guard<std::mutex> lock(logMutex);
        
        json logsArray = json::array();
        for (const auto& entry : logBuffer) {
            json logJson;
            logJson["timestamp"] = formatTimestamp(entry.timestamp);
            logJson["unixTimestamp"] = entry.timestamp;
            logJson["logLevel"] = logLevelToString(entry.logLevel);
            logJson["deviceId"] = entry.deviceId;
            logJson["deviceType"] = deviceTypeToString(entry.deviceType);
            logJson["location"] = roomToString(entry.location);
            logJson["message"] = entry.message;
            logJson["metadata"] = entry.metadata;
            
            logsArray.push_back(logJson);
        }
        
        return logsArray;
    }
    
private:
    void printToConsole(const LogEntry& entry) {
        std::cout << "[" << formatTimestamp(entry.timestamp) << "] "
                  << "[" << logLevelToString(entry.logLevel) << "] "
                  << "[" << entry.deviceId << "] "
                  << entry.message << std::endl;
    }
    
    std::string formatTimestamp(std::time_t timestamp) {
        char buffer[100];
        struct tm* timeinfo = std::localtime(&timestamp);
        std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", timeinfo);
        return std::string(buffer);
    }
    
    std::string logLevelToString(LogLevel level) {
        switch (level) {
            case LogLevel::INFO: return "INFO";
            case LogLevel::WARNING: return "WARNING";
            case LogLevel::ERROR: return "ERROR";
            case LogLevel::DEBUG: return "DEBUG";
            case LogLevel::SENSOR_DATA: return "SENSOR_DATA";
            case LogLevel::ACTUATOR_ACTION: return "ACTUATOR_ACTION";
            case LogLevel::HAZARD_DETECTED: return "HAZARD_DETECTED";
            case LogLevel::EMERGENCY: return "EMERGENCY";
            default: return "UNKNOWN";
        }
    }
    
    std::string deviceTypeToString(DeviceType type) {
        switch (type) {
            case DeviceType::WRISTBAND: return "WRISTBAND";
            case DeviceType::FIRE_SENSOR: return "FIRE_SENSOR";
            case DeviceType::PIR_SENSOR: return "PIR_SENSOR";
            case DeviceType::DOOR_LOCK: return "DOOR_LOCK";
            case DeviceType::SPRINKLER: return "SPRINKLER";
            case DeviceType::LIGHT: return "LIGHT";
            case DeviceType::SPEAKER: return "SPEAKER";
            case DeviceType::DIALER: return "DIALER";
            default: return "UNKNOWN";
        }
    }
    
    std::string roomToString(Room room) {
        switch (room) {
            case Room::KITCHEN: return "KITCHEN";
            case Room::LIVING_ROOM: return "LIVING_ROOM";
            case Room::BEDROOM: return "BEDROOM";
            case Room::BATHROOM: return "BATHROOM";
            case Room::HALLWAY: return "HALLWAY";
            default: return "UNKNOWN";
        }
    }    
    static void handleSignal(int signal) {

        if (instance != nullptr) {
            if (instance->exportToFile(instance->logFilePath)) {
                std::cout << " Logs saved to " << instance->logFilePath << std::endl;
                std::cout << " Number of logs: '" << instance->getLogCount() << std::endl;
            } else {
                std::cout << "Error saving logs" << std::endl;
            }
        }
        exit(0);
    }};

Logger* Logger::instance = nullptr;
