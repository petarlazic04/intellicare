#pragma once

#include "../core/DataModel.hpp"
#include "../core/EnumTraits.hpp"
#include "../core/Logger.hpp"
#include "../third_party/nlohmann/json.hpp"
#include <mutex>
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <chrono>

using json = nlohmann::json;

class Environment {
private:
    json houseData;
    mutable std::mutex dataMutex;

    Environment()  {
        Logger::getInstance().logInfo("Environment", DeviceType::FIRE_SENSOR, Room::HALLWAY,
            "Initializing...");
        
        houseData["timestamp"] = std::chrono::system_clock::now().time_since_epoch().count();
        houseData["rooms"] = json::object();
        
        
        std::vector<Room> rooms = {
            Room::KITCHEN, Room::LIVING_ROOM, Room::BEDROOM, 
            Room::BATHROOM, Room::HALLWAY
        };
        
        for (Room room : rooms) {
            std::string roomKey = toLowerString(to_string_enum(room));
            houseData["rooms"][roomKey] = {
                {"name", roomKey},
                {"sensors", {
                    {"fire", {{"temperature", 25.0f}, {"smokeLevel", 100.0f}, {"coLevel", 0}, {"status", "normal"}}},
                    {"pir", {{"motionDetected", false}, {"lastDetected", 0}}}
                }},
                {"actuators", {
                    {"sprinkler", {{"active", false}, {"lastActivated", 0}}},
                    {"light", {{"brightness", 0}, {"lastChanged", 0}}},
                    {"speaker", {{"volume", 0}, {"lastActivated", 0}}}
                }}
            };
        }
        
        houseData["lock"] = {{"locked", true}, {"lastChanged", 0}};
        houseData["emergency"] = {{"active", false}, {"type", ""}, {"location", ""}, {"timestamp", 0}};
        houseData["wristband"] = {{"heartRate", 72}, {"spo2", 98}, {"systolic", 120}, {"diastolic", 80}, {"location", "living_room"}, {"lastUpdate", 0}};
        
        Logger::getInstance().logInfo("Environment", DeviceType::FIRE_SENSOR, Room::HALLWAY,
            "Initialized");
    }

    std::string toLowerString(std::string s) const {
        std::transform(s.begin(), s.end(), s.begin(), ::tolower);
        return s;
    }

    std::vector<std::string> split(const std::string& s) const {
        std::vector<std::string> parts;
        std::string current;
        for (char c : s) {
            if (c == '/') {
                if (!current.empty()) parts.push_back(current);
                current = "";
            } else {
                current += c;
            }
        }
        if (!current.empty()) parts.push_back(current);
        return parts;
    }

public:
    static Environment& getInstance() {
        static Environment instance;
        return instance;
    }

    json readFromTopic(const std::string& topic) const {
        std::lock_guard<std::mutex> lock(dataMutex);
        auto parts = split(topic);
        if (parts.empty()) return json{};

        if (parts.size() == 2 && parts[0] == "actuators" && parts[1] == "lock") {
            return houseData["lock"];
        }

        if (parts.size() == 2 && parts[0] == "actuators" && parts[1] == "dialer") {
            return houseData["emergency"];
        }

        if (parts.size() >= 2 && parts[0] == "sensors" && parts[1] == "wristband") {
            if (parts.size() == 3) {
                if (houseData["wristband"].contains(parts[2])) {
                    return houseData["wristband"][parts[2]];
                }
            }
            return houseData["wristband"];
        }

        if (parts.size() >= 3 && parts[0] == "sensors") {
            if (houseData["rooms"].contains(parts[1])) {
                return houseData["rooms"][parts[1]]["sensors"][parts[2]];
            }
        }

        if (parts.size() >= 3 && parts[0] == "actuators") {
            if (houseData["rooms"].contains(parts[1])) {
                return houseData["rooms"][parts[1]]["actuators"][parts[2]];
            }
        }

        return json{};
    }

    void writeToTopic(const std::string& topic, const json& data) {
        std::lock_guard<std::mutex> lock(dataMutex);
        auto parts = split(topic);
        if (parts.empty()) return;

        // Global: actuators/lock
        if (parts.size() == 2 && parts[0] == "actuators" && parts[1] == "lock") {
            houseData["lock"] = data;
            return;
        }

        // Global: actuators/dialer -> emergency
        if (parts.size() == 2 && parts[0] == "actuators" && parts[1] == "dialer") {
            houseData["emergency"] = data;
            return;
        }

        // Wristband
        if (parts.size() >= 2 && parts[0] == "sensors" && parts[1] == "wristband") {
            if (parts.size() == 3) {
                for (auto& [key, value] : data.items()) {
                    houseData["wristband"][key] = value;
                }
            } else {
                houseData["wristband"] = data;
            }
            return;
        }

        // Room Sensors
        if (parts.size() >= 3 && parts[0] == "sensors") {
            if (houseData["rooms"].contains(parts[1])) {
                houseData["rooms"][parts[1]]["sensors"][parts[2]] = data;
            }
        }

        // Room Actuators
        else if (parts.size() >= 3 && parts[0] == "actuators") {
            if (houseData["rooms"].contains(parts[1])) {
                houseData["rooms"][parts[1]]["actuators"][parts[2]] = data;
            }
        }
    }


    json getHouseData() const { std::lock_guard<std::mutex> lock(dataMutex); return houseData; }
    

    json getRoomData(Room room) const {
        std::lock_guard<std::mutex> lock(dataMutex);
        std::string roomKey = toLowerString(to_string_enum(room));
        if (houseData["rooms"].contains(roomKey)) return houseData["rooms"][roomKey];
        return json{};
    }

    Environment(const Environment&) = delete;
    Environment& operator=(const Environment&) = delete;
};