#pragma once

#include <string>
#include <vector>
#include <map>
#include <mutex>
#include <thread>
#include <atomic>
#include <chrono>
#include <iostream>
#include <iomanip> // For pretty table formatting
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "../core/DataModel.hpp"
#include "../core/EnumTraits.hpp"


#define MONITOR_FIELD_WIDTH 30

class DeviceRegistry {
public:
    explicit DeviceRegistry(const SSDPConfig& cfg) : config(cfg) {
        running.store(true);

        listenThread = std::thread(&DeviceRegistry::receiverLoop, this);

        monitorThread = std::thread(&DeviceRegistry::monitorLoop, this);
    }

    ~DeviceRegistry() {
        stop();
    }

    void stop() {
        if (!running.load()) return;
        running.store(false);

        if (sockFd >= 0) {
            ::shutdown(sockFd, SHUT_RDWR);
            ::close(sockFd);
            sockFd = -1;
        }

        if (listenThread.joinable()) listenThread.join();
        if (monitorThread.joinable()) monitorThread.join();
    }

private:
    SSDPConfig config = {
        "239.255.255.250", 
        1900, 
        30, 
        2
    };
    std::atomic<bool> running{false};
    std::thread listenThread;
    std::thread monitorThread; // The new display thread
    int sockFd{-1};

    std::mutex mtx;
    std::map<std::string, RemoteDevice> devices;

    void monitorLoop() {
        const std::string RESET  = "\033[0m";
        const std::string BOLD   = "\033[1m";
        const std::string GREEN  = "\033[32m";
        const std::string CYAN   = "\033[36m";
        const std::string YELLOW = "\033[33m";
        const std::string RED    = "\033[31m";

        while (running.load()) {
            {
                std::lock_guard<std::mutex> lock(mtx);
                cleanupStaleDevices();

                // Clear screen and home cursor
                std::cout << "\033[2J\033[H"; 

                std::cout << BOLD << CYAN << "╔══════════════════════════════════════════════════════════════════════════════╗" << RESET << std::endl;
                std::cout << BOLD << CYAN << "║                INTELLISENSE SYSTEM - ACTIVE DEVICE REGISTRY                  ║" << RESET << std::endl;
                std::cout << BOLD << CYAN << "╚══════════════════════════════════════════════════════════════════════════════╝" << RESET << std::endl;
                
                std::cout << BOLD << std::left 
                        << std::setw(35) << "  DEVICE ID" 
                        << std::setw(20) << "TYPE" 
                        << std::setw(20) << "LOCATION" 
                        << "STATUS" << RESET << std::endl;
                std::cout << std::string(80, '-') << std::endl;

                if (devices.empty()) {
                    std::cout << YELLOW << "\n     [!] IntelliSense is scanning... (Waiting for discovery packets)" << RESET << std::endl;
                } else {
                    for (const auto& [id, dev] : devices) {
                        
                        std::string displayId = dev.id;
                        if (displayId.length() > 30) {
                            displayId = displayId.substr(0, 27) + "...";
                        }

                        std::cout << "  " 
                                << std::left << std::setw(33) << displayId 
                                << std::setw(20) << to_string_enum(dev.type) 
                                << std::setw(20) << to_string_enum(dev.location) 
                                << GREEN << BOLD << "● ONLINE" << RESET << std::endl;
                    }
                }
                std::cout << std::string(80, '-') << std::endl;
                std::cout << " System Status: " << GREEN << "HEALTHY" << RESET 
                        << " | Devices: " << BOLD << devices.size() << RESET 
                        << " | Interval: " << config.interval << "s" << std::endl;
            }

            std::this_thread::sleep_for(std::chrono::seconds(5));
        }
    }

    void receiverLoop() {
        sockFd = ::socket(AF_INET, SOCK_DGRAM, 0);
        int reuse = 1;
        ::setsockopt(sockFd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_port = htons(config.port);
        addr.sin_addr.s_addr = INADDR_ANY;
        ::bind(sockFd, (struct sockaddr*)&addr, sizeof(addr));

        struct ip_mreq mreq;
        mreq.imr_multiaddr.s_addr = ::inet_addr(config.multicastGroup.c_str());
        mreq.imr_interface.s_addr = htonl(INADDR_ANY);
        ::setsockopt(sockFd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq));

        char buffer[2048];
        while (running.load()) {
            ssize_t len = ::recv(sockFd, buffer, sizeof(buffer) - 1, 0);
            if (len > 0) {
                buffer[len] = '\0';
                processPacket(std::string(buffer));
            }
        }
    }

    void processPacket(const std::string& packet) {
        if (packet.find("NOTIFY") == std::string::npos) return;

        std::string nts = extractValue(packet, "NTS: ", "\r\n");
        std::string usn = extractValue(packet, "USN: uuid:", "\r\n");
        if (usn.empty()) return;


        size_t idEnd = usn.find("::");
        if (idEnd == std::string::npos) return; 
        std::string id = usn.substr(0, idEnd);
        
        std::lock_guard<std::mutex> lock(mtx);
        if (nts == "ssdp:byebye") {
            devices.erase(id);
        } else if (nts == "ssdp:alive") {

            std::string typeStr = extractValue(usn, "type:", "::");
            std::string roomStr = extractValue(usn, "room:", ""); 

            roomStr.erase(roomStr.find_last_not_of(" \n\r\t") + 1);

            DeviceType type = from_string_enum<DeviceType>(typeStr);
            Room room       = from_string_enum<Room>(roomStr);
            
            devices[id] = {id, type, room, std::chrono::steady_clock::now()};
        }
    }

    void cleanupStaleDevices() {
        auto now = std::chrono::steady_clock::now();
        auto timeout = std::chrono::seconds((config.interval * 2) + 2);
        for (auto it = devices.begin(); it != devices.end(); ) {
            if (now - it->second.lastSeen > timeout) it = devices.erase(it);
            else ++it;
        }
    }

    std::string extractValue(const std::string& s, const std::string& pre, const std::string& post) {
        size_t start = s.find(pre);
        if (start == std::string::npos) return "";
        start += pre.length();
        size_t end = (post.empty()) ? std::string::npos : s.find(post, start);
        return s.substr(start, end - start);
    }
};