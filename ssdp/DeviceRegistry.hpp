#pragma once

#include <string>
#include <vector>
#include <map>
#include <mutex>
#include <thread>
#include <atomic>
#include <chrono>
#include <iostream>
#include <iomanip>
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

        inventory = config.allowedDeviceIds;

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
    std::thread monitorThread; 
    int sockFd{-1};

    std::mutex mtx;
    std::map<std::string, RemoteDevice> devices;

    std::vector<std::string> inventory;

    std::map<std::string, RemoteDevice> inactiveDevices;

    std::map<std::string, RemoteDevice> unavailableDevices;

    std::map<std::string, std::chrono::steady_clock::time_point> excludedDevices;

    bool isAllowedDevice(const std::string& id, DeviceType type) {
       
        if (config.allowedDeviceIds.empty() && config.allowedTypes.empty()) return true;

        if (!config.allowedDeviceIds.empty()) {
            for (const auto& allowed : config.allowedDeviceIds) {
                if (allowed == id) return true;
            }
            return false;
        }

        if (!config.allowedTypes.empty()) {
            for (const auto& t : config.allowedTypes) {
                if (t == type) return true;
            }
            return false;
        }

        return true;
    }

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

                // ACTIVE
                std::cout << BOLD << "  Active Devices:" << RESET << std::endl;
                if (devices.empty()) {
                    std::cout << YELLOW << "    [none]" << RESET << std::endl;
                } else {
                    for (const auto& [id, dev] : devices) {
                        
                        std::string displayId = dev.id;
                        if (displayId.length() > 30) {
                            displayId = displayId.substr(0, 27) + "...";
                        }

                        auto now = std::chrono::steady_clock::now();
                        auto timeout = std::chrono::seconds((config.interval * 2) + 2);
                        bool online = (now - dev.lastSeen) <= timeout;

                        std::cout << "  " 
                                << std::left << std::setw(33) << displayId 
                                << std::setw(20) << to_string_enum(dev.type) 
                                << std::setw(20) << to_string_enum(dev.location);

                        if (online) {
                            std::cout << GREEN << BOLD << "● ONLINE" << RESET;
                        } else {
                            std::cout << YELLOW << BOLD << "○ OFFLINE" << RESET;
                        }

                        std::cout << std::endl;
                    }
                }

                // INACTIVE
                std::cout << std::endl << BOLD << "  Inactive Devices:" << RESET << std::endl;
                if (inactiveDevices.empty()) {
                    std::cout << YELLOW << "    [none]" << RESET << std::endl;
                } else {
                    for (const auto& [id, dev] : inactiveDevices) {
                        std::string displayId = dev.id;
                        if (displayId.length() > 30) displayId = displayId.substr(0,27) + "...";
                        std::cout << "  " << std::left << std::setw(33) << displayId 
                                  << std::setw(20) << to_string_enum(dev.type)
                                  << std::setw(20) << to_string_enum(dev.location)
                                  << YELLOW << BOLD << "○ INACTIVE" << RESET << std::endl;
                    }
                }

                // UNAVAILABLE
                std::cout << std::endl << BOLD << "  Unavailable Devices:" << RESET << std::endl;
                if (unavailableDevices.empty()) {
                    std::cout << YELLOW << "    [none]" << RESET << std::endl;
                } else {
                    for (const auto& [id, dev] : unavailableDevices) {
                        std::string displayId = dev.id;
                        if (displayId.length() > 30) displayId = displayId.substr(0,27) + "...";
                        std::cout << "  " << std::left << std::setw(33) << displayId 
                                  << std::setw(20) << to_string_enum(dev.type)
                                  << std::setw(20) << to_string_enum(dev.location)
                                  << RED << BOLD << "× UNAVAILABLE" << RESET << std::endl;
                    }
                }

                // EXCLUDED
                std::cout << std::endl << BOLD << "  Excluded Devices (seen but not allowed):" << RESET << std::endl;
                if (excludedDevices.empty()) {
                    std::cout << YELLOW << "    [none]" << RESET << std::endl;
                } else {
                    for (const auto& [id, ts] : excludedDevices) {
                        std::cout << "  " << std::left << std::setw(33) << id
                                  << std::setw(20) << "-" << std::setw(20) << "-"
                                  << CYAN << "! EXCLUDED" << RESET << std::endl;
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
            // move to unavailableDevices if we know this device
            auto dit = devices.find(id);
            if (dit != devices.end()) {
                unavailableDevices[id] = dit->second;
                devices.erase(dit);
            } else {
                // maybe was inactive earlier
                auto iit = inactiveDevices.find(id);
                if (iit != inactiveDevices.end()) {
                    unavailableDevices[id] = iit->second;
                    inactiveDevices.erase(iit);
                } else {
                    // unknown device sent byebye; record minimal info
                    RemoteDevice rv = {id, DeviceType::WRISTBAND, Room::HALLWAY, std::chrono::steady_clock::now()};
                    unavailableDevices[id] = rv;
                }
            }
        } else if (nts == "ssdp:alive") {

            std::string typeStr = extractValue(usn, "type:", "::");
            std::string roomStr = extractValue(usn, "room:", ""); 

            roomStr.erase(roomStr.find_last_not_of(" \n\r\t") + 1);

            DeviceType type = from_string_enum<DeviceType>(typeStr);
            Room room       = from_string_enum<Room>(roomStr);
            
            auto now = std::chrono::steady_clock::now();

            if (!isAllowedDevice(id, type)) {
                excludedDevices[id] = now;
                std::cerr << "[DeviceRegistry] Excluding device by policy: " << id << std::endl;
                return;
            }


            unavailableDevices.erase(id);
            inactiveDevices.erase(id);
            excludedDevices.erase(id);

            devices[id] = {id, type, room, now};
        }
    }

    void cleanupStaleDevices() {
        auto now = std::chrono::steady_clock::now();
        auto timeout = std::chrono::seconds((config.interval * 2) + 2);
        for (auto it = devices.begin(); it != devices.end(); ) {
            if (now - it->second.lastSeen > timeout) {
                std::cerr << "[DeviceRegistry] Marking inactive: " << it->first << std::endl;
                inactiveDevices[it->first] = it->second;
                it = devices.erase(it);
            } else ++it;
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