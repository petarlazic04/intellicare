#pragma once

#include "../mqtt/MQTT.hpp"
#include <string>
#include "../core/DataModel.hpp"
#include "../Sensor.hpp"
#include "../../environment/Environment.hpp"
#include "../../core/Topics.hpp"
#include <thread>
#include <iostream>

class HealthSensor : public Sensor {
public:
    HealthSensor(const std::string& deviceId, const std::string& broker, const std::string& topic, 
                 Environment& env, int port = 1883) :
        Sensor(deviceId, DeviceType::WRISTBAND, Room::LIVING_ROOM , broker, topic, env, port) {}

    void sample() override {
        std::string sensorTopic = topics::wristbandHealthTopic();
        json data = environment.readFromTopic(sensorTopic);

        int heartrate = data.value("heartRate", 72);
        int spo2      = data.value("spo2", 98);
        int systolic  = data.value("systolic", 120);
        int diastolic = data.value("diastolic", 80);

        Message msg;
        msg.deviceId = getId();
        msg.deviceType = DeviceType::WRISTBAND;
        msg.location = getLocation();
        msg.timestamp = std::time(nullptr);
        msg.payload.payloadType = PayloadType::HEALTH_PAYLOAD;
        
        msg.payload.data = {
            {"heartRate", heartrate},
            {"spo2", spo2},
            {"systolic", systolic},
            {"diastolic", diastolic}
        };

        std::cout << "[HealthSensor] Sampled -> HR: " << heartrate 
                  << ", SpO2: " << spo2 << "%, BP: " << systolic << "/" << diastolic << "\n";
        
        publish(msg);
    }
};