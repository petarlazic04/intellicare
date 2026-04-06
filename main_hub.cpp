#include "hub/Hub.hpp"
#include <thread>
#include <chrono>

int main() {
    // Everything happens in the constructor: 
    // Connects, Subscribes, and sets up Message Handlers
    Hub smartHub("localhost", 1883);

    std::cout << "[Main] Hub is running. Monitoring for emergencies...\n";

    // Keep the main thread alive while the MQTT callbacks run logic
    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    return 0;
}