
#pragma once

#include "../core/DataModel.hpp"
#include "../core/EnumTraits.hpp"
#include <string>
#include <vector>
#include <algorithm>

namespace topics {

  inline std::string toLower(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), ::tolower);
    return result;
  }

  inline std::string roomFireTopic(Room room) {
    return "sensors/" + toLower(to_string_enum(room)) + "/fire";
  }

  inline std::string roomPIRTopic(Room room) {
    return "sensors/" + toLower(to_string_enum(room)) + "/pir";
  }

  inline std::string wristbandMotionTopic() {
    return "sensors/wristband/motion";
  }

  inline std::string wristbandHealthTopic() {
    return "sensors/wristband/health";
  }

  inline std::string actuatorTopic(Room room, const std::string& actuatorId) {
    return "actuators/" + toLower(to_string_enum(room)) + "/" + actuatorId;
  }

  inline std::string lockTopic() {
    return "actuators/lock";
  }

  inline std::string dialerTopic() {
    return "actuators/dialer";
  }

  inline std::vector<std::string> getAllSensorTopics() {
    return {
      wristbandMotionTopic(),
      wristbandHealthTopic(),
      
      roomFireTopic(Room::KITCHEN),
      roomPIRTopic(Room::KITCHEN),
      
      roomFireTopic(Room::LIVING_ROOM),
      roomPIRTopic(Room::LIVING_ROOM),
      
      roomFireTopic(Room::BEDROOM),
      roomPIRTopic(Room::BEDROOM),
      
      roomFireTopic(Room::BATHROOM),
      roomPIRTopic(Room::BATHROOM),
      
      roomFireTopic(Room::HALLWAY),
      roomPIRTopic(Room::HALLWAY),
    };
  }

}