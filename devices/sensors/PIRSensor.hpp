#pragma once

#include "../mqtt/MQTT.hpp"
#include <string>
#include "../core/DataModel.hpp"
#include "../Sensor.hpp"
#include "../../environment/Environment.hpp"
#include "../../core/Topics.hpp"
#include "../../core/Logger.hpp"
#include <thread>
#include <iostream>

class PIRSensor : public Sensor {
public:
    PIRSensor(const std::string& deviceId, Room location,
              const std::string& broker, const std::string& topic, 
              Environment& env, int port = 1883) :
        Sensor(deviceId, DeviceType::PIR_SENSOR, location, broker, topic, env, port) {}

    void sample() override {
        // 1. Get the room-specific topic: e.g., "sensors/kitchen/pir"
        std::string sensorTopic = topics::roomPIRTopic(getLocation());
        
        // 2. Read the current state from the environment
        json data = environment.readFromTopic(sensorTopic);

        // 3. Extract motion status with a default of 'false'
        bool motionDetected = data.value("motionDetected", false);
        
        Message msg;
        msg.deviceId = getId();
        msg.deviceType = DeviceType::PIR_SENSOR;
        msg.location = getLocation();
        msg.timestamp = std::time(nullptr);
        msg.payload.payloadType = PayloadType::PIR_PAYLOAD;
        
        msg.payload.data = {
            {"motionDetected", motionDetected}
        };

        if (motionDetected) {
            Logger::getInstance().logSensorData(getId(), DeviceType::PIR_SENSOR, getLocation(),
                "Motion DETECTED in " + to_string_enum(getLocation()), {{"motionDetected", true}});
        }
        
        publish(msg);
    }
};