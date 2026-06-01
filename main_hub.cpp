#include "hub/Hub.hpp"
#include "core/Logger.hpp"
#include <thread>
#include <chrono>
#include <thread>
#include <chrono>

int main() {

    SSDPConfig config;
    config.multicastGroup = "239.255.255.250"; 
    config.port           = 1900;             
    config.interval       = 30;                
    config.ttl            = 2;                 

    config.allowedDeviceIds = {
        "WRISTBAND_HEALTH","WRISTBAND_MOTION","MAIN_DIALER","MAIN_LOCK",
        "FIRE_KITCHEN","PIR_KITCHEN","SPRINKLER_KITCHEN","LIGHT_KITCHEN","SPEAKER_KITCHEN",
        "FIRE_LIVING_ROOM","PIR_LIVING_ROOM","SPRINKLER_LIVING_ROOM","LIGHT_LIVING_ROOM","SPEAKER_LIVING_ROOM",
        "FIRE_BEDROOM","PIR_BEDROOM","SPRINKLER_BEDROOM","LIGHT_BEDROOM","SPEAKER_BEDROOM",
        "FIRE_BATHROOM","PIR_BATHROOM","SPRINKLER_BATHROOM","LIGHT_BATHROOM","SPEAKER_BATHROOM",
        "FIRE_HALLWAY","PIR_HALLWAY","SPRINKLER_HALLWAY","LIGHT_HALLWAY","SPEAKER_HALLWAY"
    };

    Hub smartHub("localhost", 1883, config);

    Logger::getInstance().logInfo("Main", DeviceType::FIRE_SENSOR, Room::HALLWAY,
        "Hub is running. Monitoring for emergencies...");

    // Keep the main thread alive while the MQTT callbacks run logic
    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    return 0;
}