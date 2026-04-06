#pragma once

#include "EnumTraits.hpp"
#include "DataModel.hpp"
#include "PayloadContracts.hpp"
#include "../third_party/nlohmann/json.hpp"

using json = nlohmann::json;

class JSONAdapter{
  public:
    static json encode(const Message& m){
      json j;

      j["deviceId"] = m.deviceId;
      j["deviceType"] = to_string_enum(m.deviceType);
      j["timestamp"] = m.timestamp;
      j["location"] = to_string_enum(m.location);

      j["payload"] = {
        {"type",to_string_enum(m.payload.payloadType)},
        {"data",m.payload.data}
      };

      return j;
    }

    static Message decode(const json&j){
      Message m;

      m.deviceId = j.at("deviceId").get<std::string>();
      m.deviceType = from_string_enum<DeviceType>(j.at("deviceType").get<std::string>());
      m.timestamp = j.at("timestamp").get<long>();
      
      m.location = from_string_enum<Room>(j.at("location").get<std::string>());
      
      json p = j.at("payload");
      m.payload.payloadType = from_string_enum<PayloadType>(p.at("type").get<std::string>());

      m.payload.data = p.at("data");
      
      return m;
    }
    
};
