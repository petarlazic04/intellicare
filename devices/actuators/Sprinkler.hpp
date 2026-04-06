#pragma once
#include "../Actuator.hpp"
#include "../core/DataModel.hpp"
#include "../core/PayloadContracts.hpp"
#include "../core/EnumTraits.hpp"
#include "../../environment/Environment.hpp"
#include "../../core/Topics.hpp"
#include <ctime>
#include <iostream>

class Sprinkler : public Actuator {
public:
    Sprinkler(const std::string& deviceId, Room location, const std::string& broker, const std::string& subscribeTopic, Environment& env, int port = 1883) :
        Actuator(deviceId, DeviceType::SPRINKLER, location, broker, subscribeTopic, env, port) {}

private:
    void act(const Message& msg) override {
        if (msg.payload.payloadType != PayloadType::COMMAND) {
            std::cerr << "[Sprinkler] Invalid payload type\n";
            return;
        }

        try {
            json data = msg.payload.data;
            DeviceActionType action = from_string_enum<DeviceActionType>(data.at("actionType").get<std::string>());
            bool shouldBeActive;

            if (action == DeviceActionType::START || action == DeviceActionType::TURN_ON) {
                shouldBeActive = true;
            } else if (action == DeviceActionType::STOP || action == DeviceActionType::TURN_OFF) {
                shouldBeActive = false;
            } else {
                std::cerr << "[Sprinkler] Unsupported action\n";
                return;
            }

            const std::string topic = topics::actuatorTopic(getLocation(), "sprinkler");
            json sprinklerData = environment.readFromTopic(topic);
            
            sprinklerData["active"] = shouldBeActive;
            sprinklerData["lastActivated"] = std::time(nullptr);
            
            environment.writeToTopic(topic, sprinklerData);

            std::cout << "[Sprinkler] " << to_string_enum(getLocation()) 
                      << " status set to " << (shouldBeActive ? "ACTIVE" : "INACTIVE") << "\n";

        } catch (const std::exception& e) {
            std::cerr << "[Sprinkler] Error: " << e.what() << "\n";
        }
    }
};