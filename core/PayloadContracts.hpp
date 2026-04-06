#pragma once
#include "EnumTraits.hpp"
#include "DataModel.hpp"
#include "../third_party/nlohmann/json.hpp"

using json = nlohmann::json;

template<typename T>
struct PayloadContract;

template<typename T>
Payload makePayload(const T& data){
  return Payload{
    PayloadContract<T>::type,
    PayloadContract<T>::to_json(data)
  };
}

template<typename T>
T extractPayload(const Payload& p){
  return PayloadContract<T>::from_json(p.data);
}


template<>
struct PayloadContract<FireDetectorData> {
  static constexpr PayloadType type = PayloadType::FIRE_DETECTOR_PAYLOAD;

  static json to_json(const FireDetectorData& d) {
    return json{
      {"temperature", d.temperature},
      {"smokeLevel", d.smokeLevel},
      {"coLevel", d.coLevel}
    };
  }

  static FireDetectorData from_json(const json& j) {
    return {
      j.at("temperature").get<float>(),
      j.at("smokeLevel").get<float>(),
      j.at("coLevel").get<int>()
    };
  }
};

template<>
struct PayloadContract<HealthData> {
  static constexpr PayloadType type = PayloadType::HEALTH_PAYLOAD;

  static json to_json(const HealthData& d) {
    return json{
      {"heartRate", d.heartRate},
      {"spo2", d.spo2},
      {"systolic", d.systolic},
      {"diastolic", d.diastolic}
    };
  }

  static HealthData from_json(const json& j) {
    return {
      j.at("heartRate").get<int>(),
      j.at("spo2").get<int>(),
      j.at("systolic").get<int>(),
      j.at("diastolic").get<int>()
    };
  }
};

template<>
struct PayloadContract<MotionData> {
  static constexpr PayloadType type = PayloadType::MOTION_PAYLOAD;

  static json to_json(const MotionData& d) {
    return json{
      {"accelX", d.accelX},
      {"accelY", d.accelY},
      {"accelZ", d.accelZ},
      {"gyroX", d.gyroX},
      {"gyroY", d.gyroY},
      {"gyroZ", d.gyroZ},
      {"magnitude", d.magnitude}
    };
  }

  static MotionData from_json(const json& j) {
    return {
      j.at("accelX").get<float>(),
      j.at("accelY").get<float>(),
      j.at("accelZ").get<float>(),
      j.at("gyroX").get<float>(),
      j.at("gyroY").get<float>(),
      j.at("gyroZ").get<float>(),
      j.at("magnitude").get<float>()
    };
  }
};

template<>
struct PayloadContract<PIRData> {
  static constexpr PayloadType type = PayloadType::PIR_PAYLOAD;

  static json to_json(const PIRData& d) {
    return json{
      {"motionDetected", d.motionDetected}
    };
  }

  static PIRData from_json(const json& j) {
    return {
      j.at("motionDetected").get<bool>()
    };
  }
};

template<>
struct PayloadContract<DeviceCommand> {
  static constexpr PayloadType type = PayloadType::COMMAND;

  static json to_json(const DeviceCommand& d) {
    return json{
      {"actionType", to_string_enum(d.actionType)},
      {"value", d.value}
    };
  }

  static DeviceCommand from_json(const json& j) {
    return {
      from_string_enum<DeviceActionType>(j.at("actionType").get<std::string>()),
      j.at("value").get<std::string>()
    };
  }
};

