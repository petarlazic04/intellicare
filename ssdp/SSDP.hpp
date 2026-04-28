#pragma once

#include <string>
#include <thread>
#include <atomic>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sstream>
#include <stdexcept>
#include <chrono>
#include "../core/DataModel.hpp"
#include "../core/EnumTraits.hpp"


class SSDP{

    private:
        SSDPConfig config;
        DeviceType deviceType;
        std::string deviceId;
        Room location;

        std::thread notifyThread;
        std::atomic<bool> running{false};
        int sockFd{-1};

        void stop() {
            if (!running.load()) return;
            running.store(false);

            sendNotify("ssdp:byebye");
            closeSocket();

            if (notifyThread.joinable())
                notifyThread.join();
        }

        bool openSocket() {
            sockFd = ::socket(AF_INET, SOCK_DGRAM, 0);
            if (sockFd < 0) return false;

            int yes = 1;
            ::setsockopt(sockFd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));

            unsigned char ttl = 4;
            ::setsockopt(sockFd, IPPROTO_IP, IP_MULTICAST_TTL, &ttl, sizeof(ttl));

            return true;
        }

        void closeSocket() {
            if (sockFd >= 0) {
                ::close(sockFd);
                sockFd = -1;
            }
        }

        std::string buildMessage(const std::string& nts) const {
            const std::string typeStr = to_string_enum(deviceType);  // e.g. "FIRE_SENSOR"
            const std::string roomStr = to_string_enum(location);    // e.g. "LIVING_ROOM"

            std::ostringstream oss;
            oss << "NOTIFY * HTTP/1.1\r\n"
                << "HOST: "          << config.multicastGroup << ":" << config.port << "\r\n"
                << "NT: urn:smarthome:device:1\r\n"
                << "NTS: "           << nts << "\r\n"
                
                << "USN: uuid:"      << deviceId 
                                    << "::type:" << typeStr 
                                    << "::room:" << roomStr << "\r\n";

            if (nts == "ssdp:alive") {
                oss << "CACHE-CONTROL: max-age=" << config.ttl << "\r\n"
                    << "LOCATION: mqtt://"       << deviceId    << "\r\n";
            }

            oss << "\r\n";
            return oss.str();
        }

        void sendNotify(const std::string& nts) {
            if (sockFd < 0) return;

            sockaddr_in dest{};
            dest.sin_family      = AF_INET;
            dest.sin_port        = htons(config.port);
            dest.sin_addr.s_addr = ::inet_addr(config.multicastGroup.c_str());

            const std::string msg = buildMessage(nts);
            ::sendto(sockFd, msg.c_str(), msg.size(), 0,
                    reinterpret_cast<sockaddr*>(&dest), sizeof(dest));
        }

        void notifyLoop() {
            sendNotify("ssdp:alive");  

            const auto interval = std::chrono::seconds(config.interval);
            auto       nextSend = std::chrono::steady_clock::now() + interval;

            while (running.load()) {
                std::this_thread::sleep_for(std::chrono::milliseconds(200));

                if (std::chrono::steady_clock::now() >= nextSend) {
                    sendNotify("ssdp:alive");
                    nextSend = std::chrono::steady_clock::now() + interval;
                }
            }
        }
        



    public:

        SSDP(SSDPConfig config, DeviceType deviceType,const std::string& deviceId, Room room) : config(config), deviceType(deviceType) , deviceId(deviceId), location(room) {
            
            if (!openSocket()){
                throw std::runtime_error("SSDP [" + deviceId + "]: failed to open UDP socket");
            }
               

            running.store(true);
            notifyThread = std::thread(&SSDP::notifyLoop, this);
        }


        ~SSDP() {
            stop();
        }

    

};