#include "house/House.hpp"
#include "core/Logger.hpp"
#include <vector>
#include <vector>

int main() {
    // Pass both broker and port here
    House myHouse("localhost", 1883); 
    std::vector<std::string> scenarios = {"scenarios/fire_emergency.json",
                                          "scenarios/health_emergency.json",
                                          "scenarios/fall_stable.json",
                                          "scenarios/fall_critical.json",
                                          "scenarios/global_reset.json"};

    myHouse.autoinstall();

    int scenario;
    Logger::getInstance().logInfo("Main", DeviceType::FIRE_SENSOR, Room::HALLWAY,
        "Available scenarios:");
    Logger::getInstance().logInfo("Main", DeviceType::FIRE_SENSOR, Room::HALLWAY,
        "1. Fire emergency");
    Logger::getInstance().logInfo("Main", DeviceType::FIRE_SENSOR, Room::HALLWAY,
        "2. Health emergency");
    Logger::getInstance().logInfo("Main", DeviceType::FIRE_SENSOR, Room::HALLWAY,
        "3. Fall (vitals stable)");
    Logger::getInstance().logInfo("Main", DeviceType::FIRE_SENSOR, Room::HALLWAY,
        "4. Fall (vitals critical)");
    Logger::getInstance().logInfo("Main", DeviceType::FIRE_SENSOR, Room::HALLWAY,
        "5. Global reset");
    
    std::cout << "Choose a scenario: ";
    std::cin >> scenario;
    
    myHouse.loadScenario(scenarios[scenario-1]);
    myHouse.start(3);
    
    return 0;
}