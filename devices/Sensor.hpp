#pragma once
#include "../mqtt/MQTT.hpp"
#include <string>
#include "../core/DataModel.hpp"
#include "Device.hpp"

class Sensor : public Device{
  protected:
  std::string publishTopic;

  void publish(const Message& msg){
    std::string json_str = JSONAdapter::encode(msg).dump();
    mqtt.publish(publishTopic, json_str);
  }

  public:
  
  Sensor(const std::string& deviceId, DeviceType deviceType, Room location,
     const std::string& broker, const std::string& topic, Environment& env, Logger& log, int port = 1883):
    Device(deviceId,deviceType,location,broker,env,log,port), publishTopic(topic){
      onConnect();
    }
  
  virtual ~Sensor() = default;
  
  virtual void sample() = 0;
  virtual void onConnect() {}

};
