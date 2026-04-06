#pragma once
#include "../Actuator.hpp"
#include "../core/DataModel.hpp"
#include "../core/PayloadContracts.hpp"
#include "../core/EnumTraits.hpp"
#include "../../environment/Environment.hpp"
#include "../../core/Topics.hpp"
#include <ctime>
#include <iostream>

class Lock : public Actuator {
  public:
    Lock(const std::string& deviceId, Room location, const std::string& broker, const std::string& subscribeTopic, Environment& env, int port = 1883):
    Actuator(deviceId, DeviceType::DOOR_LOCK, location, broker, subscribeTopic, env, port){}
    
    void act(const Message& msg) override {
        if(msg.payload.payloadType != PayloadType::COMMAND){
            std::cerr << "[Lock] Invalid payload type\n";
            return;
        }

        json data = msg.payload.data;

        if(!data.contains("actionType")){
            std::cerr << "[Lock] Missing actionType field\n";
            return;
        }

        DeviceActionType action = from_string_enum<DeviceActionType>(data.at("actionType").get<std::string>());
        bool shouldBeLocked;

        if (action == DeviceActionType::LOCK) {
            shouldBeLocked = true;
        } else if (action == DeviceActionType::UNLOCK) {
            shouldBeLocked = false;
        } else {
            std::cerr << "[Lock] Action not supported\n";
            return;
        }

        const std::string topic = topics::lockTopic();
        json lockState = environment.readFromTopic(topic);
        
        lockState["locked"] = shouldBeLocked;
        lockState["lastChanged"] = std::time(nullptr);
        
        environment.writeToTopic(topic, lockState);

        std::cout << "[Lock] Status: " << (shouldBeLocked ? "LOCKED" : "UNLOCKED") << "\n";
    }
};