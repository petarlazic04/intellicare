#pragma once

#include "../core/DataModel.hpp"
#include "../core/EnumTraits.hpp"
#include "../core/JSONAdapter.hpp"
#include "../core/PayloadContracts.hpp"
#include "../core/Topics.hpp"
#include "../core/Logger.hpp"
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
        Logger::getInstance().logInfo("Hub", DeviceType::FIRE_SENSOR, Room::HALLWAY,
            "Received message on topic: " + topic);
        try{
          Message msg = parseMessage(payload);
          Logger::getInstance().logInfo(msg.deviceId, msg.deviceType, msg.location,
              "Parsed message - Device: " + msg.deviceId);
          onMessage(msg);
        } catch (const std::exception& e){
          Logger::getInstance().logError("Hub", DeviceType::FIRE_SENSOR, Room::HALLWAY,
              std::string("Error parsing message: ") + e.what());
        }
      });

      if(!mqtt.connect()){
        Logger::getInstance().logError("Hub", DeviceType::FIRE_SENSOR, Room::HALLWAY,
            "Failed to connect Hub to MQTT broker");
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
      Logger::getInstance().logEmergency("AMBULANCE", "Calling ambulance for " + to_string_enum(location),
          {{"location", to_string_enum(location)}, {"topic", topic}});
      sendActuatorCommand(topic, "dialer_main", DeviceActionType::DIAL_AMBULANCE, location, to_string_enum(DialerActionType::DIAL_AMBULANCE));
    }

    void callFiremen(Room location) {
      std::string topic = topics::dialerTopic();
      Logger::getInstance().logEmergency("FIRE_BRIGADE", "Calling fire brigade for " + to_string_enum(location),
          {{"location", to_string_enum(location)}, {"topic", topic}});
      sendActuatorCommand(topic, "dialer_main", DeviceActionType::DIAL_FIRE_BRIGADE, location, to_string_enum(DialerActionType::DIAL_FIRE_BRIGADE));
    }

    void notifyFamily(Room location) {
      std::string topic = topics::dialerTopic();
      Logger::getInstance().logEmergency("FAMILY_NOTIFICATION", "Notifying family for " + to_string_enum(location),
          {{"location", to_string_enum(location)}, {"topic", topic}});
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
      Logger::getInstance().logActuatorAction("lock_main", DeviceType::DOOR_LOCK, location,
          "Unlocking the house door for " + to_string_enum(location),
          {{"topic", topic}, {"location", to_string_enum(location)}});
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
        Logger::getInstance().logError("Hub", DeviceType::FIRE_SENSOR, Room::HALLWAY,
            std::string("Error parsing JSON: ") + e.what());
        throw;
      }
    }

    void setupSubscriptions(){
      auto sensorTopics = topics::getAllSensorTopics();
      Logger::getInstance().logInfo("Hub", DeviceType::FIRE_SENSOR, Room::HALLWAY,
          "Hub subscribing to " + std::to_string(sensorTopics.size()) + " topics");
      for (const auto& topic : sensorTopics){
        Logger::getInstance().logInfo("Hub", DeviceType::FIRE_SENSOR, Room::HALLWAY,
            "Subscribing to: " + topic);
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

      json cmdMetadata = {{"actuatorTopic", actuatorTopic}, {"action", to_string_enum(action)}};
      if (!value.empty()) cmdMetadata["value"] = value;
      
      Logger::getInstance().logActuatorAction(deviceId, getDeviceTypeFromTopic(actuatorTopic), location,
          "Sending command " + to_string_enum(action), cmdMetadata);

      mqtt.publish(actuatorTopic, JSONAdapter::encode(cmd).dump(), MQTT_DEFAULT_QOS);
    }
    
    DeviceType getDeviceTypeFromTopic(const std::string& topic) {
      if (topic.find("sprinkler") != std::string::npos) return DeviceType::SPRINKLER;
      if (topic.find("light") != std::string::npos) return DeviceType::LIGHT;
      if (topic.find("speaker") != std::string::npos) return DeviceType::SPEAKER;
      if (topic.find("lock") != std::string::npos) return DeviceType::DOOR_LOCK;
      if (topic.find("dialer") != std::string::npos) return DeviceType::DIALER;
      return DeviceType::FIRE_SENSOR;
    }

};

// Handler implementations
void handlers::handleHealthPayload(const Message& msg, Hub& hub){
  HealthData healthData = extractPayload<HealthData>(msg.payload);

  json metadata = {{"heartRate", healthData.heartRate}, {"spo2", healthData.spo2},
                    {"systolic", healthData.systolic}, {"diastolic", healthData.diastolic}};
  Logger::getInstance().logSensorData(msg.deviceId, msg.deviceType, msg.location,
      "Processing health data", metadata);

  hub.updateVitals(healthData, msg.timestamp);
  hub.updateLocation(msg.location);

  if (hazards::isCriticalHealth(healthData)) {
    Logger::getInstance().logHazard("CRITICAL_HEALTH", to_string_enum(msg.location),
        "Critical health detected; unlocking doors and calling ambulance", metadata);
    hub.unlockDoors(msg.location);
    hub.callAmbulance(msg.location);
  } else {
    Logger::getInstance().logInfo(msg.deviceId, msg.deviceType, msg.location,
        "Health data is within normal limits");
  }
}

void handlers::handleMotionPayload(const Message& msg, Hub& hub){
  MotionData motionData = extractPayload<MotionData>(msg.payload);

  Logger::getInstance().logSensorData(msg.deviceId, msg.deviceType, msg.location,
      "Processing motion data", {{"magnitude", motionData.magnitude}});
  
  if(hazards::isFall(motionData)){

    if(!hazards::shouldProcessFall(hub.getLastFallTimestamp(), msg.timestamp)){
      Logger::getInstance().logInfo(msg.deviceId, msg.deviceType, msg.location,
          "Fall ignored due to debounce interval");
      return;
    }
    
    hub.updateLastFallTime(msg.timestamp);
    Logger::getInstance().logHazard("FALL_DETECTED", to_string_enum(hub.getLocation()),
        "Fall detected", {{"magnitude", motionData.magnitude}});
    hub.updateLocation(msg.location);

    if(hazards::isCriticalHealth(hub.getVitals())){
      Logger::getInstance().logEmergency("FALL_WITH_CRITICAL_VITALS",
          "Vitals are critical or stale; unlocking doors and calling ambulance",
          {{"location", to_string_enum(hub.getLocation())}});
      hub.unlockDoors(hub.getLocation());
      hub.callAmbulance(hub.getLocation());
    } else {
      Logger::getInstance().logInfo(msg.deviceId, msg.deviceType, msg.location,
          "Vitals are stable; notifying family");
      hub.notifyFamily(hub.getLocation());
    }

  } else {
    Logger::getInstance().logInfo(msg.deviceId, msg.deviceType, msg.location,
        "Motion event not classified as a fall");
  }
}

void handlers::handleFirePayload(const Message& msg, Hub& hub){
  FireDetectorData fireDetectorData = extractPayload<FireDetectorData>(msg.payload);
  
  json metadata = {{"temperature", fireDetectorData.temperature},
                    {"smokeLevel", fireDetectorData.smokeLevel},
                    {"coLevel", fireDetectorData.coLevel}};
  Logger::getInstance().logSensorData(msg.deviceId, msg.deviceType, msg.location,
      "Processing fire data", metadata);

  if(hazards::isFireHazard(fireDetectorData)){
    Logger::getInstance().logEmergency("FIRE_HAZARD_DETECTED",
        "FIRE HAZARD DETECTED! Activating emergency response", metadata);
    hub.updateLocation(msg.location);

    hub.unlockDoors(msg.location);
    hub.activateSprinklers(msg.location);
    hub.alertLights(msg.location);
    hub.alertSpeakers(msg.location);
    hub.callFiremen(msg.location);
  } else {
    Logger::getInstance().logInfo(msg.deviceId, msg.deviceType, msg.location,
        "Fire data received but does not meet hazard thresholds");
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
