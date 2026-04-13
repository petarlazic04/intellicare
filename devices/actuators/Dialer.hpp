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

class Dialer : public Actuator {
  public:
    Dialer(const std::string& deviceId, Room location, const std::string& broker, const std::string& subscribeTopic, Environment& env, int port = 1883):
    Actuator(deviceId, DeviceType::DIALER, location, broker, subscribeTopic, env, port){}

  private:
    void act(const Message& msg) override {
        if(msg.payload.payloadType != PayloadType::COMMAND){
            Logger::getInstance().logError(getId(), DeviceType::DIALER, getLocation(), "Invalid payload type");
            return;
        }

        try {
            json data = msg.payload.data;
            DeviceActionType action = from_string_enum<DeviceActionType>(data.at("actionType").get<std::string>());
            std::string emergencyType;

            if (action == DeviceActionType::DIAL_AMBULANCE) {
                emergencyType = "AMBULANCE";
            } else if (action == DeviceActionType::DIAL_FIRE_BRIGADE) {
                emergencyType = "FIRE_BRIGADE";
            } else if (action == DeviceActionType::NOTIFY_FAMILY) {
                emergencyType = "FAMILY_NOTIFICATION";
            } else {
                Logger::getInstance().logError(getId(), DeviceType::DIALER, getLocation(), "Unsupported action");
                return;
            }

            // Using the "emergency" key directly as per your Environment structure
            json emergencyData = environment.readFromTopic("emergency");
            
            emergencyData["active"] = true;
            emergencyData["type"] = emergencyType;
            emergencyData["location"] = to_string_enum(getLocation());
            emergencyData["timestamp"] = std::time(nullptr);
            
            environment.writeToTopic("emergency", emergencyData);

            json metadata = {{"emergencyType", emergencyType}, {"location", to_string_enum(getLocation())}, {"action", to_string_enum<DeviceActionType>(action)}};
            Logger::getInstance().logEmergency(emergencyType, "Emergency triggered: " + emergencyType, metadata);

        } catch(const std::exception& e) {
            Logger::getInstance().logError(getId(), DeviceType::DIALER, getLocation(),
                std::string("Error: ") + e.what());
        }
    }
};