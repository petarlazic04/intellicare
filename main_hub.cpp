#include "hub/Hub.hpp"
#include "core/Logger.hpp"
#include <thread>
#include <chrono>
#include <thread>
#include <chrono>

int main() {

    SSDPConfig config;
    config.multicastGroup = "239.255.255.250"; // Standard SSDP Group
    config.port           = 1900;              // Standard SSDP Port
    config.interval       = 30;                // Send "alive" every 30s
    config.ttl            = 2;                 // Hops (limit to local network)

    Hub smartHub("localhost", 1883, config);

    Logger::getInstance().logInfo("Main", DeviceType::FIRE_SENSOR, Room::HALLWAY,
        "Hub is running. Monitoring for emergencies...");

    // Keep the main thread alive while the MQTT callbacks run logic
    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    return 0;
}