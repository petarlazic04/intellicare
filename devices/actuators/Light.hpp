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

class Light : public Actuator {
  public:
    Light(const std::string& deviceId, Room location, const std::string& broker, const std::string& subscribeTopic, Environment& env, Logger& log, int port = 1883, SSDPConfig config = {}):
    Actuator(deviceId, DeviceType::LIGHT, location, broker, subscribeTopic, env, log, port, config){}

  private:
    void act(const Message& msg) override {
        if(msg.payload.payloadType != PayloadType::COMMAND){
            logger.logError(getId(), DeviceType::LIGHT, getLocation(), "Invalid payload type");
            return;
        }

        try {
            json data = msg.payload.data;
            DeviceActionType action = from_string_enum<DeviceActionType>(data.at("actionType").get<std::string>());
            int newBrightness = -1;

            if (action == DeviceActionType::TURN_ON) {
                newBrightness = 100;
            } else if (action == DeviceActionType::TURN_OFF) {
                newBrightness = 0;
            } else if (action == DeviceActionType::SET_LEVEL) {
                newBrightness = std::stoi(data.at("value").get<std::string>());
                newBrightness = std::max(0, std::min(newBrightness, (int)MAX_LIGHT_BRIGHTNESS));
            } else {
                logger.logError(getId(), DeviceType::LIGHT, getLocation(), "Unsupported action");
                return;
            }

            const std::string topic = topics::actuatorTopic(getLocation(), "light");
            json lightData = environment.readFromTopic(topic);
            
            lightData["brightness"] = newBrightness;
            lightData["lastChanged"] = std::time(nullptr);
            
            environment.writeToTopic(topic, lightData);

            json metadata = {{"brightness", newBrightness}, {"location", to_string_enum(getLocation())}};
            logger.logActuatorAction(getId(), DeviceType::LIGHT, getLocation(),
                "Brightness set to " + std::to_string(newBrightness), metadata);

        } catch(const std::exception& e) {
            std::cerr << "[Light] Error: " << e.what() << "\n";
        }
    }
};