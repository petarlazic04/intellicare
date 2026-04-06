#include "house/House.hpp"
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
    std::cout << "Scenarios:\n" << "\n1. Fire emergency\n2. Health emergency\n";
    std::cout << "3. Fall (vitals stable)\n4. Fall (vitals critical)\n5. Global reset\n";
    std::cout << "Choose a scenario:\n";
    std::cin >> scenario;
    
    myHouse.loadScenario(scenarios[scenario-1]);
    myHouse.start(3);
    
    return 0;
}