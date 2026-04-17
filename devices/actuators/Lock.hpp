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

class Lock : public Actuator {
  public:
    Lock(const std::string& deviceId, Room location, const std::string& broker, const std::string& subscribeTopic, Environment& env, Logger& log, int port = 1883):
    Actuator(deviceId, DeviceType::DOOR_LOCK, location, broker, subscribeTopic, env, log, port){}
    
    void act(const Message& msg) override {
        if(msg.payload.payloadType != PayloadType::COMMAND){
            logger.logError(getId(), DeviceType::DOOR_LOCK, getLocation(), "Invalid payload type");
            return;
        }

        json data = msg.payload.data;

        if(!data.contains("actionType")){
            logger.logError(getId(), DeviceType::DOOR_LOCK, getLocation(), "Missing actionType field");
            return;
        }

        DeviceActionType action = from_string_enum<DeviceActionType>(data.at("actionType").get<std::string>());
        bool shouldBeLocked;

        if (action == DeviceActionType::LOCK) {
            shouldBeLocked = true;
        } else if (action == DeviceActionType::UNLOCK) {
            shouldBeLocked = false;
        } else {
            logger.logError(getId(), DeviceType::DOOR_LOCK, getLocation(), "Action not supported");
            return;
        }

        const std::string topic = topics::lockTopic();
        json lockState = environment.readFromTopic(topic);
        
        lockState["locked"] = shouldBeLocked;
        lockState["lastChanged"] = std::time(nullptr);
        
        environment.writeToTopic(topic, lockState);

        json metadata = {{"locked", shouldBeLocked}, {"action", to_string_enum<DeviceActionType>(action)}};
        logger.logActuatorAction(getId(), DeviceType::DOOR_LOCK, getLocation(),
            "Status: " + std::string(shouldBeLocked ? "LOCKED" : "UNLOCKED"), metadata);
    }
};