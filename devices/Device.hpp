#pragma once
#include "../mqtt/MQTT.hpp"
#include "../ssdp/SSDP.hpp"
#include <string>
#include "../core/DataModel.hpp"
#include "../core/JSONAdapter.hpp"
#include "../core/Logger.hpp"

class Environment;  

class Device{
  protected:
    std::string deviceId;
    DeviceType deviceType;
    Room location;
    MQTT mqtt;
    SSDP ssdp;
    JSONAdapter adapter;
    Environment& environment;
    Logger& logger;

  public:
    Device(const std::string& deviceId, DeviceType deviceType, Room location,
       const std::string& broker, Environment& env, Logger& log, int mqtt_port = 1883, SSDPConfig ssdpConfig = {})
      :deviceId(deviceId), deviceType(deviceType), location(location), mqtt(broker,mqtt_port), environment(env), 
      logger(log), ssdp(ssdpConfig, deviceType, deviceId, location){
        if(!mqtt.connect()){
          logger.logError(deviceId, deviceType, location, "Failed to connect device to broker");
        }
      }
    
    virtual ~Device(){
      mqtt.disconnect();
    }

    const std::string& getId() const { return deviceId; }
    DeviceType getDeviceType() const { return deviceType; }
    Room getLocation() const { return location; }
    void setLocation(Room newLocation) { location = newLocation; }
    bool isConnected() const { return mqtt.connected(); }
    
};
