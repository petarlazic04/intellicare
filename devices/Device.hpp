#pragma once
#include "../mqtt/MQTT.hpp"
#include <string>
#include "../core/DataModel.hpp"
#include "../core/JSONAdapter.hpp"
#include "../core/Logger.hpp"

class Environment;  // Forward declaration

class Device{
  protected:
    std::string deviceId;
    DeviceType deviceType;
    Room location;
    MQTT mqtt;
    JSONAdapter adapter;
    Environment& environment;

  public:
    Device(const std::string& deviceId, DeviceType deviceType, Room location,
       const std::string& broker, Environment& env, int port = 1883)
      :deviceId(deviceId), deviceType(deviceType), location(location), mqtt(broker,port), environment(env){
        if(!mqtt.connect()){
          Logger::getInstance().logError(deviceId, deviceType, location, "Failed to connect device to broker");
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
