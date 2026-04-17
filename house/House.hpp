#pragma once

#include <vector>
#include <memory>
#include <fstream>
#include <iostream>
#include <thread>
#include <chrono>
#include "../environment/Environment.hpp"
#include "../core/Topics.hpp"
#include "../core/DataModel.hpp"
#include "../core/Logger.hpp"


#include "../devices/sensors/FireSensor.hpp"
#include "../devices/sensors/HealthSensor.hpp"
#include "../devices/sensors/MotionSensor.hpp"
#include "../devices/sensors/PIRSensor.hpp"
#include "../devices/actuators/Speaker.hpp"
#include "../devices/actuators/Sprinkler.hpp"
#include "../devices/actuators/Light.hpp"
#include "../devices/actuators/Dialer.hpp"
#include "../devices/actuators/Lock.hpp"

class House {
private:
    Environment& env;
    const std::string broker;
    int port;
    
    std::vector<std::unique_ptr<Sensor>> sensors;
    std::vector<std::unique_ptr<Actuator>> actuators;
    
    std::string toUppercase(const std::string& str) const {
        std::string result = str;
        for (char& c : result) {
            c = std::toupper(c);
        }
        return result;
    }

public:
    House(const std::string& mqttBroker, int mqttPort = 1883) 
        : env(Environment::getInstance()), broker(mqttBroker), port(mqttPort) {}

    void autoinstall() {
        Logger::getInstance().logInfo("House", DeviceType::FIRE_SENSOR, Room::HALLWAY, 
            "Starting autoinstall on port " + std::to_string(port));
        
        sensors.push_back(std::make_unique<HealthSensor>("WRISTBAND_HEALTH", broker, topics::wristbandHealthTopic(), env, Logger::getInstance(), port));
        sensors.push_back(std::make_unique<MotionSensor>("WRISTBAND_MOTION", broker, topics::wristbandMotionTopic(), env, Logger::getInstance(), port));
        
        actuators.push_back(std::make_unique<Dialer>("MAIN_DIALER", Room::HALLWAY, broker, topics::dialerTopic(), env, Logger::getInstance(), port));
        actuators.push_back(std::make_unique<Lock>("MAIN_LOCK", Room::HALLWAY, broker, topics::lockTopic(), env, Logger::getInstance(), port));

        std::vector<Room> rooms = { 
            Room::KITCHEN, Room::LIVING_ROOM, Room::BEDROOM, 
            Room::BATHROOM, Room::HALLWAY 
        };
        
        for (Room room : rooms) {
            std::string roomName = toUppercase(to_string_enum(room));

            sensors.push_back(std::make_unique<FireSensor>("FIRE_" + roomName, room, broker, topics::roomFireTopic(room), env, Logger::getInstance(), port));
            sensors.push_back(std::make_unique<PIRSensor>("PIR_" + roomName, room, broker, topics::roomPIRTopic(room), env, Logger::getInstance(), port));

            actuators.push_back(std::make_unique<Sprinkler>("SPRINKLER_" + roomName, room, broker, topics::actuatorTopic(room, "sprinkler"), env, Logger::getInstance(), port));

            actuators.push_back(std::make_unique<Light>("LIGHT_" + roomName, room, broker, topics::actuatorTopic(room, "light"), env, Logger::getInstance(), port));
            actuators.push_back(std::make_unique<Speaker>("SPEAKER_" + roomName, room, broker, topics::actuatorTopic(room, "speaker"), env, Logger::getInstance(), port));
        }

        Logger::getInstance().logInfo("House", DeviceType::FIRE_SENSOR, Room::HALLWAY,
            "Autoinstall complete. " + std::to_string(sensors.size()) + " sensors and " + std::to_string(actuators.size()) + " actuators online.");
    }

    void loadScenario(const std::string& filename) {
        std::ifstream file(filename);
        if (!file.is_open()) {
            Logger::getInstance().logError("House", DeviceType::FIRE_SENSOR, Room::HALLWAY,
                "Failed to open scenario file: " + filename);
            return;
        }
        
        try {
            nlohmann::json scenario = nlohmann::json::parse(file);
            if (scenario.contains("updates")) {
                for (auto& update : scenario["updates"]) {
                    env.writeToTopic(update["topic"], update["data"]);
                }
                Logger::getInstance().logInfo("House", DeviceType::FIRE_SENSOR, Room::HALLWAY,
                    "Scenario '" + scenario.value("name", filename) + "' loaded successfully.");
            }
        } catch (const std::exception& e) {
            Logger::getInstance().logError("House", DeviceType::FIRE_SENSOR, Room::HALLWAY,
                std::string("JSON Error: ") + e.what());
        }
    }

    void start(int intervalSeconds = 5) {
        Logger::getInstance().logInfo("House", DeviceType::FIRE_SENSOR, Room::HALLWAY,
            "Simulation started.");
        while (true) {
            Logger::getInstance().logInfo("House", DeviceType::FIRE_SENSOR, Room::HALLWAY,
                "House Cycle Update");
            for (auto& s : sensors) {
                s->sample(); 
            }
            std::this_thread::sleep_for(std::chrono::seconds(intervalSeconds));
        }
    }
};