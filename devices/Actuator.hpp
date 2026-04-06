#pragma once
#include "../mqtt/MQTT.hpp"
#include <string>
#include "../core/DataModel.hpp"
#include "Device.hpp"

class Actuator : public Device{
  protected:
  std::string subscribeTopic;

  void subscribe(const std::string& topic){
    mqtt.subscribe(topic);
  }

  public:
  
  Actuator(const std::string& deviceId, DeviceType deviceType, Room location,
     const std::string& broker, const std::string& subscribeTopic, Environment& env, int port = 1883):
    Device(deviceId,deviceType,location,broker,env,port), subscribeTopic(subscribeTopic){
      mqtt.on_message([this](const std::string& topic, const std::string& msg){
        handle_message(topic,msg);
      });
      subscribe(subscribeTopic);
    }
  
  virtual ~Actuator() = default;
  
  virtual void act(const Message& msg) = 0;

  private:
    
    void handle_message(const std::string& topic, const std::string& msg){
      try{
        json j = json::parse(msg);
        Message msg = JSONAdapter::decode(j);
        act(msg);
      } catch(const std::exception e){
        std::cerr << "Failed to parse message " << e.what() << "\n";
      }
    }
    

};
