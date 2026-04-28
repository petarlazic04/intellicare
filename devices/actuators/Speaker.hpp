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

class Speaker : public Actuator {
  public:
    Speaker(const std::string& deviceId, Room location, const std::string& broker, const std::string& subscribeTopic, Environment& env, Logger& log, int port = 1883, SSDPConfig config = {}):
    Actuator(deviceId, DeviceType::SPEAKER, location, broker, subscribeTopic, env, log, port, config){}
    
    void act(const Message& msg) override {
        if(msg.payload.payloadType != PayloadType::COMMAND){
            logger.logError(getId(), DeviceType::SPEAKER, getLocation(), "Invalid payload type");
            return;
        }

        try {
            json data = msg.payload.data;
            DeviceActionType action = from_string_enum<DeviceActionType>(data.at("actionType").get<std::string>());
            
            int newVolume = -1;

            if (action == DeviceActionType::TURN_ON) {
                newVolume = 50;
            } else if (action == DeviceActionType::TURN_OFF) {
                newVolume = 0;
            } else if (action == DeviceActionType::SET_LEVEL) {
                newVolume = std::stoi(data.at("value").get<std::string>());
                newVolume = std::max(0, std::min(newVolume, (int)MAX_SPEAKER_VOLUME));
            } else {
                logger.logError(getId(), DeviceType::SPEAKER, getLocation(), "Unsupported action");
                return;
            }

            const std::string topic = topics::actuatorTopic(getLocation(), "speaker");
            json speakerData = environment.readFromTopic(topic);
            
            speakerData["volume"] = newVolume;
            speakerData["lastActivated"] = std::time(nullptr);
            
            environment.writeToTopic(topic, speakerData);

            json metadata = {{"volume", newVolume}, {"action", to_string_enum<DeviceActionType>(action)}, {"location", to_string_enum(getLocation())}};
            logger.logActuatorAction(getId(), DeviceType::SPEAKER, getLocation(),
                "Volume set to " + std::to_string(newVolume) + ", action: " + to_string_enum<DeviceActionType>(action), metadata);
        } catch(const std::exception& e) {
            logger.logError(getId(), DeviceType::SPEAKER, getLocation(), 
                std::string("Error: ") + e.what());
        }
    }
};