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

class FireSensor : public Sensor {
public:
    FireSensor(const std::string& deviceId, Room location,
               const std::string& broker, const std::string& topic, 
               Environment& env, int port = 1883) :
        Sensor(deviceId, DeviceType::FIRE_SENSOR, location, broker, topic, env, port) {}

    void sample() override {
        std::string sensorTopic = topics::roomFireTopic(getLocation());
        json data = environment.readFromTopic(sensorTopic);

        float temperature = data.value("temperature", 25.0f);
        float smokeLevel  = data.value("smokeLevel", 0.0f);
        int coLevel       = data.value("coLevel", 0);

        Message msg;
        msg.deviceId = getId();
        msg.deviceType = DeviceType::FIRE_SENSOR;
        msg.location = getLocation();
        msg.timestamp = std::time(nullptr);
        msg.payload.payloadType = PayloadType::FIRE_DETECTOR_PAYLOAD;
        
        msg.payload.data = {
            {"temperature", temperature},
            {"smokeLevel", smokeLevel},
            {"coLevel", coLevel}
        };

        json metadata = {{"temperature", temperature}, {"smokeLevel", smokeLevel}, {"coLevel", coLevel}};
        Logger::getInstance().logSensorData(getId(), DeviceType::FIRE_SENSOR, getLocation(),
            "FireSensor sampled", metadata);
        
        publish(msg);
    }
};