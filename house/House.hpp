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

public:
    House(const std::string& mqttBroker, int mqttPort = 1883) 
        : env(Environment::getInstance()), broker(mqttBroker), port(mqttPort) {}

    void autoinstall() {
        std::cout << "[House] Starting autoinstall on port " << port << "...\n";

        
        sensors.push_back(std::make_unique<HealthSensor>("wrist_health", broker, topics::wristbandHealthTopic(), env, port));
        sensors.push_back(std::make_unique<MotionSensor>("wrist_motion", broker, topics::wristbandMotionTopic(), env, port));
        
        actuators.push_back(std::make_unique<Dialer>("main_dialer", Room::HALLWAY, broker, topics::dialerTopic(), env, port));
        actuators.push_back(std::make_unique<Lock>("main_lock", Room::HALLWAY, broker, topics::lockTopic(), env, port));

        std::vector<Room> rooms = { 
            Room::KITCHEN, Room::LIVING_ROOM, Room::BEDROOM, 
            Room::BATHROOM, Room::HALLWAY 
        };
        
        for (Room room : rooms) {
            std::string roomName = to_string_enum(room);

            sensors.push_back(std::make_unique<FireSensor>("fire_" + roomName, room, broker, topics::roomFireTopic(room), env, port));
            sensors.push_back(std::make_unique<PIRSensor>("pir_" + roomName, room, broker, topics::roomPIRTopic(room), env, port));

            actuators.push_back(std::make_unique<Sprinkler>("sprink_" + roomName, room, broker, topics::actuatorTopic(room, "sprinkler"), env, port));

            actuators.push_back(std::make_unique<Light>("light_" + roomName, room, broker, topics::actuatorTopic(room, "light"), env, port));
            actuators.push_back(std::make_unique<Speaker>("spk_" + roomName, room, broker, topics::actuatorTopic(room, "speaker"), env, port));
        }

        std::cout << "[House] Autoinstall complete. " << sensors.size() << " sensors and " << actuators.size() << " actuators online.\n";
    }

    void loadScenario(const std::string& filename) {
        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cerr << "[House] Failed to open scenario file: " << filename << std::endl;
            return;
        }
        
        try {
            nlohmann::json scenario = nlohmann::json::parse(file);
            if (scenario.contains("updates")) {
                for (auto& update : scenario["updates"]) {
                    env.writeToTopic(update["topic"], update["data"]);
                }
                std::cout << "[House] Scenario '" << scenario.value("name", filename) << "' loaded successfully.\n";
            }
        } catch (const std::exception& e) {
            std::cerr << "[House] JSON Error: " << e.what() << std::endl;
        }
    }

    void start(int intervalSeconds = 5) {

        std::cout << "[House] Simulation started.\n";
        while (true) {
            std::cout << "\n--- [House Cycle Update] ---" << std::endl;
            for (auto& s : sensors) {
                s->sample(); 
            }
            std::this_thread::sleep_for(std::chrono::seconds(intervalSeconds));
        }
    }
};