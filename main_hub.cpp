#include "hub/Hub.hpp"
#include "core/Logger.hpp"
#include <thread>
#include <chrono>
#include <thread>
#include <chrono>

int main() {
    // Everything happens in the constructor: 
    // Connects, Subscribes, and sets up Message Handlers
    Hub smartHub("localhost", 1883);

    Logger::getInstance().logInfo("Main", DeviceType::FIRE_SENSOR, Room::HALLWAY,
        "Hub is running. Monitoring for emergencies...");

    // Keep the main thread alive while the MQTT callbacks run logic
    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    return 0;
}