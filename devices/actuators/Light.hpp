#pragma once
#include "../Actuator.hpp"
#include "../core/DataModel.hpp"
#include "../core/PayloadContracts.hpp"
#include "../core/EnumTraits.hpp"
#include "../../environment/Environment.hpp"
#include "../../core/Topics.hpp"
#include <ctime>
#include <iostream>

class Light : public Actuator {
  public:
    Light(const std::string& deviceId, Room location, const std::string& broker, const std::string& subscribeTopic, Environment& env, int port = 1883):
    Actuator(deviceId, DeviceType::LIGHT, location, broker, subscribeTopic, env, port){}

  private:
    void act(const Message& msg) override {
        if(msg.payload.payloadType != PayloadType::COMMAND){
            std::cerr << "[Light] Invalid payload type\n";
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
                std::cerr << "[Light] Unsupported action\n";
                return;
            }

            const std::string topic = topics::actuatorTopic(getLocation(), "light");
            json lightData = environment.readFromTopic(topic);
            
            lightData["brightness"] = newBrightness;
            lightData["lastChanged"] = std::time(nullptr);
            
            environment.writeToTopic(topic, lightData);

            std::cout << "[Light] " << to_string_enum(getLocation()) << " brightness set to " << newBrightness << "\n";

        } catch(const std::exception& e) {
            std::cerr << "[Light] Error: " << e.what() << "\n";
        }
    }
};