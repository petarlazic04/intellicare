#include "house/House.hpp"
#include "core/Logger.hpp"
#include <vector>
#include <vector>



int main() {

    SSDPConfig config;
    config.multicastGroup = "239.255.255.250"; // Standard SSDP Group
    config.port           = 1900;              // Standard SSDP Port
    config.interval       = 30;                // Send "alive" every 30s
    config.ttl            = 2;                 // Hops (limit to local network)
    House myHouse("localhost", 1883, config); 
    
    std::vector<std::string> scenarios = {
        // Original scenarios
        "scenarios/fire_emergency.json",                          // 1
        "scenarios/health_emergency.json",                        // 2
        "scenarios/fall_stable.json",                             // 3
        "scenarios/fall_critical.json",                           // 4
        "scenarios/global_reset.json",                            // 5
        
        // New baseline & normal operation
        "scenarios/normal_operation.json",                        // 6
        
        // Fire detection scenarios
        "scenarios/extreme_fire_temperature.json",                // 7
        "scenarios/smoke_only.json",                              // 8
        "scenarios/high_co_level.json",                           // 9
        "scenarios/multi_room_fire.json",                         // 10
        "scenarios/extreme_temperature_room.json",                // 11
        
        // Individual vital parameters - Critical LOW
        "scenarios/minimum_vitals.json",                          // 12
        "scenarios/slow_heart_rate.json",                         // 13
        "scenarios/low_blood_pressure.json",                      // 14
        "scenarios/low_oxygen_saturation.json",                   // 15
        
        // Individual vital parameters - Critical HIGH
        "scenarios/maximum_vitals.json",                          // 16
        "scenarios/rapid_heart_rate.json",                        // 17
        "scenarios/high_blood_pressure.json",                     // 18
        
        // Boundary conditions
        "scenarios/oxygen_saturation_boundaries.json",            // 19
        "scenarios/edge_case_normal_boundaries.json",             // 20
        "scenarios/asymmetric_blood_pressure.json",               // 21
        
        // Fall scenarios
        "scenarios/fall_with_panic.json",                         // 22
        "scenarios/fall_immobile_critical_vitals.json",           // 23
        "scenarios/no_motion_high_vitals.json",                   // 24
        
        // Complex combined scenarios
        "scenarios/combined_fire_health_crisis.json",             // 25
        "scenarios/rapid_condition_change.json",                  // 26
        "scenarios/recovery_sequence.json",                       // 27
        
        // Sensor reliability & edge cases
        "scenarios/false_motion_detection.json",                  // 28
        "scenarios/zero_values_sensor_failure.json",              // 29
        "scenarios/stress_test_load.json"                         // 30
    };

    myHouse.autoinstall();

    int scenario;
    Logger::getInstance().logInfo("Main", DeviceType::FIRE_SENSOR, Room::HALLWAY,
        "========== INTELLICARE TEST SCENARIOS ==========");
    Logger::getInstance().logInfo("Main", DeviceType::FIRE_SENSOR, Room::HALLWAY,
        "ORIGINAL SCENARIOS (1-5):");
    Logger::getInstance().logInfo("Main", DeviceType::FIRE_SENSOR, Room::HALLWAY,
        "  1. Fire emergency");
    Logger::getInstance().logInfo("Main", DeviceType::FIRE_SENSOR, Room::HALLWAY,
        "  2. Health emergency (cardiac)");
    Logger::getInstance().logInfo("Main", DeviceType::FIRE_SENSOR, Room::HALLWAY,
        "  3. Fall (vitals stable)");
    Logger::getInstance().logInfo("Main", DeviceType::FIRE_SENSOR, Room::HALLWAY,
        "  4. Fall (vitals critical)");
    Logger::getInstance().logInfo("Main", DeviceType::FIRE_SENSOR, Room::HALLWAY,
        "  5. Global reset");
    
    Logger::getInstance().logInfo("Main", DeviceType::FIRE_SENSOR, Room::HALLWAY,
        "BASELINE (6):");
    Logger::getInstance().logInfo("Main", DeviceType::FIRE_SENSOR, Room::HALLWAY,
        "  6. Normal operation");
    
    Logger::getInstance().logInfo("Main", DeviceType::FIRE_SENSOR, Room::HALLWAY,
        "FIRE DETECTION (7-11):");
    Logger::getInstance().logInfo("Main", DeviceType::FIRE_SENSOR, Room::HALLWAY,
        "  7. Extreme fire temperature");
    Logger::getInstance().logInfo("Main", DeviceType::FIRE_SENSOR, Room::HALLWAY,
        "  8. Smoke only (slow burn)");
    Logger::getInstance().logInfo("Main", DeviceType::FIRE_SENSOR, Room::HALLWAY,
        "  9. High CO level");
    Logger::getInstance().logInfo("Main", DeviceType::FIRE_SENSOR, Room::HALLWAY,
        "  10. Multi-room fire spread");
    Logger::getInstance().logInfo("Main", DeviceType::FIRE_SENSOR, Room::HALLWAY,
        "  11. Extreme room temperature (non-fire)");
    
    Logger::getInstance().logInfo("Main", DeviceType::FIRE_SENSOR, Room::HALLWAY,
        "VITAL SIGNS - CRITICAL LOW (12-15):");
    Logger::getInstance().logInfo("Main", DeviceType::FIRE_SENSOR, Room::HALLWAY,
        "  12. Minimum vitals bundle");
    Logger::getInstance().logInfo("Main", DeviceType::FIRE_SENSOR, Room::HALLWAY,
        "  13. Slow heart rate (bradycardia)");
    Logger::getInstance().logInfo("Main", DeviceType::FIRE_SENSOR, Room::HALLWAY,
        "  14. Low blood pressure");
    Logger::getInstance().logInfo("Main", DeviceType::FIRE_SENSOR, Room::HALLWAY,
        "  15. Low oxygen saturation");
    
    Logger::getInstance().logInfo("Main", DeviceType::FIRE_SENSOR, Room::HALLWAY,
        "VITAL SIGNS - CRITICAL HIGH (16-18):");
    Logger::getInstance().logInfo("Main", DeviceType::FIRE_SENSOR, Room::HALLWAY,
        "  16. Maximum vitals bundle");
    Logger::getInstance().logInfo("Main", DeviceType::FIRE_SENSOR, Room::HALLWAY,
        "  17. Rapid heart rate (tachycardia)");
    Logger::getInstance().logInfo("Main", DeviceType::FIRE_SENSOR, Room::HALLWAY,
        "  18. High blood pressure");
    
    Logger::getInstance().logInfo("Main", DeviceType::FIRE_SENSOR, Room::HALLWAY,
        "BOUNDARY CONDITIONS (19-21):");
    Logger::getInstance().logInfo("Main", DeviceType::FIRE_SENSOR, Room::HALLWAY,
        "  19. Oxygen saturation boundaries");
    Logger::getInstance().logInfo("Main", DeviceType::FIRE_SENSOR, Room::HALLWAY,
        "  20. Edge case - normal boundaries");
    Logger::getInstance().logInfo("Main", DeviceType::FIRE_SENSOR, Room::HALLWAY,
        "  21. Asymmetric blood pressure");
    
    Logger::getInstance().logInfo("Main", DeviceType::FIRE_SENSOR, Room::HALLWAY,
        "FALL DETECTION (22-24):");
    Logger::getInstance().logInfo("Main", DeviceType::FIRE_SENSOR, Room::HALLWAY,
        "  22. Fall with panic response");
    Logger::getInstance().logInfo("Main", DeviceType::FIRE_SENSOR, Room::HALLWAY,
        "  23. Fall immobile + critical vitals");
    Logger::getInstance().logInfo("Main", DeviceType::FIRE_SENSOR, Room::HALLWAY,
        "  24. No motion but high vitals");
    
    Logger::getInstance().logInfo("Main", DeviceType::FIRE_SENSOR, Room::HALLWAY,
        "COMPLEX SCENARIOS (25-27):");
    Logger::getInstance().logInfo("Main", DeviceType::FIRE_SENSOR, Room::HALLWAY,
        "  25. Fire + Health crisis combined");
    Logger::getInstance().logInfo("Main", DeviceType::FIRE_SENSOR, Room::HALLWAY,
        "  26. Rapid condition change (collapse)");
    Logger::getInstance().logInfo("Main", DeviceType::FIRE_SENSOR, Room::HALLWAY,
        "  27. Recovery sequence");
    
    Logger::getInstance().logInfo("Main", DeviceType::FIRE_SENSOR, Room::HALLWAY,
        "SENSOR RELIABILITY (28-30):");
    Logger::getInstance().logInfo("Main", DeviceType::FIRE_SENSOR, Room::HALLWAY,
        "  28. False motion detection");
    Logger::getInstance().logInfo("Main", DeviceType::FIRE_SENSOR, Room::HALLWAY,
        "  29. Zero values (sensor failure)");
    Logger::getInstance().logInfo("Main", DeviceType::FIRE_SENSOR, Room::HALLWAY,
        "  30. Stress test - all sensors active");
    
    Logger::getInstance().logInfo("Main", DeviceType::FIRE_SENSOR, Room::HALLWAY,
        "=============================================");
    
    std::cout << "Choose a scenario (1-30): ";
    std::cin >> scenario;
    
    if (scenario < 1 || scenario > (int)scenarios.size()) {
        Logger::getInstance().logError("Main", DeviceType::FIRE_SENSOR, Room::HALLWAY,
            "Invalid scenario number");
        return 1;
    }
    
    myHouse.loadScenario(scenarios[scenario-1]);
    myHouse.start(3);
    
    return 0;
}