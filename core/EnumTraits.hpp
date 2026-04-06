#pragma once

#include <string>
#include <string_view>
#include "DataModel.hpp"
#include <algorithm>

std::string to_upper(std::string_view s) {
    std::string out(s);
    std::transform(out.begin(), out.end(), out.begin(), 
                   [](unsigned char c){ return std::toupper(c); });
    return out;
}

template<typename E>
struct EnumTraits;

template<typename E>
std::string to_string_enum(E e) {
  return std::string(EnumTraits<E>::to_string(e));
}

template<typename E>
E from_string_enum(std::string_view s) {
  return EnumTraits<E>::from_string(to_upper(s));
}

template<>
struct EnumTraits<Room> {
  static std::string_view to_string(Room r) {
    switch (r) {
      case Room::KITCHEN: return "KITCHEN";
      case Room::LIVING_ROOM: return "LIVING_ROOM";
      case Room::BEDROOM: return "BEDROOM";
      case Room::BATHROOM: return "BATHROOM";
      case Room::HALLWAY: return "HALLWAY";
      default:  return "UNKNOWN";
    }
  }

  static Room from_string(std::string_view s) {
    if (s == "KITCHEN") return Room::KITCHEN;
    if (s == "LIVING_ROOM") return Room::LIVING_ROOM;
    if (s == "BEDROOM") return Room::BEDROOM;
    if (s == "BATHROOM") return Room::BATHROOM;
    if (s == "HALLWAY") return Room::HALLWAY;
    return Room::LIVING_ROOM;
  }
};

template<>
struct EnumTraits<DeviceType> {
  static std::string_view to_string(DeviceType d) {
    switch (d) {
      case DeviceType::WRISTBAND: return "WRISTBAND";
      case DeviceType::FIRE_SENSOR: return "FIRE_SENSOR";
      case DeviceType::PIR_SENSOR: return "PIR_SENSOR";
      case DeviceType::DOOR_LOCK: return "DOOR_LOCK";
      case DeviceType::SPRINKLER: return "SPRINKLER";
      case DeviceType::LIGHT: return "LIGHT";
      case DeviceType::SPEAKER: return "SPEAKER";
      case DeviceType::DIALER: return "DIALER";
    }
    return "UNKNOWN";
  }

  static DeviceType from_string(std::string_view s) {
    if (s == "WRISTBAND") return DeviceType::WRISTBAND;
    if (s == "FIRE_SENSOR") return DeviceType::FIRE_SENSOR;
    if (s == "PIR_SENSOR") return DeviceType::PIR_SENSOR;
    if (s == "DOOR_LOCK") return DeviceType::DOOR_LOCK;
    if (s == "SPRINKLER") return DeviceType::SPRINKLER;
    if (s == "LIGHT") return DeviceType::LIGHT;
    if (s == "SPEAKER") return DeviceType::SPEAKER;
    return DeviceType::DIALER;
  }
};

template<>
struct EnumTraits<DeviceActionType> {
  static std::string_view to_string(DeviceActionType d) {
    switch (d) {
      case DeviceActionType::LOCK: return "LOCK";
      case DeviceActionType::UNLOCK: return "UNLOCK";
      case DeviceActionType::TURN_ON: return "TURN_ON";
      case DeviceActionType::TURN_OFF: return "TURN_OFF";
      case DeviceActionType::START: return "START";
      case DeviceActionType::STOP: return "STOP";
      case DeviceActionType::SET_LEVEL: return "SET_LEVEL";
      case DeviceActionType::DIAL_AMBULANCE: return "DIAL_AMBULANCE";
      case DeviceActionType::DIAL_FIRE_BRIGADE: return "DIAL_FIRE_BRIGADE";
      case DeviceActionType::NOTIFY_FAMILY: return "NOTIFY_FAMILY";
    }
    return "UNKNOWN";
  }

  static DeviceActionType from_string(std::string_view s) {
    if (s == "LOCK") return DeviceActionType::LOCK;
    if (s == "UNLOCK") return DeviceActionType::UNLOCK;
    if (s == "TURN_ON") return DeviceActionType::TURN_ON;
    if (s == "TURN_OFF") return DeviceActionType::TURN_OFF;
    if (s == "START") return DeviceActionType::START;
    if (s == "STOP") return DeviceActionType::STOP;
    if (s == "SET_LEVEL") return DeviceActionType::SET_LEVEL;
    if (s == "DIAL_AMBULANCE") return DeviceActionType::DIAL_AMBULANCE;
    if (s == "DIAL_FIRE_BRIGADE") return DeviceActionType::DIAL_FIRE_BRIGADE;
    return DeviceActionType::NOTIFY_FAMILY;
  }
};

template<>
struct EnumTraits<DialerActionType> {
  static std::string_view to_string(DialerActionType d) {
    switch (d) {
      case DialerActionType::DIAL_AMBULANCE: return "DIAL_AMBULANCE";
      case DialerActionType::DIAL_FIRE_BRIGADE: return "DIAL_FIRE_BRIGADE";
      case DialerActionType::NOTIFY_FAMILY: return "NOTIFY_FAMILY";
    }
    return "UNKNOWN";
  }

  static DialerActionType from_string(std::string_view s) {
    if (s == "DIAL_AMBULANCE") return DialerActionType::DIAL_AMBULANCE;
    if (s == "DIAL_FIRE_BRIGADE") return DialerActionType::DIAL_FIRE_BRIGADE;
    return DialerActionType::NOTIFY_FAMILY;
  }
};

template<>
struct EnumTraits<PayloadType> {
  static std::string_view to_string(PayloadType p) {
    switch (p) {
      case PayloadType::FIRE_DETECTOR_PAYLOAD: return "FIRE_DETECTOR_PAYLOAD";
      case PayloadType::HEALTH_PAYLOAD: return "HEALTH_PAYLOAD";
      case PayloadType::MOTION_PAYLOAD: return "MOTION_PAYLOAD";
      case PayloadType::PIR_PAYLOAD: return "PIR_PAYLOAD";
      case PayloadType::COMMAND: return "COMMAND";
      case PayloadType::UNKNOWN: return "UNKNOWN";
    }
    return "UNKNOWN";
  }

  static PayloadType from_string(std::string_view s) {
    if (s == "FIRE_DETECTOR_PAYLOAD") return PayloadType::FIRE_DETECTOR_PAYLOAD;
    if (s == "HEALTH_PAYLOAD") return PayloadType::HEALTH_PAYLOAD;
    if (s == "MOTION_PAYLOAD") return PayloadType::MOTION_PAYLOAD;
    if (s == "PIR_PAYLOAD") return PayloadType::PIR_PAYLOAD;
    if (s == "COMMAND") return PayloadType::COMMAND;
    return PayloadType::UNKNOWN;
  }
};