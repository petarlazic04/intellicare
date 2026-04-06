#pragma once

#include "../core/DataModel.hpp"
#include "../core/EnumTraits.hpp"
#include "../core/JSONAdapter.hpp"
#include "../core/PayloadContracts.hpp"
#include "../core/Topics.hpp"
#include "../mqtt/MQTT.hpp"
#include "handlers/Handlers.hpp"
#include <map>
#include <chrono>
#include <iostream>


class Hub{

  private:
    Room currentLocation;
    MQTT mqtt;
    long lastFallTimestamp;
    long lastVitalsTimestamp;
    long lastPIRMotionTime;
    HealthData vitals;

    using Handler = std::function<void(const Message &msg, Hub&)>;
    std::map<PayloadType, Handler> handlers;


  public:

    Hub(const std::string& broker, int port): 
      mqtt(broker,port),
      currentLocation(Room::LIVING_ROOM),
      lastVitalsTimestamp(0),
      lastPIRMotionTime(0){

      setupHandlers();

      mqtt.on_message([this](const std::string& topic, const std::string& payload){
        std::cout << "Hub received message on topic: " << topic << std::endl;
        try{
          Message msg = parseMessage(payload);
          std::cout << "Parsed message - Device: " << msg.deviceId << ", Type: " << static_cast<int>(msg.payload.payloadType) << std::endl;
          onMessage(msg);
        } catch (const std::exception& e){
          std::cerr << "Error parsing message:" << e.what() << "\n";
        }
      });

      if(!mqtt.connect()){
        std::cerr << "Failed to connect Hub to MQTT broker\n";
      }

      setupSubscriptions();

    }

      
    void updateVitals(const HealthData& healthData, long timestamp){
      vitals = healthData;
      lastVitalsTimestamp = timestamp;
    }

    void updateLocation(Room location){
      currentLocation = location;
    }

    void updateLastMotionTime(long timestamp){
      lastPIRMotionTime = timestamp;
    }

    void updateLastFallTime(long timestamp){
      lastFallTimestamp = timestamp;
    }

    Room getLocation() { return currentLocation; }
    HealthData getVitals() { return vitals; }
    long getLastVitalsTimestamp() { return lastVitalsTimestamp; }
    long getLastFallTimestamp() { return lastFallTimestamp; }

    void callAmbulance(Room location) {
      std::string topic = topics::dialerTopic();
      std::cout << "Hub: calling ambulance for " << to_string_enum(location) << " on topic " << topic << std::endl;
      sendActuatorCommand(topic, "dialer_main", DeviceActionType::DIAL_AMBULANCE, location, to_string_enum(DialerActionType::DIAL_AMBULANCE));
    }

    void callFiremen(Room location) {
      std::string topic = topics::dialerTopic();
      std::cout << "Hub: calling fire brigade for " << to_string_enum(location) << " on topic " << topic << std::endl;
      sendActuatorCommand(topic, "dialer_main", DeviceActionType::DIAL_FIRE_BRIGADE, location, to_string_enum(DialerActionType::DIAL_FIRE_BRIGADE));
    }

    void notifyFamily(Room location) {
      std::string topic = topics::dialerTopic();
      std::cout << "Hub: notifying family for " << to_string_enum(location) << " on topic " << topic << std::endl;
      sendActuatorCommand(topic, "dialer_main", DeviceActionType::NOTIFY_FAMILY, location, to_string_enum(DialerActionType::NOTIFY_FAMILY));
    }

    void activateSprinklers(Room location) {
      std::string topic = topics::actuatorTopic(location, "sprinkler");
      sendActuatorCommand(topic, "sprinkler_" + to_string_enum(location), DeviceActionType::START, location);
    }

    void alertLights(Room location) {
      std::string topic = topics::actuatorTopic(location, "light");
      sendActuatorCommand(topic, "light_" + to_string_enum(location), DeviceActionType::TURN_ON, location);
    }

    void alertSpeakers(Room location) {
      std::string topic = topics::actuatorTopic(location, "speaker");
      sendActuatorCommand(topic, "speaker_" + to_string_enum(location), DeviceActionType::TURN_ON, location, "100");
    }

    void unlockDoors(Room location) {
      std::string topic = topics::lockTopic();
      std::cout << "Hub: unlocking the house door for " << to_string_enum(location) << " on topic " << topic << std::endl;
      sendActuatorCommand(topic, "lock_main", DeviceActionType::UNLOCK, location);
    }
  
  private:

    void onMessage(const Message &msg){
      auto it = handlers.find(msg.payload.payloadType);

      if(it != handlers.end()){
        it->second(msg, *this);
      } else {
        std::cerr << "No handler for payload type\n";
      }

    }

    Message parseMessage(const std::string& payload){
      try{
        json j = json::parse(payload);
        return JSONAdapter::decode(j);
      } catch(const std::exception& e){
        std::cerr << "Error parsing JSON: " << e.what() << "\n";
        throw;
      }
    }

    void setupSubscriptions(){
      auto sensorTopics = topics::getAllSensorTopics();
      std::cout << "Hub subscribing to topics:" << std::endl;
      for (const auto& topic : sensorTopics){
        std::cout << "  - " << topic << std::endl;
        mqtt.subscribe(topic, MQTT_DEFAULT_QOS);
      }
    }


    void setupHandlers(){
      handlers[PayloadType::HEALTH_PAYLOAD] = handlers::handleHealthPayload;
      handlers[PayloadType::FIRE_DETECTOR_PAYLOAD] = handlers::handleFirePayload;
      handlers[PayloadType::MOTION_PAYLOAD] = handlers::handleMotionPayload;
      handlers[PayloadType::PIR_PAYLOAD] = handlers::handlePIRPayload;
      handlers[PayloadType::COMMAND] = handlers::handleCommand;
      handlers[PayloadType::UNKNOWN] = [](const Message &msg, Hub& hub){};
    }
    
    void sendActuatorCommand(const std::string& actuatorTopic, const std::string& deviceId, DeviceActionType action, Room location, const std::string& value = "") {
      Message cmd;
      cmd.deviceId = deviceId;
      cmd.location = location;
      cmd.timestamp = std::chrono::system_clock::now().time_since_epoch().count();
      cmd.payload = makePayload<DeviceCommand>(DeviceCommand{action, value});

      std::cout << "Hub: sending command " << to_string_enum(action) << " to " << actuatorTopic;
      if(!value.empty()) std::cout << " value=" << value;
      std::cout << std::endl;

      mqtt.publish(actuatorTopic, JSONAdapter::encode(cmd).dump(), MQTT_DEFAULT_QOS);
    }

};

// Handler implementations
void handlers::handleHealthPayload(const Message& msg, Hub& hub){
  HealthData healthData = extractPayload<HealthData>(msg.payload);

  std::cout << "Processing health data - HR: " << healthData.heartRate
            << ", SpO2: " << healthData.spo2
            << ", Sys: " << healthData.systolic
            << ", Dia: " << healthData.diastolic << std::endl;

  hub.updateVitals(healthData, msg.timestamp);
  hub.updateLocation(msg.location);

  if (hazards::isCriticalHealth(healthData)) {
    std::cout << "Critical health detected in " << to_string_enum(msg.location) << "; unlocking doors and calling ambulance." << std::endl;
    hub.unlockDoors(msg.location);
    hub.callAmbulance(msg.location);
  } else {
    std::cout << "Health data is within normal limits." << std::endl;
  }
}

void handlers::handleMotionPayload(const Message& msg, Hub& hub){
  MotionData motionData = extractPayload<MotionData>(msg.payload);

  std::cout << "Processing motion data - magnitude: " << motionData.magnitude << std::endl;
  if(hazards::isFall(motionData)){

    if(!hazards::shouldProcessFall(hub.getLastFallTimestamp(), msg.timestamp)){
      std::cout << "Fall ignored due to debounce interval." << std::endl;
      return;
    }
    
    hub.updateLastFallTime(msg.timestamp);
    std::cout << "Fall detected in " << to_string_enum(hub.getLocation()) << "." << std::endl;
    hub.updateLocation(msg.location);

    if(hazards::isCriticalHealth(hub.getVitals())){
      std::cout << "Vitals are critical or stale; unlocking doors and calling ambulance." << std::endl;
      hub.unlockDoors(hub.getLocation());
      hub.callAmbulance(hub.getLocation());
    } else {
      std::cout << "Vitals are stable; notifying family." << std::endl;
      hub.notifyFamily(hub.getLocation());
    }

  } else {
    std::cout << "Motion event not classified as a fall." << std::endl;
  }
}

void handlers::handleFirePayload(const Message& msg, Hub& hub){
  FireDetectorData fireDetectorData = extractPayload<FireDetectorData>(msg.payload);
  
  std::cout << "Processing fire data - Temp: " << fireDetectorData.temperature 
            << "°C, Smoke: " << fireDetectorData.smokeLevel 
            << ", CO: " << fireDetectorData.coLevel << std::endl;

  if(hazards::isFireHazard(fireDetectorData)){
    std::cout << "FIRE HAZARD DETECTED! Activating emergency response..." << std::endl;
    hub.updateLocation(msg.location);

    hub.unlockDoors(msg.location);
    hub.activateSprinklers(msg.location);
    hub.alertLights(msg.location);
    hub.alertSpeakers(msg.location);
    hub.callFiremen(msg.location);
  } else {
    std::cout << "Fire data received but does not meet hazard thresholds." << std::endl;
  }
}

void handlers::handlePIRPayload(const Message& msg, Hub& hub){
  PIRData pirData = extractPayload<PIRData>(msg.payload);

  if(pirData.motionDetected){
    hub.updateLocation(msg.location);
  }
}

void handlers::handleCommand(const Message& msg, Hub& hub){
  //empty for now because hub shouldn't receive commands
}
