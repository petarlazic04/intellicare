#pragma once

#include "../mqtt/MQTT.hpp"
#include <string>
#include <cmath>
#include "../core/DataModel.hpp"
#include "../Sensor.hpp"
#include "../../environment/Environment.hpp"
#include "../../core/Topics.hpp"
#include "../../core/Logger.hpp"
#include <thread>
#include <iostream>

class MotionSensor : public Sensor {
public:
    MotionSensor(const std::string& deviceId, 
                 const std::string& broker, const std::string& topic, 
                 Environment& env, Logger& log, int port = 1883, SSDPConfig config = {}) :
        Sensor(deviceId, DeviceType::WRISTBAND, Room::LIVING_ROOM, broker, topic, env, log, port, config) {}

    void sample() override {
        std::string sensorTopic = topics::wristbandMotionTopic();
        json data = environment.readFromTopic(sensorTopic);

        double accelX = data.value("accelX", 0.0);
        double accelY = data.value("accelY", 0.0);
        double accelZ = data.value("accelZ", 1.0); 
        
        double gyroX  = data.value("gyroX", 0.0);
        double gyroY  = data.value("gyroY", 0.0);
        double gyroZ  = data.value("gyroZ", 0.0);

        double magnitude = std::sqrt(accelX * accelX + accelY * accelY + accelZ * accelZ);

        Message msg;
        msg.deviceId = getId();
        msg.deviceType = DeviceType::WRISTBAND;
        
        std::string locStr = data.value("location", "LIVING_ROOM");
        msg.location = from_string_enum<Room>(locStr);
        
        msg.timestamp = std::time(nullptr);
        msg.payload.payloadType = PayloadType::MOTION_PAYLOAD;
        
        msg.payload.data = {
            {"accelX", accelX},
            {"accelY", accelY},
            {"accelZ", accelZ},
            {"gyroX", gyroX},
            {"gyroY", gyroY},
            {"gyroZ", gyroZ},
            {"magnitude", magnitude}
        };

        json metadata = {{"accelX", accelX}, {"accelY", accelY}, {"accelZ", accelZ}, 
                         {"gyroX", gyroX}, {"gyroY", gyroY}, {"gyroZ", gyroZ}, {"magnitude", magnitude}};
        logger.logSensorData(getId(), DeviceType::WRISTBAND, msg.location,
            "Independent Wearable MotionSensor sampled", metadata);
        
        publish(msg);
    }
};