#pragma once
#include "../Actuator.hpp"
#include "../core/DataModel.hpp"
#include "../core/PayloadContracts.hpp"
#include "../core/EnumTraits.hpp"
#include "../../environment/Environment.hpp"
#include "../../core/Topics.hpp"
#include "../../core/Logger.hpp"
#include <ctime>
#include <iostream>

class Sprinkler : public Actuator {
public:
    Sprinkler(const std::string& deviceId, Room location, const std::string& broker, const std::string& subscribeTopic, Environment& env, Logger& log, int port = 1883) :
        Actuator(deviceId, DeviceType::SPRINKLER, location, broker, subscribeTopic, env, log, port) {}

private:
    void act(const Message& msg) override {
        if (msg.payload.payloadType != PayloadType::COMMAND) {
            logger.logError(getId(), DeviceType::SPRINKLER, getLocation(), "Invalid payload type");
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
                logger.logError(getId(), DeviceType::SPRINKLER, getLocation(), "Unsupported action");
                return;
            }

            const std::string topic = topics::actuatorTopic(getLocation(), "sprinkler");
            json sprinklerData = environment.readFromTopic(topic);
            
            sprinklerData["active"] = shouldBeActive;
            sprinklerData["lastActivated"] = std::time(nullptr);
            
            environment.writeToTopic(topic, sprinklerData);

            json metadata = {{"active", shouldBeActive}, {"location", to_string_enum(getLocation())}};
            logger.logActuatorAction(getId(), DeviceType::SPRINKLER, getLocation(),
                "Status set to " + std::string(shouldBeActive ? "ACTIVE" : "INACTIVE"), metadata);

        } catch (const std::exception& e) {
            std::cerr << "[Sprinkler] Error: " << e.what() << "\n";
        }
    }
};